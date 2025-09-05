/**
 * @file k_ghost_io.c
 * @ingroup k_ghost_io
 * @{
 */

/* Include -------------------------------------------------------------------*/
#include "k_ghost_io.h"

#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cJSON.h"
#include "k_ghost_io_priv.h"

/* Macro ---------------------------------------------------------------------*/
#ifndef K_GHOST_IO_SERVER_PORT
#define K_GHOST_IO_SERVER_PORT 8080
#endif

#ifndef K_GHOST_IO_SSE_URI_PATH
#define K_GHOST_IO_SSE_URI_PATH "/api/sse"
#endif

#ifndef K_GHOST_IO_REST_URI_PATH
#define K_GHOST_IO_REST_URI_PATH "/api/simulate"
#endif

#define K_GHOST_IO_MAX_CLIENTS FD_SETSIZE

/* Typedef -------------------------------------------------------------------*/
/* Function Declaration ------------------------------------------------------*/
/* Constant ------------------------------------------------------------------*/
/* HTTP header for SSE events */
const char *k_ghost_io_sse_header =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/event-stream\r\n"
	"Cache-Control: no-cache\r\n"
	"Connection: keep-alive\r\n"
	"\r\n";

const char *k_ghost_io_sse_request_header  = "GET " K_GHOST_IO_SSE_URI_PATH " HTTP/1.1\r\n";
const char *k_ghost_io_rest_request_header = "POST " K_GHOST_IO_REST_URI_PATH " HTTP/1.1\r\n";

/* Variable ------------------------------------------------------------------*/
k_ghost_io_ctx_t k_ghost_io_ctx = {0};

/* Function Definition -------------------------------------------------------*/
int k_ghost_io_init(void)
{
	int ret_code = -1;
	if (0 == k_ghost_io_ctx.socket_fd)
	{
		k_ghost_io_ctx.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (-1 != k_ghost_io_ctx.socket_fd)
		{
			int				   socket_opt = 1;
			struct sockaddr_in server_addr;
			server_addr.sin_family		= AF_INET;
			server_addr.sin_addr.s_addr = INADDR_ANY;
			server_addr.sin_port		= htons(K_GHOST_IO_SERVER_PORT);
			if (0 == setsockopt(k_ghost_io_ctx.socket_fd, SOL_SOCKET, SO_REUSEADDR, &socket_opt, sizeof(socket_opt)) &&
				0 == bind(k_ghost_io_ctx.socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) && 0 == listen(k_ghost_io_ctx.socket_fd, 2) &&
				0 == pthread_create(&k_ghost_io_ctx.system_thread, NULL, k_ghost_io_thread_func, &k_ghost_io_ctx))
			{
				ret_code = 0;
			}
			else
			{
				close(k_ghost_io_ctx.socket_fd);
				k_ghost_io_ctx.socket_fd = 0;
			}
		}
		else
		{
			/* Failed to create socket. Reset fd in context */
			k_ghost_io_ctx.socket_fd = 0;
		}
	}
	else
	{
		/* Already initialized */
		ret_code = 0;
	}
	return ret_code;
}

k_ghost_io_register_ret_code_t k_ghost_io_register_interface(const char *interface_name, k_ghost_io_interface_callback_t rest_cb,
															 k_ghost_io_sync_status_t sync_cb, void *user_data_p)
{
	k_ghost_io_register_ret_code_t ret_code = K_GHOST_REGISTER_RET_CODE_ERROR;
	if (interface_name && rest_cb)
	{
		k_ghost_io_interface_t *interface_p = k_ghost_io_ctx.interfaces;
		while (interface_p)
		{
			if (0 == strcmp(interface_p->interface_name, interface_name))
			{
				ret_code = K_GHOST_REGISTER_RET_CODE_ALREADY_REGISTERED;
				break;
			}
			interface_p = (k_ghost_io_interface_t *)interface_p->next_cb;
		}
		if (K_GHOST_REGISTER_RET_CODE_ALREADY_REGISTERED != ret_code)
		{
			k_ghost_io_interface_t *new_interface = malloc(sizeof(k_ghost_io_interface_t));
			if (new_interface)
			{
				new_interface->interface_name = strdup(interface_name);
				new_interface->rest_cb		  = rest_cb;
				new_interface->sync_cb		  = sync_cb;
				new_interface->user_data_p	  = user_data_p;
				new_interface->next_cb		  = k_ghost_io_ctx.interfaces;
				k_ghost_io_ctx.interfaces	  = new_interface;
				ret_code					  = K_GHOST_REGISTER_RET_CODE_OK;
			}
		}
	}
	return ret_code;
}

