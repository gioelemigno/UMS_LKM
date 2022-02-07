#pragma once

/** @file 
* This file contains definitions and functions of objects related to a ums_context
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/hashtable.h>
#include <linux/spinlock.h>
#include <stdbool.h>
#include <linux/list.h>
#include <linux/rwlock.h>

#include "../common/ums_types.h"

#include <linux/proc_fs.h>
#include <linux/jiffies.h>


#define UMS_THREAD_STATE_IDLE       0
#define UMS_THREAD_STATE_RUNNING    1
#define UMS_THREAD_STATE_ENDED      2
// ums_context_t ########################################################################################
/**
 * @brief Represents a ums_context
 * 
 */
typedef struct ums_context_t{
    struct list_head list; /** used to arrange ums_context in ready_list */
    struct hlist_node hlist; /** used by the hashtable of ums_threads, used to map thread's pid to the ums_context_descriptor*/
    
    pid_t pid;  /** thread's pid used*/
    int id; /** descriptor */
    void* task_struct;  /** pointer to task_struct of thread used */
    pid_t pid_scheduler;    /** pid of the scheduler that manage the ums_context */

    struct proc_dir_entry* proc_entry; /** entry in /proc associated to this ums_context*/
    int num_switch; /** number of switches from running to idle and viceversa */
    int state; /** state of the ums_context: UMS_THREAD_STATE_IDLE, UMS_THREAD_STATE_RUNNING, UMS_THREAD_STATE_ENDED*/
  
    void* (*routine)(void* args);   /** routine of the user */
    void* args; /** args of user's routine */

    void* user_reserved; /** user can use it as he wants, (e.g. store some characteristics of the ums_context:  CPU or I/O BURST, and prio ) */

    u64 start_time_last_slot; /** uses jiffies */
    u64 ums_run_time;   /** uses jiffies */
}ums_context_t;

// -------------------------------------------------------------------
/**
 * @brief ums_context's constructor
 * 
 * @param p_ums_context pointer to a NON-NULL ums_context
 * @param p_routine pointer to user's routine (type:  void* (*routine)(void* args))
 * @param p_args args for user's routine
 * 
 */
#define INIT_UMS_CONTEXT(p_ums_context, p_routine, p_args)   \
    do{ \
        (p_ums_context)->id = -1;    \
        (p_ums_context)->task_struct = NULL;    \
        (p_ums_context)->routine = p_routine;   \
        (p_ums_context)->args = p_args; \
        (p_ums_context)->proc_entry = NULL; \
        (p_ums_context)->num_switch = 0; \
        (p_ums_context)->state = UMS_THREAD_STATE_IDLE; \
        (p_ums_context)->ums_run_time = 0; \
        (p_ums_context)->start_time_last_slot = 0; \
    }while(0)

/**
 * @brief ums_context's destructor
 * 
 * @param p_ums_context pointer to a NON-NULL ums_context
 * 
 */ 
#define DESTROY_UMS_CONTEXT(p_ums_context)   \
    do{ \
        (p_ums_context)->id = -1;    \
        (p_ums_context)->task_struct = NULL;    \
        (p_ums_context)->routine = NULL;   \
        (p_ums_context)->args = NULL; \
        (p_ums_context)->proc_entry = NULL; \
        (p_ums_context)->num_switch = 0; \
        (p_ums_context)->state = UMS_THREAD_STATE_IDLE; \
        (p_ums_context)->ums_run_time = 0; \
        (p_ums_context)->start_time_last_slot = 0; \
    }while(0)
// --------------------------------------------------------

/**
 * @brief Register information about the actual thread used by the ums_context
 * 
 * @param p_ums_context pointer to a NON-NULL ums_context
 * @param p_task_struct pointer to thread's task_struct
 * @param pid_sched pid of the scheduler that manages the ums_context
 * 
 */
#define ums_context_register_as_thread(p_ums_context, p_task_struct, pid_sched) \
    do{ \
        (p_ums_context)->task_struct = p_task_struct;   \
        (p_ums_context)->pid = (p_task_struct)->pid;    \
        (p_ums_context)->pid_scheduler = pid_sched; \
    }while(0)

/**
 * @brief Unregister information about the actual thread used by the ums_context
 * 
 * @param p_ums_context pointer to a NON-NULL ums_context
 */
