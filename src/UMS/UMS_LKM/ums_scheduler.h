#pragma once
/// @file 
/// This file contains definitions and functions of objects related to a ums_scheduler
///

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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include <linux/slab.h>


#include "../common/ums_types.h"


#include "ums_context.h"
#include "ums_completion_lsit.h"

#include <linux/proc_fs.h>


// ums_scheduler_t ########################################################################################
/**
 * @brief object that represent a ums_scheduler
 * 
 */
typedef struct ums_scheduler_t{
    void* scheduler_task_struct;    /** task_struct of the scheduler thread */
   
    ums_completion_list_sl_t* completion_list; /** ums_completion_list managed */
    struct list_head* current_completion_list_item; /** current ums_completion_list_item during navigation of the ums_completion_list*/

    struct list_head ready_list;    /** ready list of the scheduler */
    struct list_head* current_ready_list_item; /** current ums_context during navigation of ready_list*/

    ums_context_t* running_thread; /** pointer to the current ums_context in execution*/

    entry_point_args_t* entry_point_args; /** args of the entry_point function of the scheduler*/

    int num_switch; /** number of scheduler calls*/

    int cpu_core;   /** CPU core used */
}ums_scheduler_t;

// -------------------------------------------------------------------
/**
 * @brief ums_scheduler constructor
 * 
 * @param p_ums_scheduler NON-NULL pointer to the object to init
 * @param p_scheduler_task_struct_in pointer to task_struct of the scheduler thread
 * @param p_completion_list_in pointer to the ums_completion_list_sl to manage
 */
#define INIT_UMS_SCHEDULER(p_ums_scheduler, p_scheduler_task_struct_in, p_completion_list_in)   \
    do{ \
        (p_ums_scheduler)->scheduler_task_struct = p_scheduler_task_struct_in;  \
        \
        (p_ums_scheduler)->completion_list = p_completion_list_in;  \
        (p_ums_scheduler)->current_completion_list_item = NULL; \
        \
        INIT_LIST_HEAD(&(p_ums_scheduler)->ready_list); \
        (p_ums_scheduler)->current_ready_list_item = NULL;  \
        \
        (p_ums_scheduler)->running_thread = NULL;   \
        (p_ums_scheduler)->num_switch = 0;   \
        (p_ums_scheduler)->cpu_core = -1;   \
    }while(0)

/**
 * @brief ums_scheduler deconstructor
 * 
 * @param p_ums_scheduler NON-NULL pointer to the object to destroy
 */
