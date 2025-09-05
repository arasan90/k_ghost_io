/**
 * @file k_ghost_io_mock.c
 * @ingroup k_ghost_io_mock
 * @{
 */

/* Include -------------------------------------------------------------------*/
#include "k_ghost_io_mock.h"

/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Definition -------------------------------------------------------*/
DEFINE_FAKE_VALUE_FUNC(int, k_ghost_io_init)
DEFINE_FAKE_VALUE_FUNC(k_ghost_io_register_ret_code_t, k_ghost_io_register_interface, const char *, k_ghost_io_interface_callback_t, k_ghost_io_sync_status_t,
					   void *)
DEFINE_FAKE_VOID_FUNC(k_ghost_io_unregister_interface, const char *)
DEFINE_FAKE_VOID_FUNC(k_ghost_io_send_event, const char *)