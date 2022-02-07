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
#include "rq_ums_process.h"
#include "rq_ums_completion_list.h"
#include "rq_ums_scheduler.h"
#include "rq_ums_context.h"

#include "ums_proc.h"
struct proc_dir_entry* ums_proc_ums_folder;

#define MODULE_NAME_LOG "UMS Log: "

#define DEVICE_NAME "UMS"
#define BUF_LEN 80

#define DEBUG_REQUEST

MODULE_LICENSE("GPL");

int init_module(void);
void cleanup_module(void);
static long ums_ioctl(struct file *file, unsigned int request, unsigned long data);



static struct file_operations fops = {
    .unlocked_ioctl = ums_ioctl
};

static struct miscdevice mdev = {
    .minor = 0,
    .name = DEVICE_NAME,
    .mode = S_IALLUGO,
    .fops = &fops
};




static long ums_ioctl(struct file *file, unsigned int request, unsigned long data){
    // Each request will use copy_from_user and copy_to_user if needed
    int res;
    switch(request){
        // ---------------------------------------------------------------------
        case RQ_CREATE_PROCESS:
            res = rq_create_process((rq_create_delete_process_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "Created new process: res=%d\n", res);
        #endif
        break;
        
        case RQ_DELETE_PROCESS:
            res = rq_delete_process((rq_create_delete_process_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "Deleted a process: res=%d\n", res);
        #endif
        break;
        // ---------------------------------------------------------------------
        case RQ_CREATE_UMS_CONTEXT:
            res = rq_create_ums_context((rq_create_delete_ums_context_args_t*)data);
        #ifdef DEBUG_REQUEST            
            printk(KERN_DEBUG "Created new ums_context: res=%d\n", res);
        #endif
        break;

        case RQ_DELETE_UMS_CONTEXT:
            res = rq_delete_ums_context((rq_create_delete_ums_context_args_t*)data);
        #ifdef DEBUG_REQUEST            
            printk(KERN_DEBUG "Deleted a ums_context: res=%d\n", res);
        #endif
        break;
        // ---------------------------------------------------------------------

       // ---------------------------------------------------------------------
        case RQ_CREATE_COMPLETION_LIST:
            res = rq_create_completion_list((rq_create_delete_completion_list_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "Created new completion list: res=%d\n", res);
        #endif
        break;

        case RQ_DELETE_COMPLETION_LIST:
            res = rq_delete_completion_list((rq_create_delete_completion_list_args_t*)data);
        #ifdef DEBUG_REQUEST            
            printk(KERN_DEBUG "Deleted a completion list: res=%d\n", res);
        #endif
        break;
        // ---------------------------------------------------------------------

        case RQ_COMPLETION_LIST_ADD_UMS_CONTEXT:
            res = rq_completion_list_add_ums_context((rq_completion_list_add_remove_ums_context_args_t*)data);
        #ifdef DEBUG_REQUEST            
            printk(KERN_DEBUG "Add ums_context=%d to the completion_list=%d: res=%d\n", 
                            ((rq_completion_list_add_remove_ums_context_args_t*)data)->ums_context_d,
                            ((rq_completion_list_add_remove_ums_context_args_t*)data)->completion_list_d,
                            res);
        #endif
        break;

        case RQ_COMPLETION_LIST_REMOVE_UMS_CONTEXT:
            res = rq_completion_list_remove_ums_context((rq_completion_list_add_remove_ums_context_args_t*)data);
        #ifdef DEBUG_REQUEST            
            printk(KERN_DEBUG "Removed ums_context=%d from the completion_list=%d: res=%d\n", 
                            ((rq_completion_list_add_remove_ums_context_args_t*)data)->ums_context_d,
                            ((rq_completion_list_add_remove_ums_context_args_t*)data)->completion_list_d,
                            res);
        #endif
        break;

        case RQ_CREATE_UMS_SCHEDULER:
            res = rq_create_ums_scheduler((rq_create_delete_ums_scheduler_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "Created a new scheduler: res=%d\n", res);
        #endif
        break;

        case RQ_EXIT_UMS_SCHEDULER:
            res = rq_exit_ums_scheduler((rq_create_delete_ums_scheduler_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "Deleted a scheduler: res=%d\n", res);
        #endif
        break;

        case RQ_EXECUTE_NEXT_NEW_THREAD:
            res = rq_execute_next_new_thread((rq_execute_next_new_thread_args_t*)data);
        #ifdef DEBUG_REQUEST          
            printk(KERN_DEBUG "Execute next new thread: res=%d\n", res);
        #endif
        break;

        case RQ_STARTUP_NEW_THREAD:
            res = rq_startup_new_thread((rq_startup_new_thread_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "Startup new thread: res=%d\n", res);
        #endif
        break;

        case RQ_END_THREAD:
            res = rq_end_thread((rq_end_thread_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "End thread: res=%d\n", res);
        #endif
        break;

        case RQ_WAIT_NEXT_SCHEDULER_CALL:
            res = rq_wait_next_scheduler_call((rq_wait_next_scheduler_call_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "rq_wait_next_scheduler_call: res=%d\n", res);
        #endif
        break;

        case RQ_YIELD_UMS_CONTEXT:
            res = rq_yield_ums_context((rq_yield_ums_context_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "rq_yield_ums_context: res=%d\n", res);
        #endif       
        break;

        case RQ_EXECUTE_NEXT_READY_THREAD:
            res = rq_execute_next_ready_thread((rq_execute_next_ready_thread_args_t*)data);
        #ifdef DEBUG_REQUEST
            printk(KERN_DEBUG "rq_execute_next_ready_thread: res=%d\n", res);
        #endif       
        break;

        case RQ_GET_FROM_CL:
            res = rq_get_from_cl((rq_get_from_cl_args_t*)data);
        break;

        case RQ_EXECUTE:
            res = rq_execute((rq_execute_args_t*)data);
        break;

        case RQ_GET_FROM_RL:
            res = rq_get_from_rl((rq_get_from_rl_args_t*)data);
        break;

        case RQ_EXECUTE_READY_LIST:
            res = rq_execute_ready_list((rq_execute_args_t*)data);
        break;

        default:    //BAD REQUEST
            //kernel sets automaticaly errno by reading this return value!
            res = -EINVAL;      
        break;    
    }

    #ifdef DEBUG_PRINTK_HASHTABLE
        PRINTK_UMS_HASHTABLE(0);
    #endif

    return res;
}


int init_module(void){
    int ret;
    printk(KERN_DEBUG MODULE_NAME_LOG "init\n");

    ret = misc_register(&mdev);

    if (ret < 0){
        printk(KERN_ALERT MODULE_NAME_LOG "Registering UMS Module failed\n");
        return ret;
    }

    UMS_HASHTABLE_INIT();
    ums_proc_mount();
    //test();
    printk(KERN_DEBUG MODULE_NAME_LOG "UMS Module registered successfully\n");

    return SUCCESS;
}

void cleanup_module(void){
    ums_proc_unmount();
    misc_deregister(&mdev);
    
    
    printk(KERN_DEBUG MODULE_NAME_LOG "UMS Module un-registered successfully\n");
}













