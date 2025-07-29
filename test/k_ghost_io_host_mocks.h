/**
 * @brief Mock for k_ghost_io library
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
#include <sys/socket.h>
#include <unistd.h>

#include "fff.h"

/* Macro ---------------------------------------------------------------------*/
/* Typedef -------------------------------------------------------------------*/
typedef void *(*thread_cb_t)(void *argument);

/* Constant ------------------------------------------------------------------*/
/* Variable ------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
DECLARE_FAKE_VALUE_FUNC(int, socket, int, int, int)
DECLARE_FAKE_VALUE_FUNC(int, bind, int, const struct sockaddr *, socklen_t)
DECLARE_FAKE_VALUE_FUNC(int, listen, int, int)
DECLARE_FAKE_VALUE_FUNC(int, pthread_create, pthread_t *, const pthread_attr_t *, thread_cb_t, void *)
DECLARE_FAKE_VALUE_FUNC(int, accept, int, struct sockaddr *, socklen_t *)
DECLARE_FAKE_VALUE_FUNC(ssize_t, recv, int, void *, size_t, int)
DECLARE_FAKE_VALUE_FUNC(int, close, int)
DECLARE_FAKE_VALUE_FUNC(int, setsockopt, int, int, int, const void *, socklen_t)
DECLARE_FAKE_VALUE_FUNC(ssize_t, send, int, const void *, size_t, int)

#ifdef __cplusplus
}
#endif
/* @} */