#define ums_context_unregister_as_thread(p_ums_context) \
    do{ \
        (p_ums_context)->task_struct = NULL;   \
        (p_ums_context)->pid = 0;    \
        (p_ums_context)->pid_scheduler = 0; \
    }while(0)

static inline char* _ums_context_printable_state(ums_context_t* uc){
    switch(uc->state){
        case UMS_THREAD_STATE_RUNNING:
            return "running";
        break;
        case UMS_THREAD_STATE_ENDED:
            return "ended";
        break;
        case UMS_THREAD_STATE_IDLE:
            return "idle";
        break;

        default:
            return "unknown";
        break;
    }
}


/**
 * @brief State of a ums_context as string
 * 
 * @param p_ums_context Pointer to a NON-NULL ums_context 
 * @return State as a human-readable string
 */
#define ums_context_printable_state(p_ums_context)  \
    _ums_context_printable_state(p_ums_context)

// -------------------------------------------------------------------
// ######################################################################################################


// ums_context_sl_t ########################################################################################
/**
 * @brief ums_context_SpinLock is used to protect a ums_context between several ums_schedulers
 * 
 */
typedef struct ums_context_sl_t{
    int id; /** descriptor, the same of the ums_context managed*/
    
    bool assigned; /** (IN USE) indicates the managed ums_context has been already assigned to another scheduler*/
    spinlock_t assigned_spin_lock; /** used to protect "assigned" field */
    
    ums_context_t* ums_context; /** pointer to the ums_context managed*/
}ums_context_sl_t;

// -------------------------------------------------------------------
/**
 * @brief ums_context_sl constructor
 * 
 * @param p_ums_context_sl NON-NULL ums_context_sl pointer
 * @param p_ums_context_in descriptor of the ums_context managed
 */
#define INIT_UMS_CONTEXT_SL(p_ums_context_sl, p_ums_context_in)   \
    do{ \
        (p_ums_context_sl)->id = -1; \
        \
        (p_ums_context_sl)->assigned = false;   \
        spin_lock_init(&(p_ums_context_sl)->assigned_spin_lock);  \
        \
        (p_ums_context_sl)->ums_context = p_ums_context_in;   \
    }while(0)

/**
 * @brief ums_context_sl destructor
 * 
 * @param p_ums_context_sl NON-NULL ums_context_sl pointer
 */
