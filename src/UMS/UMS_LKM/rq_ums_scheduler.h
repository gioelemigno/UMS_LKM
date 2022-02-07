#pragma once
/// @file 
/// This file contains all the requests used to manage a ums_scheduler
///

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <asm/uaccess.h> /* for put_user */
#include <linux/proc_fs.h>
#include <linux/sched.h>

#include "../common/ums_requests.h"
#include "../common/ums_types.h"

#include "ums_hashtable.h"
#include "ums_proc.h"


// -------------------------------------------------------------------------------------------------
/**
 * Request used to create a new ums_scheduler
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_create_ums_scheduler(rq_create_delete_ums_scheduler_args_t* rq_args){
    rq_create_delete_ums_scheduler_args_t rq_args_san;
    ums_process_t* ums_process;

    ums_completion_list_sl_t* ums_completion_list_sl;

    ums_scheduler_t* ums_scheduler;
    ums_scheduler_sl_t* ums_scheduler_sl;

    pid_t pid;
    pid_t tgid;

    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    pid = current->pid;
    tgid = current->tgid;
    
    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;
    
    ums_process_get_ums_completion_list_sl(ums_process, rq_args_san.completion_list_d, ums_completion_list_sl);
    if(unlikely(ums_completion_list_sl == NULL))
        return ERR_INVALID_CLD;

    ums_scheduler = kmalloc(sizeof(ums_scheduler_t), GFP_KERNEL);
    INIT_UMS_SCHEDULER(ums_scheduler, current, ums_completion_list_sl);
    
    ums_scheduler->entry_point_args = rq_args_san.entry_point_args;
    ums_scheduler->cpu_core = rq_args_san.cpu_core;

    printk("set cpu_core = %d", ums_scheduler->cpu_core);
    ums_scheduler_sl = kmalloc(sizeof(ums_scheduler_sl_t), GFP_KERNEL);
    INIT_UMS_SCHEDULER_SL(ums_scheduler_sl, pid, ums_scheduler);
    
    ums_process_add_scheduler_sl(ums_process, ums_scheduler_sl);
    return 0;
}

/**
 * Request used by a scheduler to terminate its execution
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_exit_ums_scheduler(rq_create_delete_ums_scheduler_args_t* rq_args){
    rq_create_delete_ums_scheduler_args_t rq_args_san;
    ums_process_t* ums_process;
    pid_t tgid;
    pid_t pid;

    ums_scheduler_t* ums_scheduler;
    ums_scheduler_sl_t* ums_scheduler_sl;

    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    tgid = current->tgid;
    pid = current->pid;

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;
    
    ums_process_get_scheduler_sl(ums_process, pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL))
        return -ERR_INTERNAL;

    ums_process_remove_scheduler_sl(ums_process, ums_scheduler_sl);

    ums_scheduler_sl_remove_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL))
        return -ERR_INTERNAL;  // someone else is removing the scheduler
                    // this should never happen
    
    // flag to stop while() loop in main function of the scheduler
    ums_scheduler->entry_point_args->reason = REASON_SPECIAL_END_SCHEDULER;
    // return value of the scheduler
    ums_scheduler->entry_point_args->activation_payload = rq_args_san.return_value;

    DESTROY_UMS_SCHEDULER(ums_scheduler);
    kfree(ums_scheduler);

    DESTROY_UMS_SCHEDULER_SL(ums_scheduler_sl);
    kfree(ums_scheduler_sl);
    return SUCCESS;
}
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
/**
 * Request used to pause the execution of the current scheduler
 * 
 * @param args Arguments of the request (provided by user), currently NOT USED
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_wait_next_scheduler_call(rq_wait_next_scheduler_call_args_t* rq_args){
    rq_wait_next_scheduler_call_args_t rq_args_san;
    
    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;
    rq_args_san.ucd = 0;   //useless
    
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    return SUCCESS;
}
// ------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------
/**
 * Request used by the scheduler to execute the next available ums_context in the ums_completion_list
 * 
 * @param args Arguments of the request (provided by user)
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_execute_next_new_thread(rq_execute_next_new_thread_args_t* rq_args){
    rq_execute_next_new_thread_args_t rq_args_san;
    ums_process_t* ums_process;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;
    ums_completion_list_sl_t* ums_completion_list_sl; 
    ums_completion_list_item_t* cl_item;

    ums_context_sl_t* ums_context_sl;
    bool context_assigned;

    ums_context_t* ums_context;
    
    pid_t tgid;
    pid_t pid;

    int ret = 0;
    
    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    tgid = current->tgid;   // indicates the process
    pid = current->pid; // indicates the scheduler

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL)){
        return -ERR_INTERNAL;
    }

    ums_process_get_scheduler_sl(ums_process, pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL)){
        return -ERR_INTERNAL;
    }

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ret = -ERR_INTERNAL;  
        goto unlock;
    }


    ums_completion_list_sl = ums_scheduler->completion_list;

    while(1){
        ums_completion_list_remove_first(ums_completion_list_sl, cl_item);
        if(unlikely(cl_item == NULL)){  //EMPTY
            printk("Empty completion list\n");
            ret = -ERR_EMPTY_COMP_LIST;
            goto unlock;
        }

        ums_process_get_ums_context_sl(ums_process, cl_item->ums_context_id, ums_context_sl);
        if(unlikely(ums_context_sl==NULL)){
            ret = -ERR_INTERNAL;
            goto unlock;
        }
        ums_context_sl_try_to_acquire(ums_context_sl, &context_assigned);
        if(likely(context_assigned==true)){    //success
            rq_args_san.routine = ums_context_sl->ums_context->routine;
            rq_args_san.args = ums_context_sl->ums_context->args;
            rq_args_san.ucd = ums_context_sl->id;
            rq_args_san.pid_scheduler = pid;
            rq_args_san.cpu_core = ums_scheduler->cpu_core;

            ret = 0;

            if(copy_to_user(rq_args, &rq_args_san, sizeof(rq_args_san)))
                ret = -EFAULT;

            ums_context = ums_context_sl->ums_context;
            ums_context_update_run_time_start_slot(ums_context);
            goto unlock;
        }
        else{
            printk("pid=%d, ums context already assigned :(. I try to get the next one\n", current->pid);
        }
    }

unlock:    
    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
    return ret;
}

/**
 * Request used by the scheduler to execute the next ums_context in the ready list
 * 
 * @param args Arguments of the request (provided by user)
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_execute_next_ready_thread(rq_execute_next_ready_thread_args_t* rq_args){
    rq_execute_next_ready_thread_args_t rq_args_san;
    ums_process_t* ums_process;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;
    ums_context_t* ums_context;

    pid_t pid;
    pid_t tgid;

    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    pid = current->pid;
    tgid = current->tgid;

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;
    
    ums_process_get_scheduler_sl(ums_process, pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL))
        return -ERR_INTERNAL;

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL; 
    }

    ums_scheduler_ready_list_remove_first(ums_scheduler, ums_context);
    if(unlikely(ums_context == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_EMPTY_READY_LIST; 
    }

    ums_context_update_run_time_start_slot(ums_context);

    ums_scheduler->num_switch += 1;
    ums_scheduler->running_thread = ums_context;    
    ums_context->state = UMS_THREAD_STATE_RUNNING;

    while(!wake_up_process(ums_context->task_struct));
    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
    
    return 0;
}
// --------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/**
 * Request executed at startup of a ums_context
 * 
 * @param args Arguments of the request (provided by user)
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_startup_new_thread(rq_startup_new_thread_args_t* rq_args){
    rq_startup_new_thread_args_t rq_args_san;
    ums_process_t* ums_process;
    ums_context_sl_t* ums_context_sl;
    ums_context_t* ums_context;

    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;

    pid_t tgid;
    pid_t pid;

    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    tgid = current->tgid;
    pid = current->pid;

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;
    
    ums_process_get_scheduler_sl(ums_process, rq_args_san.pid_scheduler, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL))
        return -ERR_INTERNAL;

    ums_process_get_ums_context_sl(ums_process, rq_args_san.ucd, ums_context_sl);
    if(unlikely(ums_context_sl==NULL)){
        return -ERR_INTERNAL;
    }
    ums_context = ums_context_sl->ums_context;

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;  //should be a KERNEL PANIC
    }

    // fill fields of ums_context that represent an actual thread
    ums_context_register_as_thread(ums_context, current, rq_args_san.pid_scheduler);
    // register the new ums_context in the hashmap that map pid->ucd    
    ums_process_register_ums_thread(ums_process, ums_context);

    // register the new thread in /proc
    ums_proc_add_thread(ums_scheduler_sl->proc_entry_main_workers, ums_context->id, ums_context->proc_entry);

    ums_context_update_run_time_start_slot(ums_context);

    ums_scheduler->running_thread = ums_context;
    ums_context->state = UMS_THREAD_STATE_RUNNING;
    ums_context->num_switch += 1;

    ums_scheduler->num_switch += 1;

    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);

    return 0;
}

/**
 * Request used by a ums_context to terminate its execution
 * 
 * @param args Arguments of the request (provided by user)
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_end_thread(rq_end_thread_args_t* rq_args){
    rq_end_thread_args_t rq_args_san;
    ums_process_t* ums_process;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;
    ums_context_sl_t* ums_context_sl;
    ums_context_t* ums_context;

    struct task_struct* sched_ts;

    pid_t pid;
    pid_t tgid;

    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    pid = current->pid;
    tgid = current->tgid;

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL)){
        //printk(KERN_ALERT "invalid ums_process");
        return -ERR_INTERNAL;
    }

    ums_process_get_scheduler_sl(ums_process, rq_args_san.pid_scheduler, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL)){
        //printk(KERN_ALERT "invalid ums_scheduler_sl");
        return -ERR_INTERNAL;
    }

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        //printk(KERN_ALERT "invalid ums_scheduler");
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;
    }

    ums_process_get_ums_context_sl(ums_process, rq_args_san.ucd, ums_context_sl);
    if(unlikely(ums_context_sl == NULL))
        return -ERR_INTERNAL;

    ums_context = ums_context_sl->ums_context;
    ums_context_update_run_time_end_slot(ums_context);

    ums_context_sl->ums_context->state = UMS_THREAD_STATE_ENDED;
    //ums_context_sl->assigned = false; //release
    ums_context_sl_set_assigned(ums_context_sl, false); //release

    sched_ts = ums_scheduler->scheduler_task_struct;

    ums_scheduler->entry_point_args->reason = REASON_THREAD_ENDED;
    ums_scheduler->entry_point_args->activation_payload = rq_args_san.ucd;
    
    ums_scheduler->num_switch += 1;
    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);

    while(!wake_up_process(sched_ts));
    schedule();

    return 0;
}
// -------------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------
/**
 * @brief get a list of contexts from the completion_list
 * 
 */