void k_ghost_io_send_event(const char *data)
{
	if (data)
	{
		k_ghost_io_sse_clients_list_t *current			= k_ghost_io_ctx.sse_clients;
		const char					  *sse_event_header = "data: ";
		const size_t				   needed_space		= strlen(sse_event_header) + strlen(data) + 5;	// +3 for \r\n\r\n\0
		char						  *sse_data			= malloc(needed_space);
		if (sse_data)
		{
			snprintf(sse_data, needed_space, "%s%s\r\n\r\n", sse_event_header, data);
			while (current)
			{
				if (current->sse_client_fd > 0)
				{
					send(current->sse_client_fd, sse_data, strlen(sse_data), 0);
				}
				current = current->next_client;
			}
			free(sse_data);
		}
	}
}

void *k_ghost_io_thread_func(void *arg)
{
	k_ghost_io_ctx_t *ctx_p								 = (k_ghost_io_ctx_t *)arg;
	int				  client_fds[K_GHOST_IO_MAX_CLIENTS] = {0};
	while (1)
	{
		/* Register socket FD into the readfds list */
		FD_ZERO(&ctx_p->readfds);
		FD_SET(ctx_p->socket_fd, &ctx_p->readfds);
		int max_fd = ctx_p->socket_fd;

		/* Check clients we need to insert in the set. This is useful once we get at least one connection to reinsert it in the set */
		for (int i = 0; i < K_GHOST_IO_MAX_CLIENTS; i++)
		{
			if (client_fds[i] > 0)
			{
				FD_SET(client_fds[i], &ctx_p->readfds);
				if (client_fds[i] > max_fd)
				{
					max_fd = client_fds[i];
				}
			}
		}

		/* Wait for new connection and/or new content from clients */
		if (select(max_fd + 1, &ctx_p->readfds, NULL, NULL, NULL) > 0)
		{
			/* New connection */
			if (FD_ISSET(ctx_p->socket_fd, &ctx_p->readfds))
			{
				struct sockaddr_in client_addr;
				socklen_t		   client_len = sizeof(client_addr);
				int				   new_fd	  = accept(ctx_p->socket_fd, (struct sockaddr *)&client_addr, &client_len);
				if (new_fd > 0)
				{
					k_ghost_io_add_connection(client_fds, new_fd);
				}
			}

			/* Check if clients have sent something */
			for (int i = 0; i < K_GHOST_IO_MAX_CLIENTS; i++)
			{
				int unblocked_fd = client_fds[i];
				if (unblocked_fd > 0 && FD_ISSET(unblocked_fd, &ctx_p->readfds))
				{
					char	buffer[1024] = {0};
					ssize_t bytes		 = recv(unblocked_fd, buffer, sizeof(buffer) - 1, 0);
					if (bytes <= 0)
					{
						/* Client closed the connection */
						close(unblocked_fd);
						k_ghost_io_remove_sse_client(unblocked_fd);
						client_fds[i] = 0;
					}
					else if (strncmp(buffer, k_ghost_io_sse_request_header, strlen(k_ghost_io_sse_request_header)) == 0)
					{
						/* Client opened a connection towards the SSE endpoint. We need to keep the connection open */
						k_ghost_io_add_sse_client(unblocked_fd);
					}
					else if (strncmp(buffer, k_ghost_io_rest_request_header, strlen(k_ghost_io_rest_request_header)) == 0)
					{
						/* Client opened a connection towards the REST endpoint. We need to answer back and close the connection */
						k_ghost_io_manage_rest_request(unblocked_fd, buffer);
						client_fds[i] = 0;
					}
					else
					{
						/* Unknown request, we can close the connection after sending a 404 responses */
						k_ghost_io_manage_unknown_endpoint(unblocked_fd);
						client_fds[i] = 0;
					}
				}
			}
		}
	}
	return NULL;
}

