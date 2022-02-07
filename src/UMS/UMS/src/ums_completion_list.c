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

// -----------------------------------------------------------------------------------------------------
res_t create_ums_completion_list(ums_completion_list_descriptor_t* ums_completion_list_descriptor){
    rq_create_delete_completion_list_args_t rq_args = {
        .tgid = tgid
    };
    res_t res = ioctl(ums_fd, RQ_CREATE_COMPLETION_LIST, &rq_args);
    *ums_completion_list_descriptor = rq_args.descriptor;
    return res;
}
res_t delete_ums_completion_list(ums_completion_list_descriptor_t ums_completion_list_descriptor){
    rq_create_delete_completion_list_args_t rq_args = {
        .tgid = tgid,
        .descriptor = ums_completion_list_descriptor
    };
    return ioctl(ums_fd, RQ_DELETE_COMPLETION_LIST, &rq_args);
}
// -----------------------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------------------
res_t completion_list_add_ums_context(ums_completion_list_descriptor_t completion_list_d, ums_context_descriptor_t ums_context_d){
    rq_completion_list_add_remove_ums_context_args_t rq_args = {
        .tgid = tgid,
        .completion_list_d = completion_list_d,
        .ums_context_d = ums_context_d
    };
    return ioctl(ums_fd, RQ_COMPLETION_LIST_ADD_UMS_CONTEXT, &rq_args);
}
res_t completion_list_remove_ums_context(ums_completion_list_descriptor_t completion_list_d, ums_context_descriptor_t ums_context_d){
    rq_completion_list_add_remove_ums_context_args_t rq_args = {
        .tgid = tgid,
        .completion_list_d = completion_list_d,
        .ums_context_d = ums_context_d
    };
    return ioctl(ums_fd, RQ_COMPLETION_LIST_REMOVE_UMS_CONTEXT, &rq_args);
}
// -----------------------------------------------------------------------------------------------------