static inline int rq_get_from_cl(rq_get_from_cl_args_t* rq_args){
    rq_get_from_cl_args_t rq_args_san;

    ums_process_t* ums_process;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;
    ums_completion_list_sl_t* ums_completion_list_sl; 
    ums_completion_list_item_t* cl_item;


    ums_context_sl_t* ums_context_sl;

    ums_context_t* ums_context;
    
    info_ums_context_t* array_info_context;

    pid_t tgid;
    pid_t pid;

    int idx;
    int ret = 0;
    
    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    tgid = current->tgid;   // indicates the process
    pid = current->pid; // indicates the scheduler

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL)){
        return -ERR_INTERNAL;
    }

    ums_process_get_scheduler_sl(ums_process, pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL)){
        return -ERR_INTERNAL;
    }

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ret = -ERR_INTERNAL;  
        goto unlock;
    }

    ums_completion_list_sl = ums_scheduler->completion_list;

    ums_scheduler_completion_list_start_iteration(ums_scheduler, cl_item);

    if(ums_scheduler_list_empty(&(ums_completion_list_sl->ums_context_list))){
        ums_scheduler_completion_list_iterate_end(ums_scheduler);
        ret = -ERR_EMPTY_COMP_LIST;
        goto unlock;
    }

    array_info_context = kmalloc(rq_args_san.array_size*sizeof(info_ums_context_t), GFP_KERNEL);
    for(idx=0; idx < rq_args_san.array_size; idx++){
        if(likely(cl_item != NULL)){   
            ums_process_get_ums_context_sl(ums_process, cl_item->ums_context_id, ums_context_sl);
            ums_context = ums_context_sl->ums_context;

            array_info_context[idx].ucd = ums_context->id;
            array_info_context[idx].number_switch = ums_context->num_switch;
            array_info_context[idx].run_time_ms = ums_context_get_run_time_ms(ums_context);
            array_info_context[idx].user_reserved = ums_context->user_reserved;
            array_info_context[idx].from_cl = true;

            ums_scheduler_completion_list_iterate(ums_scheduler, cl_item);
        }
        else 
            break;
    }

    if(copy_to_user(rq_args_san.info_context_array, array_info_context, idx*sizeof(info_ums_context_t)))
        ret = -EFAULT;
    
    ret = idx;
    kfree(array_info_context);

