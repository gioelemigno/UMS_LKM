#define _GNU_SOURCE
#include <sys/sysinfo.h>
#include <sched.h>

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
#include <errno.h>


// -----------------------------------------------------------------------------------------------------
res_t create_ums_context(ums_context_descriptor_t* descriptor, void* (*routine)(void*), void* args, void* user_res){
    rq_create_delete_ums_context_args_t rq_args = {
        .tgid = tgid,
        .routine = routine,
        .args = args,
        .descriptor = -1,
        .user_res = user_res
    };
    res_t res = ioctl(ums_fd, RQ_CREATE_UMS_CONTEXT, &rq_args); 
    
    *descriptor = rq_args.descriptor;
    return res;
}
res_t delete_ums_context(ums_context_descriptor_t descriptor){
    rq_create_delete_ums_context_args_t rq_args = {
        .tgid = tgid,
        .routine = NULL,
        .args = NULL,
        .descriptor = descriptor
    };
    return ioctl(ums_fd, RQ_DELETE_UMS_CONTEXT, &rq_args); 
}
// -----------------------------------------------------------------------------------------------------



// -----------------------------------------------------------------------------------------------------
typedef struct startup_new_thread_args_t{
    ums_context_descriptor_t ucd;
    pid_t sheduler_pid;

    void* (*routine)(void*);
    void* args_routine;
}startup_new_thread_args_t;

void* startup_new_thread(void* args){
    startup_new_thread_args_t* startup_new_thread_args = (startup_new_thread_args_t*)args;
    int res;
    
    rq_startup_new_thread_args_t rq_startup_new_thread_args = {
        .ucd = startup_new_thread_args->ucd,
        .pid_scheduler = startup_new_thread_args->sheduler_pid
    };
    
    res = ioctl(ums_fd, RQ_STARTUP_NEW_THREAD, &rq_startup_new_thread_args);
    if(res==-1){
        printf("error ioctl\n");
        exit(EXIT_FAILURE);
    }

    startup_new_thread_args->routine(startup_new_thread_args->args_routine);

    
    rq_end_thread_args_t rq_end_thread_args = {
        .ucd = rq_startup_new_thread_args.ucd,
        .pid_scheduler = rq_startup_new_thread_args.pid_scheduler
    };
    res = ioctl(ums_fd, RQ_END_THREAD, &rq_end_thread_args);
    if(res==-1){
        printf("error ioctl\n");
        exit(EXIT_FAILURE);
    }
    return 0;
}
res_t execute_next_new_thread(){
    int res;
    pthread_t thread;
    int errno_backup;
        
    cpu_set_t cpu_set;
    pthread_attr_t attr;
    
    rq_execute_next_new_thread_args_t rq_args ={
        .ucd =-1,
        .routine = NULL,
        .args = NULL,
        .pid_scheduler = -1,
        .cpu_core = -1
    };
    
    res = ioctl(ums_fd, RQ_EXECUTE_NEXT_NEW_THREAD, &rq_args);
    if(res!=0)  return res;
    /*
    if(res==0)
        printf("ucd=%d, routine=%lx args=%lx\n", rq_args.ucd, (unsigned long)rq_args.routine, (unsigned long)rq_args.args);
    else{
        return res;
    }
    */
    startup_new_thread_args_t startup_new_thread_args = {
        .ucd = rq_args.ucd,
        .sheduler_pid = rq_args.pid_scheduler,
        .routine = rq_args.routine,
        .args_routine = rq_args.args,
    };
    
    if(rq_args.cpu_core == -1)
        res = pthread_create(&thread, NULL, startup_new_thread, &startup_new_thread_args);
    else{
        printf("new thread at cpu%d\n", rq_args.cpu_core);

        pthread_attr_init(&attr);
        CPU_ZERO(&cpu_set);
        CPU_SET(rq_args.cpu_core, &cpu_set);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set);
        res = pthread_create(&thread, &attr, startup_new_thread, &startup_new_thread_args);
    }
    //pthread_create(&thread, NULL, startup_new_thread, &startup_new_thread_args);
    return res;
}

// -----------------------------------------------------------------------------------------------------
res_t execute_next_ready_thread(){
    int res;
    rq_execute_next_ready_thread_args_t rq_args;

    res = ioctl(ums_fd, RQ_EXECUTE_NEXT_READY_THREAD, &rq_args);
    return res;
}
// -----------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------
res_t yield(void){
    rq_yield_ums_context_args_t rq_args;
    int res = ioctl(ums_fd, RQ_YIELD_UMS_CONTEXT, &rq_args);
    return res;
}
// --------------------------------------------------------------------

// --------------------------------------------------------------
res_t execute(info_ums_context_t* info_ums_context){
    int res;
    pthread_t thread;
    int errno_backup;

    cpu_set_t cpu_set;
    pthread_attr_t attr;

    rq_execute_args_t rq_args ={
        .ucd =-1,
        .routine = NULL,
        .args = NULL,
        .pid_scheduler = -1,
        .info_context = info_ums_context,
        .cpu_core = -1
    };
    
    if(info_ums_context->from_cl){
        printf("new thread \n");

        res = ioctl(ums_fd, RQ_EXECUTE, &rq_args);
        if(res!=0)  return res;
        
        if(res==0)
            printf("ucd=%d, routine=%lx args=%lx\n", rq_args.ucd, (unsigned long)rq_args.routine, (unsigned long)rq_args.args);
        
        startup_new_thread_args_t startup_new_thread_args = {
            .ucd = rq_args.ucd,
            .sheduler_pid = rq_args.pid_scheduler,
            .routine = rq_args.routine,
            .args_routine = rq_args.args
        };
        if(rq_args.cpu_core == -1)
            res = pthread_create(&thread, NULL, startup_new_thread, &startup_new_thread_args);
        else{
            printf("new thread at cpu%d\n", rq_args.cpu_core);
            pthread_attr_init(&attr);
            CPU_ZERO(&cpu_set);
            CPU_SET(rq_args.cpu_core, &cpu_set);
            pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set);
            res = pthread_create(&thread, &attr, startup_new_thread, &startup_new_thread_args);
        }
        //pthread_create(&thread, NULL, startup_new_thread, &startup_new_thread_args);
    }
    else{
        printf("from ready\n");
        res = ioctl(ums_fd, RQ_EXECUTE_READY_LIST, &rq_args);
    }
    return res;
}
// --------------------------------------------------------------