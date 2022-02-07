#pragma once
/// @file 
/// This file contains definitions and functions of objects related to a ums_completion_list
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

#include "../common/ums_types.h"
#include "ums_context.h"

// ums_completion_list_item_t ########################################################################################
/**
 * @brief element of completion_list
 * 
 */
typedef struct ums_completion_list_item_t{
    struct list_head list;  /** list field */
    int ums_context_id; /** descriptor of the referred ums_context */
}ums_completion_list_item_t;

// -------------------------------------------------------------------
/**
 * @brief constructor ums_completion_list_item
 * 
 * @param p_ums_completion_list_item object to init
 * @param ums_context_id_in id of the referred ums_context
 * 
 */
#define INIT_UMS_COMPLETION_LIST_ITEM(p_ums_completion_list_item, ums_context_id_in) \
    do{ \
        (p_ums_completion_list_item)->ums_context_id = ums_context_id_in;   \
    }while(0)

/**
 * @brief destructor ums_completion_list_item
 * 
 * @param p_ums_completion_list_item object to destroy
 * 
 */
#define DESTROY_UMS_COMPLETION_LIST_ITEM(p_ums_completion_list_item) \
    do{ \
        (p_ums_completion_list_item)->ums_context_id = -1;   \
    }while(0)
// -------------------------------------------------------------------
// ########################################################################################

// ums_completion_list_sl_t ########################################################################################
/**
 * @brief object that contains the ums_completion_list and protect it using a spin_lock
 * 
 */
typedef struct ums_completion_list_sl_t{
    int id; /** descriptor */

    spinlock_t ums_context_list_spin_lock;  /** used to protect the ums_completion_list */
    struct list_head ums_context_list;  /** ums_completion_list */
}ums_completion_list_sl_t;

// -------------------------------------------------------------------
/**
 * @brief constructor ums_completion_list_sl object
 * 
 * @param p_ums_completion_list_sl object to init
 */
#define INIT_UMS_COMPLETION_LIST_SL(p_ums_completion_list_sl)   \
    do{ \
        (p_ums_completion_list_sl)->id = -1; \
        \
        spin_lock_init(&(p_ums_completion_list_sl)->ums_context_list_spin_lock);  \
        INIT_LIST_HEAD(&(p_ums_completion_list_sl)->ums_context_list); \
    }while(0)

/**
 * @brief destructor ums_completion_list_sl object
 * 
 * @param p_ums_completion_list_sl object to destroy
 */