unlock:
    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
    
    return ret;
}

/**
 * @brief get list of ums_context from the ready list
 * 
 */
static inline int rq_get_from_rl(rq_get_from_rl_args_t* rq_args){
    rq_get_from_rl_args_t rq_args_san;

    ums_process_t* ums_process;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;


    ums_context_t* ums_context;
    
    info_ums_context_t* array_info_context;

    pid_t tgid;
    pid_t pid;

    int idx;
    int ret = 0;
    
    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    tgid = current->tgid;   // indicates the process
    pid = current->pid; // indicates the scheduler

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL)){
        return -ERR_INTERNAL;
    }

    ums_process_get_scheduler_sl(ums_process, pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL)){
        return -ERR_INTERNAL;
    }

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ret = -ERR_INTERNAL;  
        goto unlock;
    }


    ums_scheduler_ready_list_start_iteration(ums_scheduler, ums_context);

    if(ums_scheduler_list_empty(&(ums_scheduler->ready_list))){
        ums_scheduler_ready_list_iterate_end(ums_scheduler);
        ret = -ERR_EMPTY_READY_LIST;
        goto unlock;
    }

    array_info_context = kmalloc(rq_args_san.array_size*sizeof(info_ums_context_t), GFP_KERNEL);
    for(idx=0; idx < rq_args_san.array_size; idx++){
        if(likely(ums_context != NULL)){   

            array_info_context[idx].ucd = ums_context->id;
            array_info_context[idx].number_switch = ums_context->num_switch;
            array_info_context[idx].run_time_ms = ums_context_get_run_time_ms(ums_context);
            array_info_context[idx].user_reserved = ums_context->user_reserved;
            array_info_context[idx].from_cl = false;

            ums_scheduler_ready_list_iterate(ums_scheduler, ums_context);
        }
        else 
            break;
    }

    if(copy_to_user(rq_args_san.info_context_array, array_info_context, idx*sizeof(info_ums_context_t))){
        ret = -EFAULT;
    }
    else{
        ret = idx;
    }
    kfree(array_info_context);

