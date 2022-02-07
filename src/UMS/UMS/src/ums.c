#include "ums.h"
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <errno.h>
pid_t tgid = -1;
int ums_fd = -1;

static inline int create_process(pid_t tgid);
static inline int delete_process(pid_t tgid);


// -----------------------------------------------------------------------------------------------------


res_t ums_init(){
    int res;
    ums_fd = open("/dev/UMS", 0);
    if(ums_fd == -1){
        errno = ERR_INTERNAL;
        return -1;    
    }
    tgid = getpid();
    //NOTE: getpid() return tgid
    //      gettid() return thread-s pid
    res = create_process(tgid);
    if(res == -1){
        return -1;   
    }
    return SUCCESS;
}
res_t ums_destroy(){
    int res;
    res = delete_process(tgid);
    if(res == -1){
        return -1;   
    }

    res = close(ums_fd);
    if(res == -1){
        return -1;
    }
    return SUCCESS;
}


static inline int create_process(pid_t tgid){
    int res;
    rq_create_delete_process_args_t args = {
        .tgid = tgid
    };
    res = ioctl(ums_fd, RQ_CREATE_PROCESS, &args);
    return (res == -1)?errno:SUCCESS;
}
static inline int delete_process(pid_t tgid){
    int res;
    rq_create_delete_process_args_t args = {
        .tgid = tgid
    };
    res = ioctl(ums_fd, RQ_DELETE_PROCESS, &args);
    return (res == -1)?errno:SUCCESS; 
}
