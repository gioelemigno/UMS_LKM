#define _GNU_SOURCE
#include <sched.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>

#include "ums.h"
#include <errno.h>

int value_0 = 0;

int value_1 = 1;
int value_2 = 2;


int value_3 = 3;
int value_4 = 4;



void entry_point(entry_point_args_t* entry_point_args);

void* routine(void* args){
    int value = *((int*) args);
    int i;
    for(i=0; i<5; i++){
        printf("CORE%d - Hello world!---%d---\n", sched_getcpu(), value);
	    sleep(5);
        if(i ==2)  {
            printf("CORE%d - YIELD\n", sched_getcpu());
            yield();
        }
    }

    return NULL;
}

int main(int argc, char **argv){
    int ret;
    ums_init();
    
    ums_context_descriptor_t ucd_0;

    ums_context_descriptor_t ucd_1;
    ums_context_descriptor_t ucd_2;

    ums_context_descriptor_t ucd_3;
    ums_context_descriptor_t ucd_4;
    

    ums_completion_list_descriptor_t uld_0;
    ums_completion_list_descriptor_t uld_1;

    
    create_ums_context(&ucd_0, &routine, &value_0, NULL);
    create_ums_context(&ucd_1, &routine, &value_1, NULL);
    create_ums_context(&ucd_2, &routine, &value_2, NULL);

    create_ums_context(&ucd_3, &routine, &value_3, NULL);
    create_ums_context(&ucd_4, &routine, &value_4, NULL);
    

    create_ums_completion_list(&uld_0);

    completion_list_add_ums_context(uld_0, ucd_0);
    completion_list_add_ums_context(uld_0, ucd_1);
    completion_list_add_ums_context(uld_0, ucd_2);


    create_ums_completion_list(&uld_1);

    completion_list_add_ums_context(uld_1, ucd_0);
    completion_list_add_ums_context(uld_1, ucd_3);
    completion_list_add_ums_context(uld_1, ucd_4);

    // --------------------------------------------------

    ums_scheduler_descriptor_t sd_0;
    create_ums_scheduler(&sd_0, uld_0, entry_point, NULL, 0);


    ums_scheduler_descriptor_t sd_1;
    create_ums_scheduler(&sd_1, uld_1, entry_point, NULL, 1);

    // --------------------------------------------------


    join_scheduler(&sd_0, &ret);
    printf("ret sched = %d\n", ret);
    
    join_scheduler(&sd_1, &ret);
    printf("ret sched = %d\n", ret);

    delete_ums_completion_list(uld_0);
    delete_ums_completion_list(uld_1);


    delete_ums_context(ucd_0);
    delete_ums_context(ucd_1);
    delete_ums_context(ucd_2);
    delete_ums_context(ucd_3);
    delete_ums_context(ucd_4);
    
    ums_destroy();

    return EXIT_SUCCESS;
}


void entry_point(entry_point_args_t* entry_point_args){
    //sleep(1);
    int res;

    switch(entry_point_args->reason){
        case REASON_STARTUP:
            printf("CORE%d - Startup\n", sched_getcpu());

            res = execute_next_new_thread();
            if(res == -1)
                if(errno == ERR_EMPTY_COMP_LIST){
                    printf("CORE%d - %s(): REASON_STARTUP --- empty completion list\n", sched_getcpu(), __func__);
                    exit_scheduler(0);
                }
                else{
                    printf("CORE%d - %s(): ERROR! errno=%d\n", sched_getcpu(), __func__, errno);
                    exit_scheduler(-errno);
                }
        break;
    

        case REASON_THREAD_ENDED:
            printf("CORE%d - Thread_ended UCD=%d\n", sched_getcpu(), entry_point_args->activation_payload);

            res = execute_next_new_thread();
            if(res == -1){
                if(errno == ERR_EMPTY_COMP_LIST){
                    printf("CORE%d - %s(): REASON_THREAD_ENDED --- empty completion list\n", sched_getcpu(), __func__);
                    
                    res = execute_next_ready_thread();
                    if(res == -1){
                        if(errno == ERR_EMPTY_READY_LIST){
                            printf("CORE%d - ended ready list: BYE!\n", sched_getcpu());
                            exit_scheduler(0);
                        }
                        else {
                            printf("CORE%d - %s(): ERROR! errno=%d\n", sched_getcpu(), __func__, errno);
                            exit_scheduler(-errno);
                        }
                    }
                }
                else{
                    printf("CORE%d - %s(): ERROR! errno=%d\n", sched_getcpu(), __func__, errno);
                    exit_scheduler(-errno);
                }
            }
        break;

        case REASON_THREAD_YIELD:
            printf("CORE%d - Thread yielded UCD=%d\n", sched_getcpu(), entry_point_args->activation_payload);

            res = execute_next_new_thread();
            if(res == -1){
                if(errno == ERR_EMPTY_COMP_LIST){
                    printf("CORE%d - %s(): REASON_THREAD_YIELDED --- empty completion list\n", sched_getcpu(), __func__);
                    
                    res = execute_next_ready_thread();
                    if(res == -1){
                        if(errno == ERR_EMPTY_READY_LIST){
                            printf("CORE%d - ended ready list: BYE!\n", sched_getcpu());
                            exit_scheduler(0);
                        }
                        else{
                            printf("CORE%d - %s(): ERROR! errno=%d\n", sched_getcpu(), __func__, errno);
                            exit_scheduler(-errno);
                        }
                    }
                }
                else{
                    printf("CORE%d - %s(): ERROR! errno=%d\n", sched_getcpu(), __func__, errno);
                    exit_scheduler(-errno);
                }
            }
            
        break;

        default:
            printf("CORE%d - Unsupported REASON!\n", sched_getcpu());
        break;
    }
}
