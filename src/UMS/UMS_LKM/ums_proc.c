#include "ums_proc.h"
#include "ums_context.h"
#include "ums_process.h"
#include "ums_hashtable.h"
#include <linux/jiffies.h>

#define __USS_INFO_BUFF_SIZE    128
ssize_t ums_scheduler_snprintf_info(pid_t tgid, pid_t sched_pid, char* buff, size_t buff_size){
    ums_process_t* ums_process;
    ums_scheduler_t* ums_scheduler;
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_completion_list_sl_t* ums_completion_list_sl;
    struct list_head* completion_list;
    char buff_cl[__USS_INFO_BUFF_SIZE];
    char buff_rl[__USS_INFO_BUFF_SIZE];
    ssize_t len = 0;

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;
    
    ums_process_get_scheduler_sl(ums_process, sched_pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL))
        return -ERR_INTERNAL;

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;  //should be a KERNEL PANIC
    }

    ums_completion_list_sl = ums_scheduler->completion_list;
    ums_completion_list_sl_lock_get_list(ums_completion_list_sl, completion_list);    
        snprintf_completion_list_as_ids(buff_cl, __USS_INFO_BUFF_SIZE, completion_list);
    ums_completion_list_sl_unlock_list(ums_completion_list_sl);

    snprintf_list_of_ums_context_as_ids(buff_rl, __USS_INFO_BUFF_SIZE, &(ums_scheduler->ready_list));

    len += snprintf(buff, buff_size,
                        "ns=%d\n"
                        "cl=%s\n"
                        "rl=%s\n"
                        "run=%d\n"
                        , 
                        ums_scheduler->num_switch,
                        buff_cl,
                        buff_rl,
                        (ums_scheduler->running_thread)?ums_scheduler->running_thread->id:-1
                        );

    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);

    return len;
}





#define __USS_WORKER_BUFF_SIZE    128
ssize_t ums_scheduler_snprintf_worker(pid_t tgid, pid_t sched_pid, ums_context_descriptor_t ucd, char* buff, size_t buff_size){
    ums_process_t* ums_process;
    
    ums_scheduler_sl_t* ums_scheduler_sl;
    ums_scheduler_t* ums_scheduler;

    ums_context_sl_t* ums_context_sl;
    ums_context_t* ums_context;


    ssize_t len = 0;

    ums_hashtable_get_process(tgid, ums_process);
    if(unlikely(ums_process == NULL))
        return -ERR_INTERNAL;
    
    ums_process_get_scheduler_sl(ums_process, sched_pid, ums_scheduler_sl);
    if(unlikely(ums_scheduler_sl == NULL))
        return -ERR_INTERNAL;

    ums_scheduler_sl_lock_get_scheduler(ums_scheduler_sl, ums_scheduler);
    if(unlikely(ums_scheduler == NULL)){
        ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);
        return -ERR_INTERNAL;  //should be a KERNEL PANIC
    }
    // NOTE: It is necessary to lock the scheduler that manage the thread since at this point, only the scheduler
    // can access to the thread

    ums_process_get_ums_context_sl(ums_process, ucd, ums_context_sl);
    ums_context = ums_context_sl->ums_context;

    
    len += snprintf(buff, buff_size,
                        "ns=%d\n"
                        "state=%s\n"
                        "ums_run_time=%u\n"
                        , 
                        ums_context->num_switch,
                        ums_context_printable_state(ums_context),
                        ums_context_get_run_time_ms(ums_context)
                        );
    
    ums_scheduler_sl_unlock_scheduler(ums_scheduler_sl);

    return len;
}