int k_ghost_io_add_sse_client(const int sse_client_fd)
{
	int ret_code = -1;
	if (sse_client_fd > 0)
	{
		k_ghost_io_sse_clients_list_t *new_client = malloc(sizeof(k_ghost_io_sse_clients_list_t));
		if (new_client)
		{
			new_client->sse_client_fd  = sse_client_fd;
			new_client->next_client	   = k_ghost_io_ctx.sse_clients;
			k_ghost_io_ctx.sse_clients = new_client;
			send(sse_client_fd, k_ghost_io_sse_header, strlen(k_ghost_io_sse_header), 0);
			k_ghost_io_interface_t *interface_p = k_ghost_io_ctx.interfaces;
			while (interface_p)
			{
				if (interface_p->sync_cb)
				{
					interface_p->sync_cb();	 // Call the sync callback to send current interface status
				}
				interface_p = (k_ghost_io_interface_t *)interface_p->next_cb;
			}
			ret_code = 0;
		}
	}
	return ret_code;
}

void k_ghost_io_remove_sse_client(const int sse_client_fd)
{
	if (sse_client_fd > 0)
	{
		k_ghost_io_sse_clients_list_t *current = k_ghost_io_ctx.sse_clients;
		k_ghost_io_sse_clients_list_t *prev	   = NULL;

		while (current)
		{
			if (sse_client_fd == current->sse_client_fd)
			{
				if (prev)
				{
					/* Link previous entry to the next one */
					prev->next_client = current->next_client;
				}
				else
				{
					/* This was the first entry of the list */
					k_ghost_io_ctx.sse_clients = current->next_client;
				}
				free(current);
				break;
			}
			prev	= current;
			current = current->next_client;
		}
	}
}

void k_ghost_io_add_connection(int *client_a, const int new_connection_fd)
{
	for (int i = 0; i < K_GHOST_IO_MAX_CLIENTS; i++)
	{
		if (client_a[i] == 0)
		{
			client_a[i] = new_connection_fd;
			break;
		}
	}
}

void k_ghost_io_manage_rest_request(int client_fd, const char *request)
{
	if (request)
	{
		char  *request_body	   = strstr(request, "\r\n\r\n");
		cJSON *json_request	   = NULL;
		int	   interface_found = 0;
		if (request_body)
		{
			json_request = cJSON_Parse(request_body + 4);  // Skip the "\r\n\r\n"
			if (json_request)
			{
				char *interface = cJSON_GetStringValue(cJSON_GetObjectItem(json_request, "interface"));
				if (interface)
				{
					k_ghost_io_interface_t *interface_p = k_ghost_io_ctx.interfaces;
					while (interface_p)
					{
						if (0 == strcmp(interface_p->interface_name, interface))
						{
							interface_found = 1;
							int ret_code	= interface_p->rest_cb(json_request, interface_p->user_data_p);
							if (0 == ret_code)
							{
								const char *resp = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
								send(client_fd, resp, strlen(resp), 0);
							}
							else
							{
								const char *resp = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
								send(client_fd, resp, strlen(resp), 0);
							}
							break;
						}
						interface_p = (k_ghost_io_interface_t *)interface_p->next_cb;
					}
				}
				cJSON_Delete(json_request);
				if (!interface_found)
				{
					const char *resp = "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n";
					send(client_fd, resp, strlen(resp), 0);
				}
			}
			else
			{
				const char *resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
				send(client_fd, resp, strlen(resp), 0);
			}
		}
		else
		{
			const char *resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
			send(client_fd, resp, strlen(resp), 0);
		}
	}
	else
	{
		const char *resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
		send(client_fd, resp, strlen(resp), 0);
	}
	close(client_fd);
}

void k_ghost_io_manage_unknown_endpoint(const int client_fd)
{
	const char *resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
	send(client_fd, resp, strlen(resp), 0);
	close(client_fd);
}