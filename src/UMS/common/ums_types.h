#pragma once 

/// @file 
/// This file contains all variable types that must be shared by user and kerenl module
///

#define REASON_0        100
#define REASON_1        101
#define REASON_2        102
#define REASON_3        103
#define REASON_4        104
#define REASON_5        105
#define REASON_6        106
#define REASON_7        107

#define REASON_SPECIAL_0    200
#define REASON_SPECIAL_1    201
#define REASON_SPECIAL_2    202
#define REASON_SPECIAL_3    203


//USED TO SET errno
#define RES_ERR_0     300
#define RES_ERR_1     301
#define RES_ERR_2     302
#define RES_ERR_3     303
#define RES_ERR_4     304
#define RES_ERR_5     305
#define RES_ERR_6     306
#define RES_ERR_7     307

// --------------------------------------------------

typedef int ums_context_descriptor_t;
typedef int ums_completion_list_descriptor_t;

//typedef pthread_t ums_scheduler_descriptor_t;

typedef int res_t;
#define SUCCESS                         0
#define FAILURE                        -1

//errno value
#define ERR_EMPTY_COMP_LIST     RES_ERR_0
#define ERR_EMPTY_READY_LIST    RES_ERR_1
#define ERR_INVALID_CLD         RES_ERR_2
#define ERR_INVALID_UCD         RES_ERR_3
#define ERR_INTERNAL            RES_ERR_4   /*SHOULD BE A KERNEL PANIC*/
#define ERR_ASSIGNED            RES_ERR_5
#define ERR_CPU_SELECTED        RES_ERR_6

typedef int reason_t;
#define REASON_STARTUP              REASON_0
#define REASON_THREAD_BLOCKED       REASON_1
#define REASON_THREAD_YIELD         REASON_2
#define REASON_THREAD_ENDED         REASON_3

#define REASON_SPECIAL_END_SCHEDULER    REASON_SPECIAL_0

/**
 * @brief arguments of a entry_point function
 * 
 */
typedef struct entry_point_args_t{
    reason_t reason; /** reason of the scheduler call:
                        REASON_STARTUP
                        REASON_THREAD_YIELD
                        REASON_THREAD_ENDED */
    ums_context_descriptor_t activation_payload;    /** if reason is yielded or ended thread, 
                                                    indicates the descriptor of the ums_context */
    void* sched_args;   /** user defined scheduler arguments */
}entry_point_args_t;

/**
 * @brief used to choose a ums_context from the ready list or from the completion_list
 * 
 */
typedef struct info_ums_context_t{
    ums_context_descriptor_t ucd;
    unsigned int run_time_ms;
    int number_switch;
    
    void* user_reserved;
    bool from_cl;
}info_ums_context_t;