#define DESTROY_UMS_CONTEXT_SL(p_ums_context_sl)   \
    do{ \
        (p_ums_context_sl)->id = -1; \
        \
        (p_ums_context_sl)->assigned = false;   \
        \
        (p_ums_context_sl)->ums_context = NULL;   \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief set "assigned" field of a ums_context_sl in a secure way 
 * 
 * @param p_ums_context_sl NON-NULL ums_context_sl
 * @param assigned_in boolean value to set
 */
#define ums_context_sl_set_assigned(p_ums_context_sl, assigned_in)   \
    do{\
        spin_lock(&((p_ums_context_sl)->assigned_spin_lock)); \
        (p_ums_context_sl)->assigned = assigned_in;    \
        spin_unlock(&((p_ums_context_sl)->assigned_spin_lock));   \
    }while(0)

/**
 * @brief read "assigned" field of a ums_context_sl in a secure way
 * 
 * @param p_ums_context_sl NON-NULL ums_context_sl
 * @param p_assigned output, pointer to a bool
 * 
 */
#define ums_context_sl_get_assigned(p_ums_context_sl, p_assigned)   \
    do{\
        spin_lock(&((p_ums_context_sl)->assigned_spin_lock)); \
        *(p_assigned) = (p_ums_context_sl)->assigned;    \
        spin_unlock(&((p_ums_context_sl)->assigned_spin_lock));   \
    }while(0)
// -------------------------------------------------------------------

/**
 * @brief try to set "assigned" field of a ums_context_sl 
 * 
 * @param p_ums_context_sl NON-NULL ums_context_sl
 * @param p_res output, true if the context has been acquired, false otherwise
 * 
 */
#define ums_context_sl_try_to_acquire(p_ums_context_sl, p_res)   \
    do{\
        spin_lock(&((p_ums_context_sl)->assigned_spin_lock)); \
        if(likely((p_ums_context_sl)->assigned == false)){  \
            (p_ums_context_sl)->assigned = true;    \
            *(p_res) = true;    \
        }   \
        else    \
            *(p_res) = false;   \
        spin_unlock(&((p_ums_context_sl)->assigned_spin_lock));   \
    }while(0)
// -------------------------------------------------------------------


// ---------------------------------------------------------------------
/**
 * @brief update "ums_run_time" field of the ums_context 
 * To be called at the beginning of the slot
 * 
 * @param p_ums_context NON-NULL pointer to ums_context
 */
#define ums_context_update_run_time_start_slot(p_ums_context)  \
    do{ \
        (p_ums_context)->start_time_last_slot = get_jiffies_64();    \
    }while(0)

/**
 * @brief update "ums_run_time" field of the ums_context 
 * To be called at the end of the slot
 * 
 * @param p_ums_context NON-NULL pointer to ums_context
 */
#define ums_context_update_run_time_end_slot(p_ums_context)  \
    do{ \
        u64 time_now = get_jiffies_64();    \
        (p_ums_context)->ums_run_time += time_now-(p_ums_context)->start_time_last_slot; \
        (p_ums_context)->start_time_last_slot = 0; \
    }while(0)
// ---------------------------------------------------------------------

// --------------------------------------------------------------------
/**
 * @brief get run time of the ums_context in milliseconds
 * 
 * @param p_ums_context NON-NULL pointer to ums_context
 * 
 * @return run time as unsigned int 
 * 
 */
#define ums_context_get_run_time_ms(p_ums_context)  \
    jiffies_to_msecs((p_ums_context)->ums_run_time)
// --------------------------------------------------------------------
// ######################################################################################################



// -------------------------------------------------------------------------------------------------------------
/**
 * @brief snprintf useful to print a ums_context
 *  
 */
static inline int snprintf_ums_context(char* buff, ssize_t size_buff, ums_context_t* ums_context){
    return snprintf(buff, size_buff,
        "\t\t\t\t\tid=%d\n" 
        "\t\t\t\t\ttask_struct=%p\n"
        "\t\t\t\t\troutine=%lx\n"
        "\t\t\t\t\targs=%lx\n"
        , 
        ums_context->id,
        ums_context->task_struct,
        (unsigned long) ums_context->routine,
        (unsigned long) ums_context->args
        );
}

/**
 * @brief printK a ums_context
 * 
 */
#define PRINTK_UMS_CONTEXT(p_obj, PREFIX)   \
    do{ \
        char* __buff = kmalloc(64, GFP_KERNEL);     \
        if(snprintf_ums_context(__buff, 64, p_obj) > 64-4) \
            printk(KERN_DEBUG "\nPRINTK_UMS_CONTEXT overflow!!!\n");    \
        else printk(PREFIX "ums_context_t = \n{\n%s}\n", __buff);  \
        kfree(__buff);  \
    }while(0)

// -------------------------------------------------------------------------------------------------------------

/**
 * @brief snprintf useful to print a ums_context_sl
 * 
 */
static inline int snprintf_ums_context_sl(char* buff, ssize_t size_buff, ums_context_sl_t* ums_context_sl){
    bool __assigned; 
    int res;
    char* __buff_ums_context = kmalloc(164, GFP_KERNEL);

    spin_lock(&ums_context_sl->assigned_spin_lock);
        __assigned = ums_context_sl->assigned;
    spin_unlock(&ums_context_sl->assigned_spin_lock);
    
    if( snprintf_ums_context(__buff_ums_context, 164, ums_context_sl->ums_context) > 164-4){
        printk(KERN_DEBUG "\n snprintf_ums_context_sl() overflow!!!\n");    
    }
    res = snprintf(buff, size_buff, 
        "\t\tid=%d\n\t\tassigned=%d\n\t\tums_context=\n\t\t{\n%s\t\t}\n", 
        ums_context_sl->id, __assigned, __buff_ums_context);
    kfree(__buff_ums_context);
    return res;
}

/**
 * @brief printK ums_context_sl
 * 
 */
#define PRINTK_UMS_CONTEXT_SL(p_obj, PREFIX)   \
    do{ \
        char* __buff = kmalloc(512, GFP_KERNEL);    \
        if( snprintf_ums_context_sl(__buff, 512, p_obj) > 512-4) \
            printk(KERN_DEBUG "\n PRINTK_UMS_CONTEXT_SL() overflow!!!\n");    \
        else  printk(PREFIX "ums_context_sl_t = \n{\n%s}\n", __buff);   \
        kfree(__buff);  \
    }while(0)
// ----------------------------------------------------------------------------