unlock:
    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
    
    return ret;
}

/**
 * @brief execute a context extracted by the completion_list
 */
static inline int rq_execute(rq_execute_args_t* rq_args){
    rq_execute_args_t rq_args_san;

    info_ums_context_t info_san;
    
    ums_process_t* ums_process;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;

    ums_context_sl_t* ums_context_sl;
    bool context_assigned;

    ums_context_t* ums_context;
    ums_completion_list_item_t* ums_completion_list_item;

    pid_t tgid;
    pid_t pid;
    int ret = 0;
    
    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    if(copy_from_user(&info_san, rq_args_san.info_context, sizeof(info_san)))
        return -EFAULT;


    tgid = current->tgid;   // indicates the process
    pid = current->pid; // indicates the scheduler

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL)){
        return -ERR_INTERNAL;
    }

    ums_process_get_ums_context_sl(ums_process, rq_args_san.info_context->ucd, ums_context_sl);
    if(unlikely(ums_context_sl==NULL)){
        return -ERR_INTERNAL;
    }

    ums_process_get_scheduler_sl(ums_process, pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL)){
        return -ERR_INTERNAL;
    }

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;  
    }

    ums_context_sl_try_to_acquire(ums_context_sl, &context_assigned);
    if(likely(context_assigned==true)){ // ums_context acquired
        rq_args_san.routine = ums_context_sl->ums_context->routine;
        rq_args_san.args = ums_context_sl->ums_context->args;
        rq_args_san.ucd = ums_context_sl->id;
        rq_args_san.pid_scheduler = pid;

        rq_args_san.cpu_core = ums_scheduler->cpu_core;

        if(copy_to_user(rq_args, &rq_args_san, sizeof(rq_args_san)))
            return -EFAULT;

        ums_context = ums_context_sl->ums_context;
        ums_context_update_run_time_start_slot(ums_context);
    }
    else{
        printk("pid= %d exec, ums context already assigned :( \n", current->pid);
        ret = -ERR_ASSIGNED;
    }

    if(info_san.from_cl){
        ums_completion_list_remove_item_by_descriptor_no_sl(ums_scheduler->completion_list, ums_context_sl->id, ums_completion_list_item);
        ums_scheduler_completion_list_iterate_end(ums_scheduler);
    }
    else{
        printk("ERROR from_cl");
       // ums_scheduler_ready_list_iterate_end(ums_scheduler);
    }

    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
    return ret;
}
// -----------------------------------------------------------------------------------------------

