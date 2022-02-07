#define _GNU_SOURCE
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "../../common/ums_requests.h"
#include <stdint.h>
#include <sys/ioctl.h>
#include "ums.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <errno.h>

#include <sched.h>

// -----------------------------------------------------------------------------------------------------
res_t join_scheduler(ums_scheduler_descriptor_t* usd, int* return_value){
    pthread_t* thread_sched = (pthread_t*)usd;
    void* ret;
    pthread_join(*thread_sched, &ret);
    *return_value = (int)(unsigned long)ret;
    return SUCCESS;
}
// -----------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------
void* create_ums_scheduler_routine(void* args){
    int res;
    entry_point_args_t entry_point_args;
    rq_wait_next_scheduler_call_args_t rq_wait_next_scheduler_call;

    rq_create_delete_ums_scheduler_args_t* rq_args = (rq_create_delete_ums_scheduler_args_t*) args;
    rq_args->entry_point_args = &entry_point_args;

    // SETUP
    res = ioctl(ums_fd, RQ_CREATE_UMS_SCHEDULER, rq_args);  // create datastruct
    if(res == FAILURE){
        free(rq_args);
        printf("Error! RQ_CREATE_UMS_SCHEDULER\n");
        exit(EXIT_FAILURE);
    }

    // CONST
    entry_point_args.sched_args = rq_args->sched_args;
    // VARIABLE
    entry_point_args.activation_payload = -1;
    entry_point_args.reason = REASON_STARTUP; 
    
    while(1){
        rq_args->entry_point_func(&entry_point_args); 
        
        // check if the user ends explicity the scheduler
        if(entry_point_args.reason == REASON_SPECIAL_END_SCHEDULER){
            break;
        }
                
        res = ioctl(ums_fd, RQ_WAIT_NEXT_SCHEDULER_CALL, &rq_wait_next_scheduler_call);
        if(res != 0){
            printf("Error RQ_WAIT_NEXT_SCHEDULER_CALL!\n");
            exit(EXIT_FAILURE);
        }
        
        // TO REMOVE
        if(entry_point_args.reason == REASON_SPECIAL_END_SCHEDULER){
            break;
        }
    }
    // CLEAN
    free(rq_args);

    // return value of the scheduler
    return (void*) (unsigned long) (int) entry_point_args.activation_payload;   
}

res_t create_ums_scheduler(ums_scheduler_descriptor_t* sd, ums_completion_list_descriptor_t cd, void(*entry_point)(entry_point_args_t* entry_point_args), void* sched_args, int cpu_core){
    int res;
    cpu_set_t cpu_set;
    pthread_attr_t attr;

    pthread_t* thread_sched = (pthread_t*)sd;
    
    if(cpu_core >= get_nprocs_conf() || cpu_core < -1){
        printf("invalid cpu core\n");
        errno = ERR_CPU_SELECTED;
        return -1;
    }

    // TO BE FREED IN SCHED THREAD!
    rq_create_delete_ums_scheduler_args_t* rq_args = malloc(sizeof(rq_create_delete_ums_scheduler_args_t));
    rq_args->tgid = tgid;
    rq_args->completion_list_d = cd;
    rq_args->entry_point_func = entry_point;
    rq_args->sched_args = sched_args;
    rq_args->cpu_core = cpu_core;
    if(cpu_core == -1)
        res = pthread_create(thread_sched, NULL, create_ums_scheduler_routine, (void*)rq_args);
    else{
        printf("new scheduler at cpu%d\n", cpu_core);
        pthread_attr_init(&attr);
        CPU_ZERO(&cpu_set);
        CPU_SET(cpu_core, &cpu_set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set);
        res = pthread_create(thread_sched, &attr, create_ums_scheduler_routine, (void*)rq_args);
    }

    return (res == 0)? 0: -1;
}
// -----------------------------------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------
void exit_scheduler(int return_value){
    int res;
  
    rq_create_delete_ums_scheduler_args_t rq_args; 
    rq_args.return_value = return_value;
    res = ioctl(ums_fd, RQ_EXIT_UMS_SCHEDULER, &rq_args);

    if(res == -1){
        printf("Error RQ_EXIT_UMS_SCHEDULER\n");
        exit(EXIT_FAILURE);
    }
}
// -----------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------
res_t get_ums_contexts_from_cl(info_ums_context_t* array_info_ums_context, size_t array_size){
    rq_get_from_cl_args_t rq_args;

    rq_args.info_context_array = array_info_ums_context;
    rq_args.array_size = array_size;

    int res = ioctl(ums_fd, RQ_GET_FROM_CL, &rq_args);
    return res;
}

res_t get_ums_contexts_from_rl(info_ums_context_t* array_info_ums_context, size_t array_size){
    rq_get_from_rl_args_t rq_args;

    rq_args.info_context_array = array_info_ums_context;
    rq_args.array_size = array_size;

    int res = ioctl(ums_fd, RQ_GET_FROM_RL, &rq_args);
    return res;
}


// --------------------------------------------------------------------------

