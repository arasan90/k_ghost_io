/**
 * @brief Private header file for k_ghost_io
 * @addtogroup k_ghost_io
 * @{
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* Include -------------------------------------------------------------------*/
#include <pthread.h>
#include <sys/select.h>

#include "k_ghost_io.h"
/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
typedef struct
{
	int							   socket_fd;	   //!< File descriptor for the server socket
	pthread_t					   system_thread;  //!< Thread for handling system operations
	fd_set						   readfds;		   //!< File descriptor set for read operations with select
	k_ghost_io_sse_clients_list_t *sse_clients;	   //!< Pointer to the linked list of SSE clients
	k_ghost_io_interface_t		  *interfaces;	   //!< Pointer to the registered interfaces
} k_ghost_io_ctx_t;

/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/**
 * @brief Thread's main function.
 * @param arg Pointer to the context structure.
 *
 * @return Pointer to the result of the thread execution.
 */
void *k_ghost_io_thread_func(void *arg);

/**
 * @brief Add the SSE client to the linked list.
 * @param sse_client_fd File descriptor of the SSE client to be added.
 *
 * @return 0 in case of success, -1 in case of failure.
 */
int k_ghost_io_add_sse_client(int sse_client_fd);

/**
 * @brief Remove the SSE client from the linked list.
 * @param sse_client_fd File descriptor of the SSE client to be removed.
 */
void k_ghost_io_remove_sse_client(int sse_client_fd);

/**
 * @brief Add a new client to the list of known clients
 *
 * @param client_a Array of known clients
 * @param new_connection_fd File descriptor of the newly connected client
 */
void k_ghost_io_add_connection(int *client_a, int new_connection_fd);

/**
 * @brief Manage REST requests
 *
 * @param client_fd File descriptor of the client that sent a new REST request
 * @param request Pointer to the request data
 */
void k_ghost_io_manage_rest_request(int client_fd, const char *request);

/**
 * @brief Manage the requests to unknown endpoints
 *
 * @param client_fd File descriptor of the client that sent a request to an unknown request
 */
void k_ghost_io_manage_unknown_endpoint(int client_fd);

#ifdef __cplusplus
}
#endif
/* @} */