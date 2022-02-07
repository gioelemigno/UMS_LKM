#ifndef UMS_HASHTABLE_H_
#define UMS_HASHTABLE_H_

/// @file 
/// This file contains definitions and functions related to the ums_hashtable
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
#include "ums_scheduler.h"

#include <linux/proc_fs.h>

#include "ums_proc.h"

#include "ums_process.h"





// ums_hashtable ########################################################################################
#define UMS_HASHTABLE_HASH_BITS 10 /** size of hashtable = 2^HASH_BITS */

extern DECLARE_HASHTABLE(ums_hashtable, UMS_HASHTABLE_HASH_BITS);
extern rwlock_t ums_hashtable_rwlock;

/** init ums_hashtable */
#define UMS_HASHTABLE_INIT()   hash_init(ums_hashtable);  

// -------------------------------------------------------------------
/**
 * @brief add a ums_process to the ums_hashtable
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 * 
 * NOTE: The key will be (p_ums_process)->key
 */
#define ums_hashtable_add_process(p_ums_process)    \
    write_lock(&ums_hashtable_rwlock);  \
	    hash_add(ums_hashtable, &((p_ums_process)->hlist), (p_ums_process)->key);   \
	write_unlock(&ums_hashtable_rwlock);    \

/**
 * @brief remove a ums_process from the ums_hashtable
 * 
 * @param p_ums_process NON-NULL pointer to a ums_process
 */
#define ums_hashtable_remove_process(p_ums_process)    \
    write_lock(&ums_hashtable_rwlock);  \
	    hash_del(&((p_ums_process)->hlist));   \
	write_unlock(&ums_hashtable_rwlock);    \
// -------------------------------------------------------------------


//-------------------------------------------------------------------
/**
 * @brief get a ums_process from its tgid
 * 
 * @param tgid ums_process's tgid (key in the hashtable)
 * @param p_ums_process output, pointer to a ums_process
 */
#define ums_hashtable_get_process(tgid, p_ums_process)    \
    do{ \
    ums_process_t* current_ums_process = NULL;  \
	read_lock(&ums_hashtable_rwlock);   \
        /*iterate a bucket*/    \
        hash_for_each_possible(ums_hashtable, current_ums_process, hlist, tgid){    \
            if(likely(current_ums_process && current_ums_process->key == tgid))    break;\
        }   \
    read_unlock(&ums_hashtable_rwlock); \
    \
    p_ums_process = (likely(current_ums_process && current_ums_process->key == tgid)) ? current_ums_process : NULL; \
    }while(0)


//-------------------------------------------------------------------

// ------------------------------------------------------------------------------------
/**
 * @brief create a new ums_process and add it to the ums_hashtable
 * @param tgid ums_process's tgid, namely the tgid of the actual Linux process
 * 
 */
#define ums_hashtable_create_process(tgid)  \
    do{ \
        ums_process_t* item = kmalloc(sizeof(ums_process_t), GFP_KERNEL);   \
        INIT_UMS_PROCESS(item, tgid);   \
        \
        write_lock(&ums_hashtable_rwlock);  \
            hash_add(ums_hashtable, &item->hlist, item->key);   \
        write_unlock(&ums_hashtable_rwlock);    \
        \
        ums_proc_add_process(item->proc_entry, item->proc_entry_main_scheds, tgid); \
    }while(0)

/**
 * @brief delete a ums_process from the hashtable
 * 
 * @param tgid tgid of the ums_process to delete
 * 
 * NOTE: This function must be used only if the ums_process has been created by ums_hashtable_create_process()
 */
#define ums_hashtable_delete_process(tgid)  \
    do{ \
        ums_process_t* ums_process; \
        ums_hashtable_get_process(tgid, ums_process);   \
        if(likely(ums_process != NULL)){    \
            ums_proc_remove_process(ums_process->proc_entry, ums_process->proc_entry_main_scheds);    \
            \
            write_lock(&ums_hashtable_rwlock);  \
                hash_del(&ums_process->hlist);  \
            write_unlock(&ums_hashtable_rwlock);    \
            \
            DESTROY_UMS_PROCESS(ums_process);   \
            kfree(ums_process); \
        }   \
    }while(0)
// ------------------------------------------------------------------------------




// ########################################################################################
/**
 * @brief printK the ums_hashtable
 * 
 */
static inline int printk_ums_hashtable(void){
    int current_bucket = 0;            
    ums_process_t* current_item = NULL;                                  


    read_lock(&ums_hashtable_rwlock);
        hash_for_each(ums_hashtable, current_bucket, current_item, hlist){
            printk(KERN_DEBUG 
                        "\t |||||||||||||||||||||||||||||||||||| KEY = %d |||| VALUE = %p |||||||||||||||||||||||||||||||||||| \n"
                        , 
                        current_bucket, 
                        current_item
                        );
            printk_ums_process(current_item);

            printk(KERN_DEBUG 
                         "\t ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| \n"
                        );             
        }  
    read_unlock(&ums_hashtable_rwlock);

    return 0;
}

/**
 * @brief printK ums_hashtable
 * 
 */
#define PRINTK_UMS_HASHTABLE(ignore)  \
    do{ \
        printk("################################### ums_hashtable ###################################\n"); \
        printk_ums_hashtable(); \
        printk("####################################################################################\n"); \
    }while(0)


int test(void);






#endif/*UMS_HASHTABLE_H_*/