/**
 * @brief execute a context extracted from the ready list
 * 
 * @param rq_args 
 * @return int 
 */
static inline int rq_execute_ready_list(rq_execute_args_t* rq_args){
    rq_execute_args_t rq_args_san;

    info_ums_context_t info_san;
    
    ums_process_t* ums_process;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;

    ums_context_sl_t* ums_context_sl;

    ums_context_t* ums_context;
    //ums_completion_list_item_t* ums_completion_list_item;

    pid_t tgid;
    pid_t pid;
    
    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    if(copy_from_user(&info_san, rq_args_san.info_context, sizeof(info_san)))
        return -EFAULT;


    tgid = current->tgid;   // indicates the process
    pid = current->pid; // indicates the scheduler

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL)){
        return -ERR_INTERNAL;
    }

    ums_process_get_scheduler_sl(ums_process, pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL)){
        return -ERR_INTERNAL;
    }

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;  
    }

    ums_process_get_ums_context_sl(ums_process, rq_args_san.info_context->ucd, ums_context_sl);
    if(unlikely(ums_context_sl==NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;
    }

    ums_context = ums_context_sl->ums_context;
    
    if(info_san.from_cl){
        printk("ERROR from_cl");
        //ums_completion_list_remove_item_by_descriptor_no_sl(ums_scheduler->completion_list, ums_context_sl->id, ums_completion_list_item);
        //ums_scheduler_completion_list_iterate_end(ums_scheduler);
    }
    else{
        ums_scheduler_ready_list_remove(ums_scheduler, ums_context);
        ums_scheduler_ready_list_iterate_end(ums_scheduler);
    }

    ums_context_update_run_time_start_slot(ums_context);

    ums_scheduler->num_switch += 1;
    ums_scheduler->running_thread = ums_context;    
    ums_context->state = UMS_THREAD_STATE_RUNNING;

    while(!wake_up_process(ums_context->task_struct));
    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
    return 0;
}
// -----------------------------------------------------------------------------------------------

