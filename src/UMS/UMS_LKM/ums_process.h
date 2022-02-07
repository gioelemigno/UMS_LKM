#pragma once
/// @file 
/// This file contains definitions and functions of objects related to a ums_process
///

#include <linux/proc_fs.h>
#include "ums_scheduler.h"

// ums_process_t ########################################################################################
#define UMS_PROCESS_COMPLETION_LIST_MIN_ID  0       /** Lower value for a ums_completion_list descriptor */
#define UMS_PROCESS_COMPLETION_LIST_MAX_ID  127     /** Higher value for a ums_completion_list descriptor */

#define UMS_PROCESS_UMS_CONTEXT_MIN_ID  0           /** Lower value for a ums_context descriptor */
#define UMS_PROCESS_UMS_CONTEXT_MAX_ID  127         /** Higher value for a ums_context descriptor */

#define HASHTABLE_UMS_SCHEDULERS_HASH_BITS 6    /** size of hashtable = 2^HASH_BITS */
#define HASHTABLE_UMS_THREADS_HASH_BITS 6       /** size of hashtable = 2^HASH_BITS */

/**
 * @brief Represent a ums_process object
 * 
 */
typedef struct ums_process_t{
    struct hlist_node hlist; /** field used to arrange it in the ums_hashmap */
    int key; /** key in the ums_hashtable (equals to tgid (thread id)) */

    DECLARE_HASHTABLE(hashtable_ums_schedulers, HASHTABLE_UMS_SCHEDULERS_HASH_BITS);    /** hashtable that contains schedulers, the key of as scheduler is its pid*/  
    rwlock_t hashtable_ums_schedulers_rwlock; /** rw_spin_lock of ums_scheduler_hashtable */

    DECLARE_HASHTABLE(hashtable_ums_threads, HASHTABLE_UMS_THREADS_HASH_BITS);  /** hashtable used to map a thread to its ums_context */
    rwlock_t hashtable_ums_threads_rwlock; /** rw_spin_lock of the ums_thraed_hashtable */


    struct idr idr_completion_list;  /** idr struct used to allocate the ums_completion_lists managed by this process*/
    rwlock_t idr_completion_list_rwlock;    /** rw_spin_lock of idr_completion_list */

    struct idr idr_ums_context;  /** idr struct used to store ums_contexts managed by this process*/
    rwlock_t idr_ums_context_rwlock;    /** rw_spin_lock of idr_ums_context */

    struct proc_dir_entry* proc_entry;  /** entry in /proc, corresponds to /proc/ums/<tgid> */
    struct proc_dir_entry* proc_entry_main_scheds; /** entry in /proc, corresponds to /proc/ums/<tgid>/schedulers */
}ums_process_t;

// -------------------------------------------------------------------
/**
 * @brief ums_process constructor
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param key_in key used in the ums_hashtable, it corresponds to <tgid>
 */
#define INIT_UMS_PROCESS(p_ums_process, key_in)		\
	do {						\
		(p_ums_process)->key = key_in;    	\
        (p_ums_process)->proc_entry = NULL;  \
        \
        hash_init((p_ums_process)->hashtable_ums_schedulers);   \
        rwlock_init(&(p_ums_process)->hashtable_ums_schedulers_rwlock);    \
        \
        hash_init((p_ums_process)->hashtable_ums_threads);  \
        rwlock_init(&(p_ums_process)->hashtable_ums_threads_rwlock);   \
        \
        idr_init(&(p_ums_process)->idr_completion_list);       \
        rwlock_init(&(p_ums_process)->idr_completion_list_rwlock); \
        \
        idr_init(&(p_ums_process)->idr_ums_context);       \
        rwlock_init(&(p_ums_process)->idr_ums_context_rwlock); \
	} while (0)

