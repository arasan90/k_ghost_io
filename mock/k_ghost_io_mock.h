/**
* @brief Mock for k_ghost_io library
 * @addtogroup k_ghost_io_mock
 * @{
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

	/* Include -------------------------------------------------------------------*/
#include "fff.h"
#include "k_ghost_io.h"
	/* Macro ---------------------------------------------------------------------*/
	/* Typedef -------------------------------------------------------------------*/
	/* Constant ------------------------------------------------------------------*/
	/* Variable ------------------------------------------------------------------*/
	/* Function Declaration ------------------------------------------------------*/
	DECLARE_FAKE_VALUE_FUNC(int, k_ghost_io_init)
	DECLARE_FAKE_VALUE_FUNC(k_ghost_io_register_ret_code_t, k_ghost_io_register_interface, const char *, k_ghost_io_interface_callback_t,
																 k_ghost_io_sync_status_t)
DECLARE_FAKE_VOID_FUNC(k_ghost_io_send_event, const char *)

	#ifdef __cplusplus
	}
#endif
/* @} */