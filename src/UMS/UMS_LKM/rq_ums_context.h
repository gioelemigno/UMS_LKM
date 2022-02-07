#pragma once
/// @file 
/// This file contains all the requests used to manage a ums_context
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


//-----------------------------------------------------------------------------------------
/**
 * Request used to create a new ums_context
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_create_ums_context(rq_create_delete_ums_context_args_t* args){
    rq_create_delete_ums_context_args_t args_san;
    ums_context_t* ums_context;
    ums_context_sl_t* ums_context_sl;
    ums_process_t* ums_process;

    if(copy_from_user(&args_san, args, sizeof(args_san)))
        return -EFAULT;

    ums_context = kmalloc(sizeof(ums_context_t), GFP_KERNEL);
    if(likely(ums_context)) 
        INIT_UMS_CONTEXT(ums_context, args_san.routine, args_san.args);
    else    
        return -ERR_INTERNAL;
    
    ums_context->user_reserved = args_san.user_res;
    
    ums_context_sl = kmalloc(sizeof(ums_context_sl_t), GFP_KERNEL);
    if(likely(ums_context_sl))
        INIT_UMS_CONTEXT_SL(ums_context_sl, ums_context);
    else
        return -ERR_INTERNAL;

    ums_hashtable_get_process(args_san.tgid, ums_process);
    if(likely(ums_process)){
        ums_process_add_ums_context_sl(ums_process, ums_context_sl);
        args_san.descriptor = ums_context_sl->id;
        
        if(copy_to_user(args, &args_san, sizeof(args_san)))
            return -EFAULT;
        
        return 0;
    }
    else
        return -ERR_INTERNAL;
}

/**
 * Request used to delete a ums_context
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_delete_ums_context(rq_create_delete_ums_context_args_t* args){
    rq_create_delete_ums_context_args_t args_san;
    ums_process_t* ums_process;
    ums_context_sl_t* ums_context_sl;
    bool assigned;
    ums_context_t* ums_context;

    if(copy_from_user(&args_san, args, sizeof(args_san)))
        return -EFAULT;

    ums_hashtable_get_process(args_san.tgid, ums_process);

    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;

    ums_process_get_ums_context_sl(ums_process, args_san.descriptor, ums_context_sl);

    assigned = true;
    ums_context_sl_get_assigned(ums_context_sl, &assigned); 
    if(unlikely(assigned))  // someone is using it! We cannot delete it
        return -ERR_INTERNAL;
    
    ums_process_remove_ums_context_sl(ums_process, ums_context_sl);
    ums_context = ums_context_sl->ums_context;
    
    ums_proc_remove_thread(ums_context->proc_entry);

    DESTROY_UMS_CONTEXT(ums_context);
    kfree(ums_context);

    DESTROY_UMS_CONTEXT_SL(ums_context_sl);
    kfree(ums_context_sl);

    return 0;
}
// ---------------------------------------------------------------------------------------

// ----------------------------------------------------------------------------------------
/**
 * Request used by a ums thread to yield the control to the scheduler
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_yield_ums_context(rq_yield_ums_context_args_t* args){
    rq_yield_ums_context_args_t args_san;
    ums_process_t* ums_process;
    ums_context_t* ums_context;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;

    pid_t pid;
    pid_t tgid;

    if(copy_from_user(&args_san, args, sizeof(args_san)))
        return -EFAULT;

    pid = current->pid;
    tgid = current->tgid;

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;

    ums_process_get_ums_thread(ums_process, pid, ums_context);
    if(unlikely(ums_context == NULL))
        return -ERR_INTERNAL;

    ums_process_get_scheduler_sl(ums_process, ums_context->pid_scheduler, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL))
        return -ERR_INTERNAL;

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;
    }

    ums_scheduler->running_thread = NULL;
    
    ums_context_update_run_time_end_slot(ums_context);

    ums_context->state = UMS_THREAD_STATE_IDLE;
    ums_context->num_switch += 1;
    ums_scheduler_ready_list_add(ums_scheduler, ums_context);

    // prepare arguments for the next call of entry_point function
    ums_scheduler->entry_point_args->reason = REASON_THREAD_YIELD;
    ums_scheduler->entry_point_args->activation_payload = ums_context->id;

    set_current_state(TASK_INTERRUPTIBLE);
    while(!wake_up_process(ums_scheduler->scheduler_task_struct));

    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
    
    schedule();

    return 0;
}
// ---------------------------------------------------------------------------------------