/**
 * @brief ums_process destructor
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 */
#define DESTROY_UMS_PROCESS(p_ums_process)   \
    do {    \
        idr_destroy(&(p_ums_process)->idr_completion_list);       \
        \
        idr_destroy(&(p_ums_process)->idr_ums_context);       \
        \
        (p_ums_process)->key = 0;    	\
        (p_ums_process)->proc_entry = NULL;  \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief add a ums_scheduler_sl object to the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_scheduler_sl NON-NULL pointer to the ums_scheduler_sl to add
 */
#define ums_process_add_scheduler_sl(p_ums_process, p_ums_scheduler_sl)   \
    do{ \
        write_lock(&((p_ums_process)->hashtable_ums_schedulers_rwlock));    \
            hash_add(p_ums_process->hashtable_ums_schedulers, &((p_ums_scheduler_sl)->hlist), (p_ums_scheduler_sl)->key);   \
        write_unlock(&((p_ums_process)->hashtable_ums_schedulers_rwlock));  \
        \
        ums_proc_add_scheduler((p_ums_process)->proc_entry_main_scheds, (p_ums_scheduler_sl)->key, (p_ums_scheduler_sl)->proc_entry, (p_ums_scheduler_sl)->proc_entry_info, (p_ums_scheduler_sl)->proc_entry_main_workers);    \
        \
    }while(0)

/**
 * @brief remove a ums_scheduler_sl from the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_scheduler_sl NON-NULL pointer to the ums_scheduler to remove
 */
#define ums_process_remove_scheduler_sl(p_ums_process, p_ums_scheduler_sl)   \
    do{ \
        write_lock(&((p_ums_process)->hashtable_ums_schedulers_rwlock));    \
            hash_del(&((p_ums_scheduler_sl)->hlist));   \
        write_unlock(&((p_ums_process)->hashtable_ums_schedulers_rwlock));  \
        \
        ums_proc_remove_scheduler((p_ums_scheduler_sl)->proc_entry, (p_ums_scheduler_sl)->proc_entry_info, (p_ums_scheduler_sl)->proc_entry_main_workers);    \
        \
    }while(0)
// -------------------------------------------------------------------


// -------------------------------------------------------------------
/**
 * @brief register a ums_context in hashtable_ums_thread of the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_context NON-NULL pointer to a ums_context to add
 */
#define ums_process_register_ums_thread(p_ums_process, p_ums_context)   \
    do{ \
        write_lock(&((p_ums_process)->hashtable_ums_threads_rwlock));    \
            hash_add(p_ums_process->hashtable_ums_threads, &((p_ums_context)->hlist), (p_ums_context)->pid);   \
        write_unlock(&((p_ums_process)->hashtable_ums_threads_rwlock));  \
    }while(0)

/**
 * @brief unregister a ums_context from hashtable_ums_thread of the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_context NON-NULL pointer to the ums_context to remove
 * 
 */
#define ums_process_unregister_ums_thread(p_ums_process, p_ums_context)   \
    do{ \
        write_lock(&((p_ums_process)->hashtable_ums_threads_rwlock));    \
            hash_del(&((p_ums_context)->hlist));   \
        write_unlock(&((p_ums_process)->hashtable_ums_threads_rwlock));  \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief get a ums_context from hashtable_threads of the process by its pid
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param key_in pid of the thread
 * @param p_ums_context_OUT output, pointer to a ums_context
 */
#define ums_process_get_ums_thread(p_ums_process, key_in, p_ums_context_OUT)    \
    do{ \
    ums_context_t* current_ums_context = NULL;  \
	read_lock(&((p_ums_process)->hashtable_ums_threads_rwlock));   \
        /*iterate a bucket*/    \
        hash_for_each_possible((p_ums_process)->hashtable_ums_threads, current_ums_context, hlist, key_in){    \
            if(likely(current_ums_context && current_ums_context->pid == key_in))    break;\
        }   \
    read_unlock(&((p_ums_process)->hashtable_ums_threads_rwlock)); \
    \
    p_ums_context_OUT = (likely(current_ums_context && current_ums_context->pid == key_in)) ? current_ums_context : NULL; \
    }while(0)
// -----------


// -------------------------------------------------------------------
/**
 * @brief get a ums_scheduler_sl from the ums_hashtable
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param key_in pid of the scheduler
 * @param p_ums_scheduler_sl_OUT output, pointer to a ums_scheduler_sl
 * 
 */
#define ums_process_get_scheduler_sl(p_ums_process, key_in, p_ums_scheduler_sl_OUT)    \
    do{ \
    ums_scheduler_sl_t* current_ums_scheduler_sl = NULL;  \
	read_lock(&((p_ums_process)->hashtable_ums_schedulers_rwlock));   \
        /*iterate a bucket*/    \
        hash_for_each_possible((p_ums_process)->hashtable_ums_schedulers, current_ums_scheduler_sl, hlist, key_in){    \
            if(likely(current_ums_scheduler_sl && current_ums_scheduler_sl->key == key_in))    break;\
        }   \
    read_unlock(&((p_ums_process)->hashtable_ums_schedulers_rwlock)); \
    \
    p_ums_scheduler_sl_OUT = (likely(current_ums_scheduler_sl && current_ums_scheduler_sl->key == key_in)) ? current_ums_scheduler_sl : NULL; \
    }while(0)
// ------------------------------------------------------------------


// -------------------------------------------------------------------
/**
 * @brief add a ums_context_sl to idr_ums_context of the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_context_sl NON-NULL pointer ums_context_sl to add
 */
#define ums_process_add_ums_context_sl(p_ums_process, p_ums_context_sl) \
    do{\
        write_lock(&((p_ums_process)->idr_ums_context_rwlock));     \
            (p_ums_context_sl)->id = idr_alloc(&((p_ums_process)->idr_ums_context), p_ums_context_sl, UMS_PROCESS_UMS_CONTEXT_MIN_ID, UMS_PROCESS_UMS_CONTEXT_MAX_ID, GFP_KERNEL); \
            (p_ums_context_sl)->ums_context->id = (p_ums_context_sl)->id; \
        write_unlock(&((p_ums_process)->idr_ums_context_rwlock));   \
    }while(0)

/**
 * @brief remove a ums_context_sl from idr_ums_context of the ums_process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_context_sl NON-NULL pointer to the ums_context_sl to remove
 * 
 */
#define ums_process_remove_ums_context_sl(p_ums_process, p_ums_context_sl) \
    do{ \
        write_lock(&((p_ums_process)->idr_ums_context_rwlock));   \
            idr_remove(&((p_ums_process)->idr_ums_context), (p_ums_context_sl)->id); \
            (p_ums_context_sl)->id = -1;    \
            (p_ums_context_sl)->ums_context->id = -1; \
        write_unlock(&((p_ums_process)->idr_ums_context_rwlock));   \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief add a ums_completion_list_sl to the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_completion_list_sl NON-NULL pointer to the object to add
 */
#define ums_process_add_ums_completion_list_sl(p_ums_process, p_ums_completion_list_sl) \
    do{\
        write_lock(&((p_ums_process)->idr_completion_list_rwlock));     \
            (p_ums_completion_list_sl)->id = idr_alloc(&((p_ums_process)->idr_completion_list), p_ums_completion_list_sl, UMS_PROCESS_COMPLETION_LIST_MIN_ID, UMS_PROCESS_COMPLETION_LIST_MAX_ID, GFP_KERNEL); \
        write_unlock(&((p_ums_process)->idr_completion_list_rwlock));   \
    }while(0)

/**
 * @brief remove a ums_completion_list_sl to the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param p_ums_completion_list_sl NON-NULL pointer to the object to remove
 */
#define ums_process_remove_ums_completion_list_sl(p_ums_process, p_ums_completion_list_sl) \
    do{\
        write_lock(&((p_ums_process)->idr_completion_list_rwlock));     \
            idr_remove(&((p_ums_process)->idr_completion_list), (p_ums_completion_list_sl)->id); \
            (p_ums_completion_list_sl)->id = -1;    \
        write_unlock(&((p_ums_process)->idr_completion_list_rwlock));   \
    }while(0)
// -------------------------------------------------------------------

// ------------------------------------------------------------------
/**
 * @brief get a ums_completion_list_sl object from the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param id ums_completion_list_sl descriptor
 * @param p_ums_completion_list_sl_OUT output, pointer object to get
 */
#define ums_process_get_ums_completion_list_sl(p_ums_process, id, p_ums_completion_list_sl_OUT) \
    do{ \
        read_lock(&((p_ums_process)->idr_completion_list_rwlock));     \
            p_ums_completion_list_sl_OUT = idr_find(&((p_ums_process)->idr_completion_list), id); \
        read_unlock(&((p_ums_process)->idr_completion_list_rwlock));   \
    }while(0)
// ------------------------------------------------------------------

// ------------------------------------------------------------------
/**
 * @brief get a ums_context_sl object from the process
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * @param id ums_context descriptor
 * @param p_ums_context_sl_OUT output, pointer object to get
 */
#define ums_process_get_ums_context_sl(p_ums_process, id, p_ums_context_sl_OUT) \
    do{ \
        read_lock(&((p_ums_process)->idr_ums_context_rwlock));     \
            p_ums_context_sl_OUT = idr_find(&((p_ums_process)->idr_ums_context), id); \
        read_unlock(&((p_ums_process)->idr_ums_context_rwlock));   \
    }while(0)
// ------------------------------------------------------------------


// ########################################################################################


/**
 * @brief printK a ums_process
 * 
 */
static inline int printk_ums_process(ums_process_t* ums_process){
    idr_for_each_handler_arg_t args;
  

    char* __buff_hashtable = kmalloc(8192, GFP_KERNEL);    
    char* __buff_sched_sl = kmalloc(8192, GFP_KERNEL);
    char* __buff_idr_context = kmalloc(8192, GFP_KERNEL);
    char* __buff_idr_completion = kmalloc(8192, GFP_KERNEL);

    int offset_buff = 0;
    int current_bucket = 0;            
    ums_scheduler_sl_t* current_item = NULL;    
    int k =0;
    printk(KERN_DEBUG " __________________________________________ ums_process_t __________________________________________");

    printk(KERN_DEBUG 
            "\tkey=%d\n"
            "\thashtable_ums_schedulers=\n\t{\n"
            ,
            ums_process->key
    );

    read_lock(&ums_process->hashtable_ums_schedulers_rwlock);
        hash_for_each(ums_process->hashtable_ums_schedulers, current_bucket, current_item, hlist){
            if((k = snprintf_ums_scheduler_sl(__buff_sched_sl, 8192, current_item)) >8192-4)
                printk(KERN_DEBUG "\n snprintf_ums_process() overflow!!!\n");  
            
            offset_buff += printk(KERN_DEBUG 
                "\t................. BUCKET = %d VALUE = %p .......................\n"
                "\tums_scheduler_sl_item=\n""\t{\n%s""\t}\n"
                "\t.....................................................................\n"
                , 
                current_bucket, 
                current_item,
                __buff_sched_sl
            );
            //printk("offset=%d\n", offset_buff);
        }  
    read_unlock(&ums_process->hashtable_ums_schedulers_rwlock);

    if(offset_buff == 0){
        __buff_hashtable[0] = '\n';
        __buff_hashtable[1] = '\0';
    }
    if(offset_buff > 8192-4)    printk(KERN_DEBUG "\n snprintf_ums_process() overflow!!!\n");   
    
    printk(KERN_DEBUG 
            "\t}\n"
    );

    printk(KERN_DEBUG 
            "\tidr_completion_list=\n\t{\n"
    );

    args.buff = __buff_idr_completion;
    args.buff_size = 8192;
    args.offset = 0;
    read_lock(&ums_process->idr_completion_list_rwlock);
        idr_for_each(&ums_process->idr_completion_list, &idr_ums_completion_list_for_each_handler, &args);
    read_unlock(&ums_process->idr_completion_list_rwlock);
    if(args.offset > args.buff_size-4)    printk(KERN_DEBUG "\n snprintf_ums_process() overflow!!!\n");   

    printk(KERN_DEBUG 
            "\t}\n"
    );
    printk(KERN_DEBUG 
            "\tidr_ums_context=\n\t{\n"
    );

    args.buff = __buff_idr_context;
    args.buff_size = 8192;
    args.offset = 0;
    read_lock(&ums_process->idr_ums_context_rwlock);
        idr_for_each(&ums_process->idr_ums_context, &idr_ums_context_for_each_handler, &args);
    read_unlock(&ums_process->idr_ums_context_rwlock);
    if(args.offset > args.buff_size-4)    printk(KERN_DEBUG "\n snprintf_ums_process() overflow!!!\n");   

    printk(KERN_DEBUG 
        "\t}\n"
    );

    kfree(__buff_hashtable);
    kfree(__buff_sched_sl);
    kfree(__buff_idr_context);
    kfree(__buff_idr_completion);
    
    printk(KERN_DEBUG "________________________________________________________________________________________");

    return 0; 
}

/**
 * @brief printk a ums_process
 * 
 */
#define PRINTK_UMS_PROCESS(p_ums_process, ignore) \
    do{\
        printk_ums_process(p_ums_process);   \
    }while(0)

//-----------------------------------------------------

