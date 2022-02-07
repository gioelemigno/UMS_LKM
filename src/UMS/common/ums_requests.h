#ifndef UMS_REQUEST_H_
#define UMS_REQUEST_H_

/// @file 
/// This file contains all the request macros
///

#include "ums_types.h"

#define REQUEST_0       120
#define REQUEST_1       119
#define REQUEST_2       118
#define REQUEST_3       117
#define REQUEST_4       116
#define REQUEST_5       115
#define REQUEST_6       114
#define REQUEST_7       113
#define REQUEST_8       112
#define REQUEST_9       111
#define REQUEST_10      110

#define REQUEST_11      109
#define REQUEST_12      108
#define REQUEST_13      107
#define REQUEST_14      106
#define REQUEST_15      105
#define REQUEST_16      104
#define REQUEST_17      103
#define REQUEST_18      102
#define REQUEST_19      101
#define REQUEST_20      100


#define REQUEST_DEBUG_0     255
#define REQUEST_DEBUG_1     254
#define REQUEST_DEBUG_2     253
#define REQUEST_DEBUG_3     252
#define REQUEST_DEBUG_4     251





#define RQ_CREATE_PROCESS               REQUEST_0
#define RQ_DELETE_PROCESS               REQUEST_1
typedef struct rq_create_delete_process_args_t{
    pid_t tgid;
}rq_create_delete_process_args_t;


#define RQ_CREATE_COMPLETION_LIST       REQUEST_2
#define RQ_DELETE_COMPLETION_LIST       REQUEST_3
typedef struct rq_create_delete_completion_list_args_t{
    pid_t tgid;
    ums_completion_list_descriptor_t descriptor;
}rq_create_delete_completion_list_args_t;


#define RQ_CREATE_UMS_CONTEXT           REQUEST_4
#define RQ_DELETE_UMS_CONTEXT           REQUEST_5
typedef struct rq_create_delete_ums_context_args_t{
    pid_t tgid;
    ums_context_descriptor_t descriptor;
    void* (*routine)(void* args);
    void* args;
    void* user_res;
    int cpu_core;
}rq_create_delete_ums_context_args_t;


#define RQ_COMPLETION_LIST_ADD_UMS_CONTEXT      REQUEST_6
#define RQ_COMPLETION_LIST_REMOVE_UMS_CONTEXT   REQUEST_7
typedef struct rq_completion_list_add_remove_ums_context_args_t{
    pid_t tgid;
    ums_completion_list_descriptor_t completion_list_d;
    ums_context_descriptor_t ums_context_d;

//    ums_completion_list_item_t* ums_completion_list_item;
}rq_completion_list_add_remove_ums_context_args_t;



#define RQ_CREATE_UMS_SCHEDULER        REQUEST_8
#define RQ_EXIT_UMS_SCHEDULER          REQUEST_9
typedef struct rq_create_delete_ums_scheduler_args_t{
    pid_t tgid; //pid of the process
    pid_t pid;  //pid of the scheduler's thread
    ums_completion_list_descriptor_t completion_list_d;
    void (*entry_point_func)(entry_point_args_t* ep_args);
    void* sched_args;

    entry_point_args_t* entry_point_args;

    int return_value;
    int cpu_core;
}rq_create_delete_ums_scheduler_args_t;



#define RQ_EXECUTE_NEXT_NEW_THREAD          REQUEST_10
typedef struct rq_execute_next_new_thread_args_t{
    pid_t tgid; //pid of the process
    pid_t pid;  //pid of the scheduler's thread

    void* (*routine)(void* args);
    void* args;

    pid_t pid_scheduler;
    int cpu_core;

    ums_context_descriptor_t ucd;
}rq_execute_next_new_thread_args_t;


#define RQ_EXECUTE_NEXT_READY_THREAD        REQUEST_11
typedef struct rq_execute_next_ready_thread_args_t{
    pid_t tgid; //pid of the process
    pid_t pid;  //pid of the scheduler's thread
}rq_execute_next_ready_thread_args_t;


#define RQ_STARTUP_NEW_THREAD       REQUEST_12
typedef struct rq_startup_new_thread_args_t{
    //ums_scheduler_descriptor_t usd;
    ums_context_descriptor_t ucd;
    pid_t pid_scheduler;
}rq_startup_new_thread_args_t;


#define RQ_END_THREAD       REQUEST_13
typedef struct rq_end_thread_args_t{
    ums_context_descriptor_t ucd;
    pid_t pid_scheduler;
}rq_end_thread_args_t;


#define RQ_WAIT_NEXT_SCHEDULER_CALL     REQUEST_14
typedef struct rq_wait_next_scheduler_call_args_t{
    ums_context_descriptor_t ucd;
    reason_t reason;
}rq_wait_next_scheduler_call_args_t;


#define RQ_YIELD_UMS_CONTEXT            REQUEST_15
typedef struct rq_yield_ums_context_args_t{
    int unused;
}rq_yield_ums_context_args_t;


#define RQ_GET_FROM_CL      REQUEST_16
typedef struct rq_get_from_cl_args_t{
    info_ums_context_t* info_context_array;
    size_t array_size;
}rq_get_from_cl_args_t;


#define RQ_GET_FROM_RL      REQUEST_17
typedef struct rq_get_from_rl_args_t{
    info_ums_context_t* info_context_array;
    size_t array_size;
}rq_get_from_rl_args_t;

#define RQ_EXECUTE          REQUEST_18
typedef struct rq_execute_args_t{
    info_ums_context_t* info_context;

    pid_t tgid; //pid of the process
    pid_t pid;  //pid of the scheduler's thread

    void* (*routine)(void* args);
    void* args;

    pid_t pid_scheduler;
    ums_context_descriptor_t ucd;

    int cpu_core;

}rq_execute_args_t;

#define RQ_EXECUTE_READY_LIST   REQUEST_19


#endif /* UMS_REQUEST_H_ */