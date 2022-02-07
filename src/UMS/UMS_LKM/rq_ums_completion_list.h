#pragma once
/// @file 
/// This file contains all the requests used to manage a ums_completion_list
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

//-------------------------------------------------------------------------------------------------------
/**
 * Request used to create a new completion_list
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_create_completion_list(rq_create_delete_completion_list_args_t* args){
    rq_create_delete_completion_list_args_t args_san;
    ums_process_t* ums_process;
    ums_completion_list_sl_t* ums_completion_list_sl;

    if(copy_from_user(&args_san, args, sizeof(args_san)))
        return -EFAULT;

    ums_hashtable_get_process(args_san.tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;

    ums_completion_list_sl =  kmalloc(sizeof(ums_completion_list_sl_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_SL(ums_completion_list_sl);

    ums_process_add_ums_completion_list_sl(ums_process, ums_completion_list_sl);

    args_san.descriptor = ums_completion_list_sl->id;

    if(copy_to_user(args, &args_san, sizeof(args_san)))
        return -EFAULT;

    return 0;
}

/**
 * Request used to delete a new completion_list
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_delete_completion_list(rq_create_delete_completion_list_args_t* args){
    rq_create_delete_completion_list_args_t args_san;
    ums_process_t* ums_process;
    ums_completion_list_sl_t* ums_completion_list_sl;

    if(copy_from_user(&args_san, args, sizeof(args_san)))
        return -EFAULT;

    ums_hashtable_get_process(args_san.tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;
    
    ums_process_get_ums_completion_list_sl(ums_process, args_san.descriptor, ums_completion_list_sl);

    if(unlikely(ums_completion_list_sl == NULL))
        return -ERR_INTERNAL;
    
    ums_process_remove_ums_completion_list_sl(ums_process, ums_completion_list_sl);
    DESTROY_UMS_COMPLETION_LIST_SL(ums_completion_list_sl);

    kfree(ums_completion_list_sl);
    
    return 0;
}
// ----------------------------------------------------------------------------------------------------


// ---------------------------------------------------------------------------------------------------
/**
 * Request used to add ums_context to a completion_list
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_completion_list_add_ums_context(rq_completion_list_add_remove_ums_context_args_t* rq_args){
    rq_completion_list_add_remove_ums_context_args_t rq_args_san;
    ums_process_t* ums_process;
    ums_completion_list_sl_t* ums_completion_list_sl;
    ums_completion_list_item_t* ums_completion_list_item; 

    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    ums_hashtable_get_process(rq_args_san.tgid, ums_process);

    ums_process_get_ums_completion_list_sl(ums_process, rq_args_san.completion_list_d, ums_completion_list_sl);
    if(unlikely(ums_completion_list_sl == NULL))
        return -ERR_INTERNAL;
    
    ums_completion_list_item = kmalloc(sizeof(ums_completion_list_item_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item, rq_args_san.ums_context_d);

    ums_completion_list_add_item(ums_completion_list_sl, ums_completion_list_item);

    return 0; 
}

/**
 * Request used to remove a ums_context from a completion_list
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0 on sucess, otherwise -errno  
 */
static inline int rq_completion_list_remove_ums_context(rq_completion_list_add_remove_ums_context_args_t* rq_args){
    rq_completion_list_add_remove_ums_context_args_t rq_args_san;
    ums_process_t* ums_process;
    ums_completion_list_sl_t* ums_completion_list_sl;
    ums_completion_list_item_t* ums_completion_list_item; 

    if(copy_from_user(&rq_args_san, rq_args, sizeof(rq_args_san)))
        return -EFAULT;

    ums_hashtable_get_process(rq_args_san.tgid, ums_process);
    
    ums_process_get_ums_completion_list_sl(ums_process, rq_args_san.completion_list_d, ums_completion_list_sl);
    if(unlikely(ums_completion_list_sl == NULL))
        return -ERR_INTERNAL;
        
    ums_completion_list_remove_item_by_descriptor(ums_completion_list_sl, rq_args_san.ums_context_d, ums_completion_list_item);

    DESTROY_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item);
    kfree(ums_completion_list_item);

    return 0;
}


