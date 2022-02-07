#pragma once
/// @file 
/// This file contains all functions available to the user
///

#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include "../../common/ums_requests.h"
#include <stdint.h>

typedef pthread_t ums_scheduler_descriptor_t;

extern pid_t tgid;
extern int ums_fd;

/**
 * Initializes UMS
 * 
 * Opens the UMS virtual device and creates a ums_process entity, it performs a RQ_CREATE_PROCESS request
 * 
 * @return Returns 0 on sucess, otherwise -1 and sets errno according to 
 */
res_t ums_init(void);

/**
 * Destroys UMS 
 * 
 * Closes the UMS virtual device and destorys the ums_process entity, it performs a RQ_DELETE_PROCESS request
 * 
 * @return Returns 0 on sucess, otherwise -1 and sets errno according to 
 */
res_t ums_destroy(void);


/**
 * Creates a ums_context object
 * 
 * It performs a RQ_CREATE_UMS_CONTEXT request
 * @param descriptor Pointer used to save the ums_context_descriptor assigned
 * @param routine Function poiter to the routine of the new ums_context
 * @param args Arguments to be passed to the ums_context's routine 
 * @param user_res user managed object
 * 
 * @return Returns 0 on sucess, otherwise -1 and sets errno according to  
 */
res_t create_ums_context(ums_context_descriptor_t* descriptor,void* (*routine)(void*), void* args, void* user_res);


/**
 * Deletes a ums_context
 * 
 * Deletes the ums_context indicated by descriptor parameter, it performs a RQ_DELETE_UMS_CONTEXT request
 * @param descriptor Descriptor of the ums_context to delete
 * 
 * @return Returns 0 on sucess, otherwise -1 and sets errno according to   
 */
res_t delete_ums_context(ums_context_descriptor_t descriptor);


/**
 * @brief Create a ums completion list object
 * 
 * It performs a RQ_CREATE_COMPLETION_LIST request
 * @param ums_completion_list_descriptor Pointer to where to store the descriptor of the new ums_completions_list
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to   
 */
res_t create_ums_completion_list(ums_completion_list_descriptor_t* ums_completion_list_descriptor);

/**
 * @brief Delete a ums completion list object
 * 
 * It performs a RQ_REMOVE_COMPLETION_LIST request
 * @param ums_completion_list_descriptor Descriptor of the ums_completion_list to delete 
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to   
 */
res_t delete_ums_completion_list(ums_completion_list_descriptor_t ums_completion_list_descriptor);

/**
 * @brief Add a ums_context to a ums_completion_list
 * 
 * It performs a RQ_COMPLETION_LIST_ADD_UMS_CONTEXT request
 * @param completion_list_d Descriptor of the ums_completion_list 
 * @param ums_context_d Descriptor of the ums_context to add
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to   
 */
res_t completion_list_add_ums_context(ums_completion_list_descriptor_t completion_list_d, ums_context_descriptor_t ums_context_d);

/**
 * @brief Remove a ums_context from a ums_completion_list
 * 
 * It performs a RQ_COMPLETION_LIST_REMOVE_UMS_CONTEXT request
 * @param completion_list_d Descriptor of the ums_completion_list 
 * @param ums_context_d  Descriptor of the ums_context to remove
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to 
 */
res_t completion_list_remove_ums_context(ums_completion_list_descriptor_t completion_list_d, ums_context_descriptor_t ums_context_d);



/**
 * @brief Create a ums scheduler object
 * 
 * It creates a new thread that will be used as scheduler. The new thread performs a RQ_CREATE_UMS_SCHEDULER request
 * NOTE: entry_point must have a specific structure composed by a swicth-case statement 
 * 
 * @param sd Pointer used to store the descriptor of the new ums_scheduler
 * @param cd Descriptor of the ums_completion_list to use
 * @param entry_point Entry_point function of the scheduler
 * @param sched_args Arguments to pass to entry_point functions
 * @param cpu_core CPU core to use
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to 
 */
res_t create_ums_scheduler(ums_scheduler_descriptor_t* sd, ums_completion_list_descriptor_t cd, void(*entry_point)(entry_point_args_t* entry_point_args), void* sched_args, int cpu_core);

/**
 * @brief exit() function for the scheduler
 * 
 * This function must be used in the entry_point function and replaces the classic exit() syscall. It performs a RQ_EXIT_SCHEDULER request
 * NOTE: The user MUST use this function in order to terminate the scheduler's thread
 * @param return_value Exit value of the scheduler thread
 */
void exit_scheduler(int return_value);

/**
 * @brief Execute the next ums_context in the ums_completion_list of the scheduler
 * 
 * It performs a RQ_EXECUTE_NEXT_NEW_THREAD request to get the routine and the arguments of the ums_context then it creates a new threads
 * NOTE: It always returns a value, due to the fact that at every call of the entry_point of the scheduler, the entire entry_point function is executed!
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to 
 */
res_t execute_next_new_thread(void);

/**
 * @brief Execute the next ums_context in the ready_list of the scheduler
 * 
 * It performs a RQ_EXECUTE_NEXT_READY_THREAD request 
 * NOTE: It always returns a value, due to the fact that at every call of the entry_point of the scheduler, the entire entry_point function is executed!
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to 
 */
res_t execute_next_ready_thread(void);



/**
 * @brief Join scheduler thread
 * 
 * It waits the end of the scheduler thread indicated by the ums_scheduler_descriptor pointer
 * @param usd Poiter to the ums_scheduler_descriptor
 * @param return_value Pointer to where store the return value of the scheduler thread
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to
 */
res_t join_scheduler(ums_scheduler_descriptor_t* usd, int* return_value);

/**
 * @brief Current ums_context in execution leaves the control to the scheduler
 * 
 * It performs a RQ_YIELD_UMS_CONTEXT request 
 * @return res_t Returns 0 on sucess, otherwise -1 and sets errno according to 
 */
res_t yield(void);

/**
 * @brief Get the ums contexts from the completion_list of the scheduler
 * 
 * @param array_info_ums_context output, array of info_ums_context_t
 * @param array_size input, size of array, maximum number of ums_context to read
 * @return return (>0) the number of context readed, return -1 on failure and set errno according to
 * 
 * NOTE: After a successfully execution, execute() function must be always called
 */
res_t get_ums_contexts_from_cl(info_ums_context_t* array_info_ums_context, size_t array_size);

/**
 * @brief Get the ums contexts from the ready_list of the scheduler
 * 
 * @param array_info_ums_context output, array of info_ums_context_t
 * @param array_size input, size of array, maximum number of ums_context to read
 * @return return (>0) the number of context readed, return -1 on failure and set errno according to
 * 
 * NOTE: After a successfully execution, execute() function must be always called
 */
res_t get_ums_contexts_from_rl(info_ums_context_t* array_info_ums_context, size_t array_size);

/**
 * @brief execute a ums_context identified by a pointer to a info_ums_context_t
 * 
 * @param info_ums_context pointer to the info_ums_context_t object to execute
 * @return return 0 on success, return -1 on failure and set errno according to
 */
res_t execute(info_ums_context_t* info_ums_context);