#define DESTROY_UMS_COMPLETION_LIST_SL(p_ums_completion_list_sl)   \
    do{ \
        (p_ums_completion_list_sl)->id = -1; \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief add a ums_completion_list item to the ums_completion_list
 * 
 * @param p_ums_completion_list pointer to ums_context_list (ums_completion_list)
 * @param p_ums_completion_list_item pointer to the element to add
 * 
 */
#define ums_completion_list_add_item(p_ums_completion_list, p_ums_completion_list_item) \
    do{ \
        spin_lock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
            list_add_tail(&((p_ums_completion_list_item)->list), &((p_ums_completion_list)->ums_context_list));  \
        spin_unlock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
    }while(0)

/**
 * @brief remove a ums_completion_list item to the ums_completion_list
 * 
 * @param p_ums_completion_list pointer to ums_context_list (ums_completion_list)
 * @param p_ums_completion_list_item pointer to the element to remove
 * 
 */
#define ums_completion_list_remove_item(p_ums_completion_list, p_ums_completion_list_item) \
    do{ \
        spin_lock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
            list_del(&((p_ums_completion_list_item)->list));  \
        spin_unlock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
    }while(0)
// -------------------------------------------------------------------

// -------------------------------------------------------------------
/**
 * @brief get and lock the ums_completion_list
 * 
 * @param p_ums_completion_list_sl  pointer to ums_completion_list_sl
 * @param p_list_head pointer to the actual ums_completion_list
 * 
 */
#define ums_completion_list_sl_lock_get_list(p_ums_completion_list_sl, p_list_head)   \
    do{ \
        spin_lock(&((p_ums_completion_list_sl)->ums_context_list_spin_lock));  \
        p_list_head = &((p_ums_completion_list_sl)->ums_context_list);  \
    }while(0)

/**
 * @brief unlock the ums_completion_list
 * 
 * @param p_ums_completion_list_sl  pointer to ums_context_list_sl
 *   
 */
#define ums_completion_list_sl_unlock_list(p_ums_completion_list_sl)    \
    do{ \
        spin_unlock(&((p_ums_completion_list_sl)->ums_context_list_spin_lock));  \
    }while(0)
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
/**
 * @brief remove a ums_completion_list_item by the ums_context descriptor to which it refers to
 * 
 * @param  p_ums_completion_list pointer to ums_completion_list_sl
 * @param ums_context_descriptor descriptor of the ums_context
 * @param p_ums_completion_list_item output, ums_completion_list_item removed
 */
#define ums_completion_list_remove_item_by_descriptor(p_ums_completion_list, ums_context_descriptor, p_ums_completion_list_item) \
    do{ \
        struct list_head* current_item_list;    \
        ums_completion_list_item_t* current_item;   \
        \
        spin_lock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
        list_for_each(current_item_list, &((p_ums_completion_list)->ums_context_list)){ \
            current_item = list_entry(current_item_list, ums_completion_list_item_t, list); \
            if(unlikely(current_item->ums_context_id == ums_context_descriptor)){   \
                list_del(&((current_item)->list));  \
                p_ums_completion_list_item = current_item; \
                break;  \
            }   \
        }   \
        spin_unlock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
    }while(0)

/**
 * @brief without ue of spin_lock, remove a ums_completion_list_item by the ums_context descriptor to which it refers to
 * 
 * @param  p_ums_completion_list pointer to ums_completion_list_sl
 * @param ums_context_descriptor descriptor of the ums_context
 * @param p_ums_completion_list_item output, ums_completion_list_item removed
 * 
 * NOTE: This function assumes that spin_lock has been already called
 */
#define ums_completion_list_remove_item_by_descriptor_no_sl(p_ums_completion_list, ums_context_descriptor, p_ums_completion_list_item) \
    do{ \
        struct list_head* current_item_list;    \
        ums_completion_list_item_t* current_item;   \
        \
        list_for_each(current_item_list, &((p_ums_completion_list)->ums_context_list)){ \
            current_item = list_entry(current_item_list, ums_completion_list_item_t, list); \
            if(unlikely(current_item->ums_context_id == ums_context_descriptor)){   \
                list_del(&((current_item)->list));  \
                p_ums_completion_list_item = current_item; \
                break;  \
            }   \
        }   \
    }while(0)
// ------------------------------------------------------------------------------

// --------------------------------------------------------------------------------
/**
 * @brief remove first element from the ums_completion_list
 * 
 * @param p_ums_completion_list pointer to ums_completion_list_sl
 * @param p_ums_completion_list_item_OUT, output, removed element
 * 
 * NOTE: If the list is empty, it return NULL
 */
#define ums_completion_list_remove_first(p_ums_completion_list, p_ums_completion_list_item_OUT) \
    do{ \
        spin_lock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
        p_ums_completion_list_item_OUT = list_first_entry_or_null(&(p_ums_completion_list)->ums_context_list, ums_completion_list_item_t, list);    \
        if(likely((p_ums_completion_list_item_OUT) != NULL))   \
            list_del(&((p_ums_completion_list_item_OUT)->list));  \
        spin_unlock(&((p_ums_completion_list)->ums_context_list_spin_lock));  \
    }while(0)
// --------------------------------------------------------------------------------

// ########################################################################################
/**
 * @brief snprintf for a ums_completion_list_item
 * 
 */
static inline int snprintf_ums_completion_list_item(char* buff, ssize_t size_buff, ums_completion_list_item_t* ums_completion_list_item){
    return snprintf(buff, size_buff, "\t\t\t\t\tums_context_id=%d\n", ums_completion_list_item->ums_context_id);
}

/**
 * @brief printK ums_completion_list_item
 * 
 */
#define PRINTK_UMS_COMPLETION_LIST_ITEM(p_obj, PREFIX)   \
    do{ \
        char* __buff = kmalloc(512, GFP_KERNEL);    \
        if( snprintf_ums_completion_list_item(__buff, 512, p_obj) > 512-4)   \
            printk(KERN_DEBUG "\n PRINTK_UMS_COMPLETION_LIST_ITEM() overflow!!!\n");    \
        else printk(PREFIX "ums_completion_list_item_t = \n{\n%s}\n", __buff); \
        kfree(__buff);  \
    }while(0)

// --------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------
/**
 * @brief snprintf for ums_completion_list, print it as a list of IDs
 * 
 */
static inline int snprintf_completion_list_as_ids(char* buff, ssize_t size_buff, struct list_head* list_of){
    struct list_head* current_item_list;
    ums_completion_list_item_t* current_item;
    int offset = 0;

    if(unlikely(list_empty(list_of))){
        offset += snprintf(buff+offset, size_buff-offset, "%s", "-");
    }
    else{
        list_for_each(current_item_list, list_of){
            current_item = list_entry(current_item_list, ums_completion_list_item_t, list);
            //printk("id=%d\n", current_item->ums_context_id);
            if(unlikely(offset)==0)
                offset += snprintf(buff+offset, size_buff-offset, "%d", current_item->ums_context_id);
            else
                offset += snprintf(buff+offset, size_buff-offset, ",%d", current_item->ums_context_id);
        }
    }
    return offset;
}

// ------------------------------------------------------------------------------------------

/**
 * @brief snprintf ums_completion_list_sl object
 * 
 */
static inline int snprintf_ums_completion_list_sl(char* buff, ssize_t size_buff, ums_completion_list_sl_t* ums_completion_list_sl){
    struct list_head* current_item_list;
    ums_completion_list_item_t* current_item;
    int offset = 0;
    offset += snprintf(buff+offset, size_buff-offset, "\t\t\tid=%d\n", ums_completion_list_sl->id);
    offset += snprintf(buff+offset, size_buff-offset, "\t\t\tums_context_list=\n\t\t\t{\n");
    if(spin_trylock(&ums_completion_list_sl->ums_context_list_spin_lock)){
        list_for_each(current_item_list, &ums_completion_list_sl->ums_context_list){
            current_item = list_entry(current_item_list, ums_completion_list_item_t, list);
            offset += snprintf(buff+offset, size_buff-offset, "\t\t\t\tums_completion_list_item=\n""\t\t\t\t{\n");
            offset += snprintf_ums_completion_list_item(buff+offset, size_buff-offset, current_item);
            offset += snprintf(buff+offset, size_buff-offset, "\t\t\t\t}\n");
        }
        spin_unlock(&ums_completion_list_sl->ums_context_list_spin_lock);
    }
    else offset += snprintf(buff+offset, size_buff-offset, "\t\t\t\t BUSY \n");
    offset += snprintf(buff+offset, size_buff-offset, "\t\t\t}\n");
    return offset;
}

/**
 * @brief printK ums_completion_list_sl object
 * 
 */
#define PRINTK_UMS_COMPLETION_LIST_SL(p_obj, PREFIX)   \
    do{ \
        char* __buff = kmalloc(4096, GFP_KERNEL);   \
        if( snprintf_ums_completion_list_sl(__buff, 4096, p_obj) > 4096-4)  \
            printk(KERN_DEBUG "\n PRINTK_UMS_COMPLETION_LIST_SL() overflow!!!\n");   \
        else printk(PREFIX "\tums_completion_list_sl_t = \n\t\t{\n%s\t\t}\n", __buff);    \
        kfree(__buff);  \
    }while(0)

// ---------------------------------------------------------------------------