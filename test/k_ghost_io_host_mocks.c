/**
 * @file k_ghost_io_mock.c
 * @ingroup k_ghost_io_mock
 * @{
 */

/* Include -------------------------------------------------------------------*/
#include "k_ghost_io_host_mocks.h"

/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Definition -------------------------------------------------------*/
DEFINE_FFF_GLOBALS

DEFINE_FAKE_VALUE_FUNC(int, socket, int, int, int)
DEFINE_FAKE_VALUE_FUNC(int, bind, int, const struct sockaddr *, socklen_t)
DEFINE_FAKE_VALUE_FUNC(int, listen, int, int)
DEFINE_FAKE_VALUE_FUNC(int, pthread_create, pthread_t *, const pthread_attr_t *, thread_cb_t, void *)
DEFINE_FAKE_VALUE_FUNC(int, accept, int, struct sockaddr *, socklen_t *)
DEFINE_FAKE_VALUE_FUNC(ssize_t, recv, int, void *, size_t, int)
DEFINE_FAKE_VALUE_FUNC(int, close, int)
DEFINE_FAKE_VALUE_FUNC(int, setsockopt, int, int, int, const void *, socklen_t)
DEFINE_FAKE_VALUE_FUNC(ssize_t, send, int, const void *, size_t, int)
