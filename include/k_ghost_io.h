/**
 * @brief Header file for k_ghost_io
 * @addtogroup k_ghost_io
 * @{
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* Include -------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>

#include "cJSON.h"
/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/**
 * @brief Callback function type for handling specific interface requests.
 *
 * @param input_data_p Pointer to the input data for the callback, in cJSON format.
 * @param user_data_p Pointer to provided user data.
 *
 * @return int Returns 0 on success, or -1 on failure.
 */
typedef int (*k_ghost_io_interface_callback_t)(const cJSON *input_data_p, void *user_data_p);

/**
 * @brief Callback function type for synchronizing the status of the system.
 *
 * This callback is used to synchronize the status of the system with the clients.
 */
typedef void (*k_ghost_io_sync_status_t)(void);

typedef enum
{
	K_GHOST_REGISTER_RET_CODE_ERROR				 = -2,	//!< Error occurred during registration
	K_GHOST_REGISTER_RET_CODE_ALREADY_REGISTERED = -1,	//!< Callback already registered for this interface
	K_GHOST_REGISTER_RET_CODE_OK				 = 0,	//!< Registration successful
} k_ghost_io_register_ret_code_t;

typedef struct
{
	int	  sse_client_fd;  //!< File descriptor for the SSE client
	void *next_client;	  //!< Pointer to the next client in the linked list
} k_ghost_io_sse_clients_list_t;

typedef struct
{
	char						   *interface_name;	 //!< Interface name this callback is used for
	k_ghost_io_interface_callback_t rest_cb;		 //!< Callback to be used for the specific hardware interface type
	k_ghost_io_sync_status_t		sync_cb;		 //!< Callback to be used for synchronizing the status of the system with the SSE clients
	void						   *user_data_p;	 //!< User data to be passed to the callback
	void						   *next_cb;		 //!< Pointer to the next REST API callback in the list
} k_ghost_io_interface_t;

/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/**
 * @brief Initialize the k_ghost_io system.
 *
 * @return int Returns 0 on success, or -1 on failure.
 */
int k_ghost_io_init(void);

/**
 * @brief Register a new interface with the k_ghost_io system.
 *
 * @param interface_name Name of the interface to register.
 * @param rest_cb Callback function for handling REST requests for this interface.
 * @param sync_cb Optional. Callback function for synchronizing the status of the system with the SSE clients.
 * @param user_data_p Optional. Pointer to user data to pass to callback.
 *
 * @return Returns registration status code. Refer to k_ghost_io_register_ret_code_t for possible values.
 */
k_ghost_io_register_ret_code_t k_ghost_io_register_interface(const char *interface_name, k_ghost_io_interface_callback_t rest_cb,
															 k_ghost_io_sync_status_t sync_cb, void *user_data_p);

/**
 * @brief Unregisters an interface from the ghost IO system.
 *
 * @param interface_name Pointer to a string representing the name of the interface to be unregistered.
 */
void k_ghost_io_unregister_interface(const char *interface_name);

/**
 * @brief Send the data payload to be sent via SSE to connected clients
 * @param data Pointer to data to send in SSE data payload
 */
void k_ghost_io_send_event(const char *data);
#ifdef __cplusplus
}
#endif
/* @} */