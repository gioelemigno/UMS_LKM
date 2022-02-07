#pragma once
/// @file 
/// This file contains all the requests used to manage a ums_process
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

// -----------------------------------------------------------------------------
/**
 * Request used to create a new ums_process
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0  
 */
static inline int rq_create_process(rq_create_delete_process_args_t* args){
    rq_create_delete_process_args_t args_san;
    if(copy_from_user(&args_san, args, sizeof(args_san)))
        return -EFAULT;

    ums_hashtable_create_process(args_san.tgid);    
    return 0;
}

/**
 * Request used to delete a ums_process
 * 
 * @param args Arguments of the request (provided by user) 
 * 
 * @return Returns 0  
 */
static inline int rq_delete_process(rq_create_delete_process_args_t* args){
    rq_create_delete_process_args_t args_san;
    if(copy_from_user(&args_san, args, sizeof(args_san)))
        return -EFAULT;

    ums_hashtable_delete_process(args_san.tgid);
    return 0;
}
// -------------------------------------------------------------------------------
