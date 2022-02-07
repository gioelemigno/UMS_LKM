
#include "ums_hashtable.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/hashtable.h>
#include <stdbool.h>
#include <linux/spinlock.h>
#include <linux/rwlock.h>

#include "../common/ums_requests.h"

#define MIN_ID 0
#define MAX_ID 99

DEFINE_HASHTABLE(ums_hashtable, UMS_HASHTABLE_HASH_BITS);

rwlock_t ums_hashtable_rwlock = __RW_LOCK_UNLOCKED(ums_hashtable_rwlock);



void* routine_example(void* args);
char* args_test = "args_test";


ums_context_t* ums_context_0;
ums_context_t* ums_context_1;
ums_context_t* ums_context_2;
ums_context_t* ums_context_3;
ums_context_t* ums_context_4;

ums_context_sl_t* ums_context_sl_0;
ums_context_sl_t* ums_context_sl_1;
ums_context_sl_t* ums_context_sl_2;
ums_context_sl_t* ums_context_sl_3;
ums_context_sl_t* ums_context_sl_4;

ums_completion_list_item_t* ums_completion_list_item_0;
ums_completion_list_item_t* ums_completion_list_item_1;
ums_completion_list_item_t* ums_completion_list_item_2;
ums_completion_list_item_t* ums_completion_list_item_3;
ums_completion_list_item_t* ums_completion_list_item_4;
ums_completion_list_item_t* ums_completion_list_item_5;

ums_completion_list_sl_t* ums_completion_list_sl_0;
ums_completion_list_sl_t* ums_completion_list_sl_1;

ums_scheduler_t* ums_scheduler_0;
ums_scheduler_t* ums_scheduler_1;

ums_scheduler_sl_t* ums_scheduler_sl_0;
ums_scheduler_sl_t* ums_scheduler_sl_1;

ums_process_t* ums_process;

void init_test(void);
void end_test(void);

int test(void){
    //    ums_completion_list_item_t* item;
    ums_context_t* context;
    int i;
    init_test();


   // PRINTK_UMS_HASHTABLE(KERN_DEBUG);
    
    ums_hashtable_remove_process(ums_process);
   // PRINTK_UMS_HASHTABLE(KERN_DEBUG);

    ums_hashtable_add_process(ums_process);
   // PRINTK_UMS_HASHTABLE(KERN_DEBUG);
    
    //ums_scheduler_completion_list_start_iteration(ums_scheduler_0, item);
    //PRINTK_UMS_COMPLETION_LIST_ITEM(item, KERN_DEBUG);
    ums_scheduler_ready_list_add(ums_scheduler_0, ums_context_0);
    ums_scheduler_ready_list_add(ums_scheduler_0, ums_context_1);
    ums_scheduler_ready_list_add(ums_scheduler_0, ums_context_2);


    ums_scheduler_ready_list_start_iteration(ums_scheduler_0, context);
    //PRINTK_UMS_CONTEXT(context, KERN_DEBUG);
   //PRINTK_UMS_HASHTABLE(KERN_DEBUG);
    
    for(i =0; i< 5; i++){
        //ums_scheduler_completion_list_iterate(ums_scheduler_0, item);
        ums_scheduler_ready_list_iterate(ums_scheduler_0, context);
            //PRINTK_UMS_HASHTABLE(KERN_DEBUG);
        if(!context) break;
                PRINTK_UMS_SCHEDULER(ums_scheduler_0, KERN_DEBUG);

        //PRINTK_UMS_COMPLETION_LIST_ITEM(item, KERN_DEBUG);
        //PRINTK_UMS_HASHTABLE(KERN_DEBUG);
        //PRINTK_UMS_CONTEXT(context, KERN_DEBUG);
    }
    ums_scheduler_ready_list_iterate_end(ums_scheduler_0);
                PRINTK_UMS_SCHEDULER(ums_scheduler_0, KERN_DEBUG);

    //ums_scheduler_completion_list_iterate_end(ums_scheduler_0);

    printk("i=%d NULL \n", i);
    //PRINTK_UMS_HASHTABLE(KERN_DEBUG);

    end_test();
    return 0;
}