#define DESTROY_UMS_SCHEDULER(p_ums_scheduler)   \
    do{ \
        (p_ums_scheduler)->scheduler_task_struct = NULL;  \
        \
        (p_ums_scheduler)->completion_list = NULL;  \
        (p_ums_scheduler)->current_completion_list_item = NULL; \
        \
        (p_ums_scheduler)->current_ready_list_item = NULL;  \
        \
        (p_ums_scheduler)->running_thread = NULL;   \
        (p_ums_scheduler)->num_switch = 0;   \
        (p_ums_scheduler)->cpu_core = -1;   \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief add a ums_context to ready list of the scheduler 
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_context NON-NULL pointer to the ums_context to add
 */
#define ums_scheduler_ready_list_add(p_ums_scheduler, p_ums_context) \
    do{ \
        list_add_tail(&((p_ums_context)->list), &((p_ums_scheduler)->ready_list));  \
    }while(0)

/**
 * @brief remove a ums_context from the ready list of the scheduler 
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_context NON-NULL pointer to the ums_context to remove
 */
#define ums_scheduler_ready_list_remove(p_ums_scheduler, p_ums_context) \
    do{ \
        list_del(&((p_ums_context)->list));  \
    }while(0)
// -------------------------------------------------------------------

// --------------------------------------------------------------------------------
/**
 * @brief remove first ums_context from the ready list
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_context_OUT output, pointer to a ums_context
 */
#define ums_scheduler_ready_list_remove_first(p_ums_scheduler, p_ums_context_OUT) \
    do{ \
        p_ums_context_OUT = list_first_entry_or_null(&(p_ums_scheduler)->ready_list, ums_context_t, list);    \
        if(likely((p_ums_context_OUT) != NULL))   \
            list_del(&((p_ums_context_OUT)->list));  \
    }while(0)
// --------------------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief start to iterate the completion_list
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_completion_list_item_out output, pointer to ums_completion_item. it's NULL if the list ends
 * 
 */
#define ums_scheduler_completion_list_start_iteration(p_ums_scheduler, p_ums_completion_list_item_out)  \
    do{ \
        spin_lock(&((p_ums_scheduler)->completion_list->ums_context_list_spin_lock));    \
        (p_ums_scheduler)->current_completion_list_item = (p_ums_scheduler)->completion_list->ums_context_list.next;    \
        if(unlikely(list_empty(&((p_ums_scheduler)->completion_list->ums_context_list)))) \
            p_ums_completion_list_item_out = NULL;  \
        else    \
            p_ums_completion_list_item_out = list_entry((p_ums_scheduler)->current_completion_list_item, ums_completion_list_item_t, list);    \
    }while(0)

/**
 * @brief get current element during the iteration of the completion_list
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_completion_list_item_out output, pointer to ums_completion_item
 * 
 */
#define ums_scheduler_completion_list_iteration_get_current(p_ums_scheduler, p_ums_completion_list_item_out)  \
    do{ \
        p_ums_completion_list_item_out = (p_ums_scheduler)->current_completion_list_item;   \
    }while(0)


/**
 * @brief get next ums_completion_list_item during navigation
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_completion_list_item_out output, pointer to ums_completion_item, return null at the end of the list
 * 
 */
#define ums_scheduler_completion_list_iterate(p_ums_scheduler, p_ums_completion_list_item_out)  \
    do{ \
        (p_ums_scheduler)->current_completion_list_item = (p_ums_scheduler)->current_completion_list_item->next;    \
        if(likely((p_ums_scheduler)->current_completion_list_item != &((p_ums_scheduler)->completion_list->ums_context_list)))  \
            p_ums_completion_list_item_out = list_entry((p_ums_scheduler)->current_completion_list_item, ums_completion_list_item_t, list);    \
        else p_ums_completion_list_item_out = NULL; \
    }while(0)

/**
 * @brief end to iterate the completion_list
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * 
 */
#define ums_scheduler_completion_list_iterate_end(p_ums_scheduler)  \
    do{ \
        (p_ums_scheduler)->current_completion_list_item = NULL; \
        spin_unlock(&((p_ums_scheduler)->completion_list->ums_context_list_spin_lock)); \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief start to iterate the ready_list
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_completion_list_item_out output, pointer to ums_context
 * 
 */
#define ums_scheduler_ready_list_start_iteration(p_ums_scheduler, p_ums_context_out)  \
    do{ \
        (p_ums_scheduler)->current_ready_list_item = (p_ums_scheduler)->ready_list.next;    \
        p_ums_context_out = list_entry((p_ums_scheduler)->current_ready_list_item, ums_context_t, list);    \
    }while(0)

/**
 * @brief get next ums_context during navigation
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * @param p_ums_completion_list_item_out output, pointer to ums_context
 * 
 */
#define ums_scheduler_ready_list_iterate(p_ums_scheduler, p_ums_context_out)  \
    do{ \
        (p_ums_scheduler)->current_ready_list_item = (p_ums_scheduler)->current_ready_list_item->next;    \
        if(likely((p_ums_scheduler)->current_ready_list_item != &((p_ums_scheduler)->ready_list)))  \
            p_ums_context_out = list_entry((p_ums_scheduler)->current_ready_list_item, ums_context_t, list);    \
        else p_ums_context_out = NULL; \
    }while(0)

/**
 * @brief end to iterate the ready_list
 * 
 * @param p_ums_scheduler NON-NULL pointer to the scheduler
 * 
 */
#define ums_scheduler_ready_list_iterate_end(p_ums_scheduler)  \
    do{ \
        (p_ums_scheduler)->current_ready_list_item = NULL; \
    }while(0)
// -------------------------------------------------------------------

// ------------------------------------------------------------------
/**
 * @brief set reason of the next scheduler call
 * 
 */
#define ums_scheduler_set_reason_end_sheduler(p_ums_scheduler)  \
    do{ \
        (p_ums_scheduler)->entry_point_args->reason = REASON_SPECIAL_END_SCHEDULER; \
    }while(0)
// ------------------------------------------------------------------
// ########################################################################################


// ums_scheduler_sl_t ########################################################################################
/**
 * @brief object used to arrange a ums_scheduler in a hashtable and to protect it with a spin_lock
 * 
 */
typedef struct ums_scheduler_sl_t{
    struct hlist_node hlist;    /** used to arrange in the hashtable of process' schedulers */
    int key; /** key in the hashtable, corresponds to scheduler's pid*/

    spinlock_t ums_scheduler_spin_lock; /** protect ums_scheduler */
    ums_scheduler_t* ums_scheduler; /** pointer to the scheduler to protect */

    struct proc_dir_entry* proc_entry; /** entry in /proc, corresponds to /proc/ums/<tgid>/schedulers/<pid> */
    struct proc_dir_entry* proc_entry_info; /** entry in /proc, corresponds to /proc/ums/<tgid>/schedulers/<pid>/info */
    struct proc_dir_entry* proc_entry_main_workers; /** entry in /proc, corresponds to /proc/ums/<tgid>/schedulers/<pid>/workers */
}ums_scheduler_sl_t;

// -------------------------------------------------------------------
/**
 * @brief ums_scheduler_sl constructor
 * 
 * @param p_ums_scheduler_sl NON-NULL pointer ums_scheduler_sl object to init
 * @param key_in key in the hashtable of process' scheduler, corresponds to scheudler's pid
 * @param p_ums_scheduler_in NON-NULL pointer to the ums_scheduler to manage
 */
#define INIT_UMS_SCHEDULER_SL(p_ums_scheduler_sl, key_in, p_ums_scheduler_in)   \
    do{ \
        (p_ums_scheduler_sl)->key = key_in; \
        spin_lock_init(&(p_ums_scheduler_sl)->ums_scheduler_spin_lock);  \
        \
        (p_ums_scheduler_sl)->ums_scheduler = p_ums_scheduler_in;   \
        \
        (p_ums_scheduler_sl)->proc_entry = NULL;   \
        (p_ums_scheduler_sl)->proc_entry_info = NULL;   \
        (p_ums_scheduler_sl)->proc_entry_main_workers = NULL;  \
    } while(0)

/**
 * @brief ums_scheduler_sl destructor
 * 
 * @param p_ums_scheduler_sl NON-NULL pointer ums_scheduler_sl object to destroy
 */
#define DESTROY_UMS_SCHEDULER_SL(p_ums_scheduler_sl)    \
    do{ \
        (p_ums_scheduler_sl)->key = 0; \
        (p_ums_scheduler_sl)->ums_scheduler = NULL;   \
        \
        (p_ums_scheduler_sl)->proc_entry = NULL;   \
        (p_ums_scheduler_sl)->proc_entry_info = NULL;   \
        (p_ums_scheduler_sl)->proc_entry_main_workers = NULL;  \
    } while(0)
// -------------------------------------------------------------------

// --------------------------------------------------------------------------
/**
 * @brief remove ums_scheduler from the ums_scheduler_sl 
 * 
 * @param p_ums_scheduler_sl NON-NULL pointer ums_scheduler_sl object 
 * @param p_ums_scheduler_OUT output, pointer to the ums_scheduler removed
 * 
 */
#define ums_scheduler_sl_remove_scheduler(p_ums_scheduler_sl, p_ums_scheduler_OUT)  \
    do{ \
        spin_lock(&((p_ums_scheduler_sl)->ums_scheduler_spin_lock));    \
        p_ums_scheduler_OUT = (p_ums_scheduler_sl)->ums_scheduler;  \
        (p_ums_scheduler_sl)->ums_scheduler = NULL;  \
        spin_unlock(&((p_ums_scheduler_sl)->ums_scheduler_spin_lock));    \
    }while(0)
// --------------------------------------------------------------------------

// -------------------------------------------------------
/**
 * @brief lock the ums_scheduler in the ums_scheduler_sl object
 * 
 * @param p_ums_scheduler_sl NON-NULL pointer ums_scheduler_sl object 
 * @param p_ums_scheduler_OUT output, pointer to the ums_scheduler locked
 */
#define ums_scheduler_sl_lock_get_scheduler(p_ums_scheduler_sl, p_ums_scheduler_OUT)  \
    do{ \
        spin_lock(&((p_ums_scheduler_sl)->ums_scheduler_spin_lock));    \
        p_ums_scheduler_OUT = (p_ums_scheduler_sl)->ums_scheduler;  \
    }while(0)

/**
 * @brief unlock the ums_scheduler in the ums_scheduler_sl object
 * 
 * @param p_ums_scheduler_sl NON-NULL pointer ums_scheduler_sl object 
 */
#define ums_scheduler_sl_unlock_scheduler(p_ums_scheduler_sl)  \
    do{ \
        spin_unlock(&((p_ums_scheduler_sl)->ums_scheduler_spin_lock));  \
    }while(0)
// ------------------------------------------------------

// ---------------------------------------------------------------
/**
 * @brief macro used to check if a list is empty
 * 
 * @param p_list_head it can be the ready_list or the ums_context_list (ums_completion_list)
 */
#define ums_scheduler_list_empty(p_list_head)   \
    list_empty(p_list_head)

// ---------------------------------------------------------------

// ########################################################################################



/**
 * @brief snprintf useful to print a list of ums_context
 */
static inline int snprintf_list_of_ums_context(char* buff, ssize_t size_buff, struct list_head* list_of){
    struct list_head* current_item_list;
    ums_context_t* current_item;
    int offset = 0;
    if(unlikely(list_empty(list_of))){
        offset += snprintf(buff+offset, size_buff-offset, "%s", "-");
    }
    else{
        list_for_each(current_item_list, list_of){
            current_item = list_entry(current_item_list, ums_context_t, list);
            offset += snprintf(buff+offset, size_buff-offset, "\t\t\t\tums_context_t=\n""\t\t\t\t{\n");
            offset += snprintf_ums_context(buff+offset, size_buff-offset, current_item);
            offset += snprintf(buff+offset, size_buff-offset, "\t\t\t\t}\n");
        }
    }
    return offset;
}

/**
 * @brief snprintf useful to print a list of ums_context using only their descriptors 
 */
static inline int snprintf_list_of_ums_context_as_ids(char* buff, ssize_t size_buff, struct list_head* list_of){
    struct list_head* current_item_list;
    ums_context_t* current_item;
    int offset = 0;
    if(unlikely(list_empty(list_of))){
        offset += snprintf(buff+offset, size_buff-offset, "%s", "-");
    }
    else{
        list_for_each(current_item_list, list_of){
            current_item = list_entry(current_item_list, ums_context_t, list);
            //printk("id=%d\n", current_item->id);
            if(unlikely(offset)==0)
                offset += snprintf(buff+offset, size_buff-offset, "%d", current_item->id);
            else
                offset += snprintf(buff+offset, size_buff-offset, ",%d", current_item->id);
        }
    }
    return offset;
}

// --------------------------------------------------------------
/**
 * @brief snprintf useful to print a ums_scheduler object
 * 
 */
static inline int snprintf_ums_scheduler(char* buff, ssize_t size_buff, ums_scheduler_t* ums_scheduler){
    char* __buff = kmalloc(4096, GFP_KERNEL);
    char* __buff_curr_list_item = kmalloc(512, GFP_KERNEL);
    char* __buff_ready_list = kmalloc(4096, GFP_KERNEL);
    char* __buff_curr_ready_item = kmalloc(512, GFP_KERNEL);
    char* __buff_runn_thread = kmalloc(512, GFP_KERNEL);

    int tmp;

    int offset = 0;
    if( snprintf_ums_completion_list_sl(__buff, 4096, ums_scheduler->completion_list) > 4096-4){
        printk(KERN_DEBUG "\n snprintf_ums_scheduler() overflow!!!\n");
    }
    if(ums_scheduler->current_completion_list_item){            
        if(snprintf_ums_completion_list_item(__buff_curr_list_item, 512, list_entry(ums_scheduler->current_completion_list_item, ums_completion_list_item_t, list)) > 512-4){
            printk(KERN_DEBUG "\n snprintf_ums_scheduler() overflow!!!\n");
        }
    }
    else    snprintf(__buff_curr_list_item, 512, "\t\t\tNULL\n");

    tmp = snprintf_list_of_ums_context(__buff_ready_list, 4096, &ums_scheduler->ready_list);
    if(tmp==0) {
        __buff_ready_list[0] = '\n';
        __buff_ready_list[1] = '\0';
    }
    else if(tmp> 4096-4)
        printk(KERN_DEBUG "\n snprintf_ums_scheduler() overflow!!!\n");

    if(ums_scheduler->current_ready_list_item){         
        if( snprintf_ums_context(__buff_curr_ready_item, 512, list_entry(ums_scheduler->current_ready_list_item, ums_context_t, list)) > 512-4){
            printk(KERN_DEBUG "\n snprintf_ums_scheduler() overflow!!!\n");
        }   
    }
    else    snprintf(__buff_curr_ready_item, 512, "\t\t\tNULL\n");

    if(ums_scheduler->running_thread){
        if(snprintf_ums_context(__buff_runn_thread, 512, ums_scheduler->running_thread) > 512-4){
            printk(KERN_DEBUG "\n snprintf_ums_scheduler() overflow!!!\n");
        }
    }
    else    snprintf(__buff_runn_thread, 512, "\t\t\tNULL\n");

    offset += snprintf(buff+offset, size_buff-offset, 
        "\t\tscheduler_task_struct=%p\n"
        "\t\tcompletion_list=\n" "\t\t{\n%s" "\t\t}\n"
        "\t\tcurrent_completion_list_item=\n""\t\t{\n%s" "\t\t}\n"
        "\t\tready_list=\n""\t\t{\n%s" "\t\t}\n"
        "\t\tcurrent_ready_list_item=\n""\t\t{\n%s" "\t\t}\n"
        "\t\trunning_thread=\n""\t\t{\n%s" "\t\t}\n"
        , 
        (void*)ums_scheduler->scheduler_task_struct,
        __buff,
        __buff_curr_list_item,
        __buff_ready_list,
        __buff_curr_ready_item,
        __buff_runn_thread
        );

    kfree( __buff);
    kfree(__buff_curr_list_item);
    kfree(__buff_ready_list);
    kfree(__buff_curr_ready_item);
    kfree(__buff_runn_thread);
    return offset;
}

/**
 * @brief printK a ums_scheduler
 * 
 */
#define PRINTK_UMS_SCHEDULER(p_obj, PREFIX)   \
    do{ \
        char* __buff =  kmalloc(4096, GFP_KERNEL);  \
        if(snprintf_ums_scheduler(__buff, 4096, p_obj) > 4096-4)    \
            printk(KERN_DEBUG "\n PRINTK_UMS_SCHEDULER() overflow!!!\n");   \
        else printk(PREFIX "ums_scheduler_t = \n{\n%s}\n", __buff);    \
        kfree(__buff);  \
    }while(0)

// ----------------------------------------------------------------------

/**
 * @brief snprntf useful to print a ums_scheduler_sl
 * 
 */
static inline int snprintf_ums_scheduler_sl(char* buff, ssize_t buff_size, ums_scheduler_sl_t* ums_scheduler_sl){
    int res;
    char* __buff = kmalloc(4096, GFP_KERNEL);

    spin_lock(&ums_scheduler_sl->ums_scheduler_spin_lock);
        res = snprintf_ums_scheduler(__buff, 4096, ums_scheduler_sl->ums_scheduler);
    spin_unlock(&ums_scheduler_sl->ums_scheduler_spin_lock);

    if(res > 4096-4) printk(KERN_DEBUG "\n snprintf_ums_scheduler_sl() overflow!!!\n");   

    res = snprintf(buff, buff_size,
        "\tkey=%d\n"
        "\tums_scheduler=\n""\t{\n%s""\t}\n"
        ,
        ums_scheduler_sl->key,
        __buff
    );

    kfree(__buff);
    return res;
}

/**
 * @brief printK ums_scheduler_sl
 * 
 */
#define PRINTK_UMS_SCHEDULER_SL(p_obj, PREFIX)   \
    do{ \
        char* __buff =  kmalloc(4096, GFP_KERNEL);  \
        if( snprintf_ums_scheduler_sl(__buff, 4096, p_obj) > 4096-4)    \
            printk(KERN_DEBUG "\n PRINTK_UMS_SCHEDULER_SL() overflow!!!\n");   \
        else printk(PREFIX "ums_scheduler_sl_t = \n{\n%s}\n", __buff);    \
        kfree(__buff);  \
    }while(0)

// ---------------------------------------------------------------------


typedef struct idr_for_each_handler_arg_t{
    char* buff;
    ssize_t buff_size;
    int offset;
}idr_for_each_handler_arg_t;

static int idr_ums_context_for_each_handler(int id, void *p, void *idr_for_each_handler_arg){
    idr_for_each_handler_arg_t* args = (idr_for_each_handler_arg_t*) idr_for_each_handler_arg;
    ums_context_sl_t* ums_context_sl = (ums_context_sl_t*) p;

    char* __buff = kmalloc(1024, GFP_KERNEL);
    if( snprintf_ums_context_sl(__buff, 1024, ums_context_sl) > 1024-4)
        printk(KERN_DEBUG "\n idr_ums_context_for_each_handler() overflow!!!\n");   

    args->offset += printk(KERN_DEBUG
        "\tID = %d ----------------------------\n"
        "\t\tums_context_sl_t=\n" "\t\t{\n%s""\t\t}\n"
        "\t -----------------------------------\n"
        ,
        id,
        __buff
        );
    
    kfree(__buff);
    return 0;
}

static int idr_ums_completion_list_for_each_handler(int id, void *p, void *idr_for_each_handler_arg){
    idr_for_each_handler_arg_t* args = (idr_for_each_handler_arg_t*) idr_for_each_handler_arg;

    ums_completion_list_sl_t* ums_completion_list_sl = (ums_completion_list_sl_t*) p;

    char* __buff = kmalloc(1024, GFP_KERNEL);
    if( snprintf_ums_completion_list_sl(__buff, 1024, ums_completion_list_sl) > 1024-4)
        printk(KERN_DEBUG "\n idr_ums_completion_list_for_each_handler() overflow!!!\n");   

    args->offset += printk(KERN_DEBUG
        "\t-------------ID = %d -------------\n"
        "\t\tums_completion_list_sl_t=\n" "\t\t{\n%s""\t\t}\n"
        "\t -----------------------------------\n"
        ,
        id,
        __buff
        );

    kfree(__buff);
    return 0;
}


//###################################################################################