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

typedef struct my_info_t{
    int prio;
}my_info_t;

void entry_point(entry_point_args_t* entry_point_args);

void* routine(void* args){
    int value = *((int*) args);
    int i;
    for(i=0; i<5; i++){
        printf("Hello world!---%d---\n", value);
	    sleep(1);
        if(i ==2)  {
            printf("YIELD\n");
            yield();
        }
    }

    return NULL;
}

int main(int argc, char **argv){
    int ret;
    ums_init();
    // ----------------------------------------------
    ums_context_descriptor_t ucd_0;
    my_info_t info_0 = {
        .prio = 4
    };

    ums_context_descriptor_t ucd_1;
    my_info_t info_1 = {
        .prio = 3
    };

    ums_context_descriptor_t ucd_2;
    my_info_t info_2 = {
        .prio = 2
    };


    // -----------------------------------------------



    ums_completion_list_descriptor_t uld_0;

    
    create_ums_context(&ucd_0, &routine, &value_0, &info_0);
    create_ums_context(&ucd_1, &routine, &value_1, &info_1);
    create_ums_context(&ucd_2, &routine, &value_2, &info_2);


    

    create_ums_completion_list(&uld_0);

    completion_list_add_ums_context(uld_0, ucd_0);
    completion_list_add_ums_context(uld_0, ucd_1);
    completion_list_add_ums_context(uld_0, ucd_2);


    // --------------------------------------------------

    ums_scheduler_descriptor_t sd_0;
    create_ums_scheduler(&sd_0, uld_0, entry_point, NULL, -1);

    // --------------------------------------------------


    join_scheduler(&sd_0, &ret);
    printf("ret sched = %d\n", ret);
    

    delete_ums_completion_list(uld_0);



    delete_ums_context(ucd_0);
    delete_ums_context(ucd_1);
    delete_ums_context(ucd_2);

    
    ums_destroy();

    return EXIT_SUCCESS;
}


void entry_point(entry_point_args_t* entry_point_args){
    //sleep(1);
    int res;
    info_ums_context_t info_ums_context[5];
    info_ums_context_t* iuc_to_exec;
    my_info_t* my_info;

    int best_prio;
    int best_id;

    switch(entry_point_args->reason){
        case REASON_STARTUP:
            printf("Startup\n");

            res = get_ums_contexts_from_cl(info_ums_context, 5);
            if(res > 0){
                for(int i=0; i<res; i++){
                    my_info = info_ums_context[i].user_reserved;
                    printf("cl : info: id=%d, "
                                        "time%u, " 
                                        "ns=%d, "
                                        "user=%d"
                                        "\n"
                                        , 
                                        info_ums_context[i].ucd,
                                        info_ums_context[i].run_time_ms, 
                                        info_ums_context[i].number_switch, 
                                        my_info->prio);
                }
                iuc_to_exec = &(info_ums_context[0]);
                res = execute(iuc_to_exec);
                if(res == -1){
                    if(errno==ERR_ASSIGNED){
                        printf("ums context already assigned, I will try the next one\n");
                        res = execute_next_new_thread();
                        if(res == -1){
                            printf("error during execute_next_new_thread(). Exit\n");
                            exit_scheduler(EXIT_FAILURE);
                        }
                   }
                   else{
                       printf("unexpected errno=%d\n", errno);
                       exit_scheduler(EXIT_FAILURE);
                   }
                }
            }
            else{
                printf("empty completion list at startup. Exit\n");
                exit_scheduler(EXIT_FAILURE);
            }
        break;


        default:
            printf("Thread yielded or ended UCD=%d\n", entry_point_args->activation_payload);

           res = execute_next_new_thread();
            if(res == -1){
                if(errno == ERR_EMPTY_COMP_LIST){
                    printf("%s(): REASON_THREAD_YIELDED --- empty completion list\n", __func__);
                    res = get_ums_contexts_from_rl(info_ums_context, 5);
                    if(res > 0){
                        best_prio = 99;
                        best_id = -1;

                        for(int i=0; i<res; i++){
                            my_info = (my_info_t*) info_ums_context[i].user_reserved;

                            printf("rl : info: id=%d, "
                                                "time%u, " 
                                                "ns=%d, "
                                                "user=%d"
                                                "\n"
                                                , 
                                                info_ums_context[i].ucd,
                                                info_ums_context[i].run_time_ms, 
                                                info_ums_context[i].number_switch, 
                                                my_info->prio);

                            if(my_info->prio < best_prio){
                                best_prio = my_info->prio;
                                best_id = info_ums_context[i].ucd;
                            }
                        }
                        printf("chosen %d\n", best_id);
                        iuc_to_exec = &(info_ums_context[best_id]);
                        res = execute(iuc_to_exec);
                        if(res == -1){
                            printf("unexpected errno=%d\n", errno);
                            exit_scheduler(EXIT_FAILURE);
                        }
                    }
                    else if(errno == ERR_EMPTY_READY_LIST){
                        printf("ready list ended\n");
                        exit_scheduler(0);
                    }
                    else{
                        printf("unexpected errno=%d\n", errno);
                        exit_scheduler(EXIT_FAILURE);
                    }
                }
                else{
                    printf("unexpected errno=%d\n", errno);
                    exit_scheduler(EXIT_FAILURE);
                }
            }            
        break;


    }
}