void init_test(void){
    ums_context_0 = kmalloc(sizeof(ums_context_t), GFP_KERNEL);
    INIT_UMS_CONTEXT(ums_context_0, &routine_example, &args_test);
    ums_context_sl_0 = kmalloc(sizeof(ums_context_sl_t), GFP_KERNEL);
    INIT_UMS_CONTEXT_SL(ums_context_sl_0, ums_context_0);

    ums_context_1 = kmalloc(sizeof(ums_context_t), GFP_KERNEL);
    INIT_UMS_CONTEXT(ums_context_1, &routine_example, &args_test);
    ums_context_sl_1 = kmalloc(sizeof(ums_context_sl_t), GFP_KERNEL);
    INIT_UMS_CONTEXT_SL(ums_context_sl_1, ums_context_1);

    ums_context_2 = kmalloc(sizeof(ums_context_t), GFP_KERNEL);
    INIT_UMS_CONTEXT(ums_context_2, &routine_example, &args_test);
    ums_context_sl_2 = kmalloc(sizeof(ums_context_sl_t), GFP_KERNEL);
    INIT_UMS_CONTEXT_SL(ums_context_sl_2, ums_context_2);

    ums_context_3 = kmalloc(sizeof(ums_context_t), GFP_KERNEL);
    INIT_UMS_CONTEXT(ums_context_3, &routine_example, &args_test);
    ums_context_sl_3 = kmalloc(sizeof(ums_context_sl_t), GFP_KERNEL);
    INIT_UMS_CONTEXT_SL(ums_context_sl_3, ums_context_3);

    ums_context_4 = kmalloc(sizeof(ums_context_t), GFP_KERNEL);
    INIT_UMS_CONTEXT(ums_context_4, &routine_example, &args_test);
    ums_context_sl_4 = kmalloc(sizeof(ums_context_sl_t), GFP_KERNEL);
    INIT_UMS_CONTEXT_SL(ums_context_sl_4, ums_context_4);



    ums_completion_list_item_0 = kmalloc(sizeof(ums_completion_list_item_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_0, 0);
    ums_completion_list_item_1 = kmalloc(sizeof(ums_completion_list_item_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_1, 1);
    ums_completion_list_item_2 = kmalloc(sizeof(ums_completion_list_item_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_2, 2);

    ums_completion_list_item_3 = kmalloc(sizeof(ums_completion_list_item_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_3, 0);
    ums_completion_list_item_4 = kmalloc(sizeof(ums_completion_list_item_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_4, 3);
    ums_completion_list_item_5 = kmalloc(sizeof(ums_completion_list_item_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_5, 4);


    ums_completion_list_sl_0 =  kmalloc(sizeof(ums_completion_list_sl_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_SL(ums_completion_list_sl_0);
    ums_completion_list_add_item(ums_completion_list_sl_0, ums_completion_list_item_0);
    ums_completion_list_add_item(ums_completion_list_sl_0, ums_completion_list_item_1);
    ums_completion_list_add_item(ums_completion_list_sl_0, ums_completion_list_item_2);


    ums_completion_list_sl_1 =  kmalloc(sizeof(ums_completion_list_sl_t), GFP_KERNEL);
    INIT_UMS_COMPLETION_LIST_SL(ums_completion_list_sl_1);
    ums_completion_list_add_item(ums_completion_list_sl_1, ums_completion_list_item_3);
    ums_completion_list_add_item(ums_completion_list_sl_1, ums_completion_list_item_4);
    ums_completion_list_add_item(ums_completion_list_sl_1, ums_completion_list_item_5);



    ums_scheduler_0 = kmalloc(sizeof(ums_scheduler_t), GFP_KERNEL);
    INIT_UMS_SCHEDULER(ums_scheduler_0, NULL, ums_completion_list_sl_0);
    ums_scheduler_sl_0 = kmalloc(sizeof(ums_scheduler_sl_t), GFP_KERNEL);
    INIT_UMS_SCHEDULER_SL(ums_scheduler_sl_0, 1235, ums_scheduler_0);

    ums_scheduler_1 = kmalloc(sizeof(ums_scheduler_t), GFP_KERNEL);
    INIT_UMS_SCHEDULER(ums_scheduler_1, NULL, ums_completion_list_sl_1);
    ums_scheduler_sl_1 = kmalloc(sizeof(ums_scheduler_sl_t), GFP_KERNEL);
    INIT_UMS_SCHEDULER_SL(ums_scheduler_sl_1, 1236, ums_scheduler_1);


    ums_process = kmalloc(sizeof(ums_process_t), GFP_KERNEL);
    INIT_UMS_PROCESS(ums_process, 1234);

    ums_process_add_scheduler_sl(ums_process, ums_scheduler_sl_0);
    ums_process_add_scheduler_sl(ums_process, ums_scheduler_sl_1);

    ums_process_add_ums_context_sl(ums_process, ums_context_sl_0);
    ums_process_add_ums_context_sl(ums_process, ums_context_sl_1);
    ums_process_add_ums_context_sl(ums_process, ums_context_sl_2);
    ums_process_add_ums_context_sl(ums_process, ums_context_sl_3);
    ums_process_add_ums_context_sl(ums_process, ums_context_sl_4);

    ums_process_add_ums_completion_list_sl(ums_process, ums_completion_list_sl_0);
    ums_process_add_ums_completion_list_sl(ums_process, ums_completion_list_sl_1);
 
    ums_hashtable_add_process(ums_process);
}

void end_test(void){

     //#############################
    DESTROY_UMS_CONTEXT(ums_context_0);
    kfree(ums_context_0);

    DESTROY_UMS_CONTEXT(ums_context_1);
    kfree(ums_context_1);

    DESTROY_UMS_CONTEXT(ums_context_2);
    kfree(ums_context_2);

    DESTROY_UMS_CONTEXT(ums_context_3);
    kfree(ums_context_3);

    DESTROY_UMS_CONTEXT(ums_context_4);
    kfree(ums_context_4);
    // #############################
    DESTROY_UMS_CONTEXT_SL(ums_context_sl_0);
    kfree(ums_context_sl_0);

    DESTROY_UMS_CONTEXT_SL(ums_context_sl_1);
    kfree(ums_context_sl_1);

    DESTROY_UMS_CONTEXT_SL(ums_context_sl_2);
    kfree(ums_context_sl_2);

    DESTROY_UMS_CONTEXT_SL(ums_context_sl_3);
    kfree(ums_context_sl_3);

    DESTROY_UMS_CONTEXT_SL(ums_context_sl_4);
    kfree(ums_context_sl_4);
    //###############################
    DESTROY_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_0);
    kfree(ums_completion_list_item_0);
    
    DESTROY_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_1);
    kfree(ums_completion_list_item_1);

    DESTROY_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_2);
    kfree(ums_completion_list_item_2);
    
    DESTROY_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_3);
    kfree(ums_completion_list_item_3);

    DESTROY_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_4);
    kfree(ums_completion_list_item_4);

    DESTROY_UMS_COMPLETION_LIST_ITEM(ums_completion_list_item_5);
    kfree(ums_completion_list_item_5);
    //################################
    DESTROY_UMS_COMPLETION_LIST_SL(ums_completion_list_sl_0);
    kfree(ums_completion_list_sl_0);

    DESTROY_UMS_COMPLETION_LIST_SL(ums_completion_list_sl_1);
    kfree(ums_completion_list_sl_1);
    // ###############################
    DESTROY_UMS_SCHEDULER(ums_scheduler_0);
    kfree(ums_scheduler_0);

    DESTROY_UMS_SCHEDULER(ums_scheduler_1);
    kfree(ums_scheduler_1);

    // ###################################
    DESTROY_UMS_SCHEDULER_SL(ums_scheduler_sl_0);
    kfree(ums_scheduler_sl_0);
        DESTROY_UMS_SCHEDULER_SL(ums_scheduler_sl_1);
    kfree(ums_scheduler_sl_1);
    //#####################################
    DESTROY_UMS_PROCESS(ums_process);
    kfree(ums_process);
    //######################################
}


void* routine_example(void* args){
    printk("Routine_test %s\n", (char*)args);
    return NULL;
}