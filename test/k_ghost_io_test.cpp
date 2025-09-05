#include "k_ghost_io.h"

#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include "fff.h"
#include "k_ghost_io_host_mocks.h"
#include "k_ghost_io_priv.h"

extern k_ghost_io_ctx_t k_ghost_io_ctx;

class KGhostIOTest : public ::testing::Test
{
   protected:
	void SetUp() override
	{
		RESET_FAKE(socket);
		RESET_FAKE(bind);
		RESET_FAKE(listen);
		RESET_FAKE(pthread_create);
		RESET_FAKE(accept);
		RESET_FAKE(recv);
		RESET_FAKE(close);
		RESET_FAKE(setsockopt);
		RESET_FAKE(send);
		memset(&k_ghost_io_ctx, 0, sizeof(k_ghost_io_ctx_t));
	}

	void TearDown() override
	{
		k_ghost_io_interface_t *interface_p = k_ghost_io_ctx.interfaces;
		while (interface_p)
		{
			k_ghost_io_interface_t *next = (k_ghost_io_interface_t *)interface_p->next_cb;
			free(interface_p->interface_name);
			free(interface_p);
			interface_p = next;
		}
		memset(&k_ghost_io_ctx, 0, sizeof(k_ghost_io_ctx_t));
	}
};

TEST(system, initSuccess)
{
	RESET_FAKE(socket);
	socket_fake.return_val		   = 3;
	bind_fake.return_val		   = 0;
	listen_fake.return_val		   = 0;
	pthread_create_fake.return_val = 0;
	setsockopt_fake.return_val	   = 0;
	EXPECT_EQ(k_ghost_io_init(), 0);
	EXPECT_EQ(k_ghost_io_ctx.socket_fd, 3);
	EXPECT_EQ(close_fake.call_count, 0);
	EXPECT_EQ(socket_fake.call_count, 1);
}

TEST(system, alreadyInitializedSuccess)
{
	RESET_FAKE(socket);
	socket_fake.return_val		   = 3;
	bind_fake.return_val		   = 0;
	listen_fake.return_val		   = 0;
	pthread_create_fake.return_val = 0;
	setsockopt_fake.return_val	   = 0;
	EXPECT_EQ(k_ghost_io_init(), 0);
	EXPECT_EQ(k_ghost_io_init(), 0);
	EXPECT_EQ(k_ghost_io_ctx.socket_fd, 3);
	EXPECT_EQ(close_fake.call_count, 0);
	EXPECT_EQ(socket_fake.call_count, 1);
}

TEST(system, initFailForSocket)
{
	RESET_FAKE(socket);
	socket_fake.return_val		   = -1;
	bind_fake.return_val		   = 0;
	listen_fake.return_val		   = 0;
	pthread_create_fake.return_val = 0;
	EXPECT_EQ(k_ghost_io_init(), -1);
	EXPECT_EQ(k_ghost_io_ctx.socket_fd, 0);
	EXPECT_EQ(close_fake.call_count, 0);
	EXPECT_EQ(socket_fake.call_count, 1);
}

TEST(system, initFailForBind)
{
	RESET_FAKE(socket);
	socket_fake.return_val		   = 3;
	bind_fake.return_val		   = -1;
	listen_fake.return_val		   = 0;
	pthread_create_fake.return_val = 0;
	EXPECT_EQ(k_ghost_io_init(), -1);
	EXPECT_EQ(k_ghost_io_ctx.socket_fd, 0);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(socket_fake.call_count, 1);
}

TEST(system, initFailForListen)
{
	RESET_FAKE(socket);
	socket_fake.return_val		   = 3;
	bind_fake.return_val		   = 0;
	listen_fake.return_val		   = -1;
	pthread_create_fake.return_val = 0;
	EXPECT_EQ(k_ghost_io_init(), -1);
	EXPECT_EQ(k_ghost_io_ctx.socket_fd, 0);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(socket_fake.call_count, 1);
}

TEST(system, initFailForThreadCreate)
{
	RESET_FAKE(socket);
	socket_fake.return_val		   = 3;
	bind_fake.return_val		   = 0;
	listen_fake.return_val		   = 0;
	pthread_create_fake.return_val = -1;
	EXPECT_EQ(k_ghost_io_init(), -1);
	EXPECT_EQ(k_ghost_io_ctx.socket_fd, 0);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(socket_fake.call_count, 1);
}

TEST_F(KGhostIOTest, KGhostIOAddSseClientSuccess)
{
	int							   sse_client_fd  = 5;
	k_ghost_io_sse_clients_list_t *sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_EQ(sse_client_ptr, nullptr);
	EXPECT_EQ(k_ghost_io_add_sse_client(sse_client_fd), 0);
	sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_NE(sse_client_ptr, nullptr);
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd);
	EXPECT_EQ(sse_client_ptr->next_client, nullptr);
	free(sse_client_ptr);
}

TEST_F(KGhostIOTest, KGhostIOAdd2SseClientsSuccess)
{
	int							   sse_client_fd1 = 5;
	int							   sse_client_fd2 = 9;
	k_ghost_io_sse_clients_list_t *sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_EQ(sse_client_ptr, nullptr);
	EXPECT_EQ(k_ghost_io_add_sse_client(sse_client_fd1), 0);
	sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_NE(sse_client_ptr, nullptr);
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd1);
	EXPECT_EQ(sse_client_ptr->next_client, nullptr);

	EXPECT_EQ(k_ghost_io_add_sse_client(sse_client_fd2), 0);
	sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_NE(sse_client_ptr, nullptr);
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd2);
	EXPECT_NE(sse_client_ptr->next_client, nullptr);
	sse_client_ptr = (k_ghost_io_sse_clients_list_t *)k_ghost_io_ctx.sse_clients->next_client;
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd1);
	EXPECT_EQ(sse_client_ptr->next_client, nullptr);

	k_ghost_io_remove_sse_client(sse_client_fd1);
	k_ghost_io_remove_sse_client(sse_client_fd2);
}

TEST_F(KGhostIOTest, KGhostIORemoveSingleSseClientSuccess)
{
	int							   sse_client_fd  = 5;
	k_ghost_io_sse_clients_list_t *sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_EQ(sse_client_ptr, nullptr);
	k_ghost_io_add_sse_client(sse_client_fd);
	k_ghost_io_remove_sse_client(sse_client_fd);
	sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_EQ(sse_client_ptr, nullptr);
}

TEST_F(KGhostIOTest, KGhostIORemoveHeadSseClientSuccess)
{
	int							   sse_client_fd1 = 5;
	int							   sse_client_fd2 = 9;
	k_ghost_io_sse_clients_list_t *sse_client_ptr = k_ghost_io_ctx.sse_clients;
	k_ghost_io_add_sse_client(sse_client_fd1);
	k_ghost_io_add_sse_client(sse_client_fd2);
	k_ghost_io_remove_sse_client(sse_client_fd2);
	sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_NE(sse_client_ptr, nullptr);
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd1);
	EXPECT_EQ(sse_client_ptr->next_client, nullptr);
	k_ghost_io_remove_sse_client(sse_client_fd1);
}

TEST_F(KGhostIOTest, KGhostIORemoveTailSseClientSuccess)
{
	int							   sse_client_fd1 = 5;
	int							   sse_client_fd2 = 9;
	k_ghost_io_sse_clients_list_t *sse_client_ptr = k_ghost_io_ctx.sse_clients;
	k_ghost_io_add_sse_client(sse_client_fd1);
	k_ghost_io_add_sse_client(sse_client_fd2);
	k_ghost_io_remove_sse_client(sse_client_fd1);
	sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_NE(sse_client_ptr, nullptr);
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd2);
	EXPECT_EQ(sse_client_ptr->next_client, nullptr);
	k_ghost_io_remove_sse_client(sse_client_fd2);
}

TEST_F(KGhostIOTest, KGhostIORemoveCentralSseClientSuccess)
{
	int							   sse_client_fd1 = 5;
	int							   sse_client_fd2 = 9;
	int							   sse_client_fd3 = 12;
	k_ghost_io_sse_clients_list_t *sse_client_ptr = k_ghost_io_ctx.sse_clients;
	k_ghost_io_add_sse_client(sse_client_fd1);
	k_ghost_io_add_sse_client(sse_client_fd2);
	k_ghost_io_add_sse_client(sse_client_fd3);
	k_ghost_io_remove_sse_client(sse_client_fd2);
	sse_client_ptr = k_ghost_io_ctx.sse_clients;
	EXPECT_NE(sse_client_ptr, nullptr);
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd3);
	EXPECT_NE(sse_client_ptr->next_client, nullptr);
	sse_client_ptr = (k_ghost_io_sse_clients_list_t *)k_ghost_io_ctx.sse_clients->next_client;
	EXPECT_EQ(sse_client_ptr->sse_client_fd, sse_client_fd1);
	EXPECT_EQ(sse_client_ptr->next_client, nullptr);
	k_ghost_io_remove_sse_client(sse_client_fd1);
	k_ghost_io_remove_sse_client(sse_client_fd3);
}

TEST_F(KGhostIOTest, KGhostIOAddConnectionToEmptyArraySuccess)
{
	int client_fds[10]	  = {0};
	int new_connection_fd = 5;
	k_ghost_io_add_connection(client_fds, new_connection_fd);
	EXPECT_EQ(client_fds[0], new_connection_fd);
	for (int i = 1; i < 10; i++)
	{
		EXPECT_EQ(client_fds[i], 0);
	}
}

TEST_F(KGhostIOTest, KGhostIOAddConnectionToAlmostFullSuccess)
{
	int client_fds[1]	  = {0};
	int new_connection_fd = 5;
	k_ghost_io_add_connection(client_fds, new_connection_fd);
	EXPECT_EQ(client_fds[0], new_connection_fd);
	for (int i = 1; i < 1; i++)
	{
		EXPECT_EQ(client_fds[i], 0);
	}
}

TEST_F(KGhostIOTest, KGhostIOManageRestRequest)
{
	std::string request =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n"
		"{}";
	k_ghost_io_manage_rest_request(5, request.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n"));
}

TEST_F(KGhostIOTest, KGhostIOManageUnknownRequest)
{
	k_ghost_io_manage_unknown_endpoint(5);
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n"));
}

TEST_F(KGhostIOTest, KGhostIORegisterCallbackSuccess)
{
	EXPECT_EQ(
		k_ghost_io_register_interface("test_interface", [](const cJSON *, void *user_data_p) { return 0; }, []() {}, nullptr), K_GHOST_REGISTER_RET_CODE_OK);
	EXPECT_NE(k_ghost_io_ctx.interfaces, nullptr);
	EXPECT_EQ(k_ghost_io_ctx.interfaces->next_cb, nullptr);
	EXPECT_STREQ(k_ghost_io_ctx.interfaces->interface_name, "test_interface");
}

TEST_F(KGhostIOTest, KGhostIORegisterCallbackAlreadyRegistered)
{
	EXPECT_EQ(
		k_ghost_io_register_interface("test_interface", [](const cJSON *, void *user_data_p) { return 0; }, []() {}, nullptr), K_GHOST_REGISTER_RET_CODE_OK);
	EXPECT_EQ(k_ghost_io_register_interface(
				  "test_interface", [](const cJSON *, void *user_data_p) { return 0; }, []() {}, nullptr),
			  K_GHOST_REGISTER_RET_CODE_ALREADY_REGISTERED);
	EXPECT_NE(k_ghost_io_ctx.interfaces, nullptr);
	EXPECT_EQ(k_ghost_io_ctx.interfaces->next_cb, nullptr);
	EXPECT_STREQ(k_ghost_io_ctx.interfaces->interface_name, "test_interface");
}

TEST_F(KGhostIOTest, KGhostIORegisterCallbackNullInterfaceName)
{
	EXPECT_EQ(k_ghost_io_register_interface(nullptr, [](const cJSON *, void *user_data_p) { return 0; }, []() {}, nullptr), K_GHOST_REGISTER_RET_CODE_ERROR);
	EXPECT_EQ(k_ghost_io_ctx.interfaces, nullptr);
}

TEST_F(KGhostIOTest, KGhostIORegisterCallbackNullCallback)
{
	EXPECT_EQ(k_ghost_io_register_interface("test_interface", nullptr, []() {}, nullptr), K_GHOST_REGISTER_RET_CODE_ERROR);
	EXPECT_EQ(k_ghost_io_ctx.interfaces, nullptr);
}

TEST_F(KGhostIOTest, KGhostIORegisterCallbackNullSyncCallback)
{
	EXPECT_EQ(
		k_ghost_io_register_interface("test_interface", [](const cJSON *, void *user_data_p) { return 0; }, nullptr, nullptr), K_GHOST_REGISTER_RET_CODE_OK);
	EXPECT_NE(k_ghost_io_ctx.interfaces, nullptr);
	EXPECT_EQ(k_ghost_io_ctx.interfaces->next_cb, nullptr);
	EXPECT_STREQ(k_ghost_io_ctx.interfaces->interface_name, "test_interface");
}

TEST_F(KGhostIOTest, KGhostIORegisterCallbackMultipleSuccess)
{
	EXPECT_EQ(
		k_ghost_io_register_interface("test_interface1", [](const cJSON *, void *user_data_p) { return 0; }, []() {}, nullptr), K_GHOST_REGISTER_RET_CODE_OK);
	EXPECT_EQ(
		k_ghost_io_register_interface("test_interface2", [](const cJSON *, void *user_data_p) { return 0; }, []() {}, nullptr), K_GHOST_REGISTER_RET_CODE_OK);
	EXPECT_NE(k_ghost_io_ctx.interfaces, nullptr);
	EXPECT_NE(k_ghost_io_ctx.interfaces->next_cb, nullptr);
	EXPECT_EQ(((k_ghost_io_interface_t *)k_ghost_io_ctx.interfaces->next_cb)->next_cb, nullptr);
	EXPECT_STREQ(k_ghost_io_ctx.interfaces->interface_name, "test_interface2");
	EXPECT_STREQ(((k_ghost_io_interface_t *)k_ghost_io_ctx.interfaces->next_cb)->interface_name, "test_interface1");
}

TEST_F(KGhostIOTest, KGhostIOCallRestCBForRegisteredInterface)
{
	std::string request =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n"
		"{\"interface\": \"test_interface\"}";
	static int cbCalled = 0;
	k_ghost_io_register_interface(
		"test_interface",
		[](const cJSON *input, void *user_data_p)
		{
			cbCalled++;
			return 0;
		},
		[]() {}, nullptr);
	k_ghost_io_manage_rest_request(5, request.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"));
	EXPECT_EQ(cbCalled, 1);
}

TEST_F(KGhostIOTest, KGhostIOCallRestCBForRegisteredInterfaceInternalError)
{
	std::string request =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n"
		"{\"interface\": \"test_interface\"}";
	static int cbCalled = 0;
	k_ghost_io_register_interface("test_interface", [](const cJSON *input, void *user_data_p) { return -1; }, []() {}, nullptr);
	k_ghost_io_manage_rest_request(5, request.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n"));
}

TEST_F(KGhostIOTest, KGhostIOCallRestCBForMultipleRegisteredInterface)
{
	std::string request1 =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n"
		"{\"interface\": \"test_interface\"}";
	std::string request2 =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n"
		"{\"interface\": \"test_interface\"}";
	static int cbCalled = 0;
	k_ghost_io_register_interface(
		"test_interface",
		[](const cJSON *input, void *user_data_p)
		{
			cbCalled++;
			return 0;
		},
		[]() {}, nullptr);
	k_ghost_io_register_interface(
		"test_interface2",
		[](const cJSON *input, void *user_data_p)
		{
			cbCalled++;
			return 0;
		},
		[]() {}, nullptr);
	k_ghost_io_manage_rest_request(5, request1.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"));
	EXPECT_EQ(cbCalled, 1);
	k_ghost_io_manage_rest_request(5, request1.c_str());
	EXPECT_EQ(send_fake.call_count, 2);
	EXPECT_EQ(close_fake.call_count, 2);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"));
	EXPECT_EQ(cbCalled, 2);
}

TEST_F(KGhostIOTest, KGhostIOCallRestCBForUnknownInterface)
{
	std::string request =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n"
		"{\"interface\": \"unknown_interface\"}";
	k_ghost_io_manage_rest_request(5, request.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 204 No Content\r\nContent-Length: 0\r\n\r\n"));
}

TEST_F(KGhostIOTest, KGhostIOCallRestCBForInvalidRequest)
{
	std::string request =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n";
	k_ghost_io_manage_rest_request(5, request.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n"));
}

TEST_F(KGhostIOTest, KGhostIOCallRestCBForMissingBodyRequest)
{
	std::string request =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0";
	k_ghost_io_manage_rest_request(5, request.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n"));
}

TEST_F(KGhostIOTest, KGhostIOCallRestCBForNullRequest)
{
	k_ghost_io_manage_rest_request(5, nullptr);
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n"));
}

TEST_F(KGhostIOTest, KGhostIOCallSyncCb)
{
	static int syncCbCalled = 0;
	k_ghost_io_register_interface("test_interface", [](const cJSON *input, void *user_data_p) { return 0; }, []() { syncCbCalled++; }, nullptr);
	EXPECT_EQ(syncCbCalled, 0);
	k_ghost_io_add_sse_client(5);
	EXPECT_EQ(syncCbCalled, 1);
	k_ghost_io_remove_sse_client(5);
}

TEST_F(KGhostIOTest, KGhostIOCallSendEventNoSSEClients)
{
	k_ghost_io_send_event("test");
	EXPECT_EQ(send_fake.call_count, 0);
}

TEST_F(KGhostIOTest, KGhostIOCallSendEventOneSSEClient)
{
	k_ghost_io_add_sse_client(5);
	k_ghost_io_send_event("test");
	EXPECT_EQ(send_fake.call_count, 2);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_EQ(send_fake.arg2_val, strlen("data: test\r\n\r\n"));
	k_ghost_io_remove_sse_client(5);
}

TEST_F(KGhostIOTest, KGhostIOCallSendEventMultipleSSEClients)
{
	k_ghost_io_add_sse_client(5);
	k_ghost_io_add_sse_client(6);
	k_ghost_io_add_sse_client(7);
	k_ghost_io_send_event("test");
	EXPECT_EQ(send_fake.call_count, 6);
	EXPECT_EQ(send_fake.arg0_history[3], 7);
	EXPECT_EQ(send_fake.arg0_history[4], 6);
	EXPECT_EQ(send_fake.arg0_history[5], 5);
	EXPECT_EQ(send_fake.arg2_val, strlen("data: test\r\n\r\n"));
	k_ghost_io_remove_sse_client(5);
	k_ghost_io_remove_sse_client(6);
	k_ghost_io_remove_sse_client(7);
}

TEST_F(KGhostIOTest, KGhostIORegisterCallbackWithUserParam)
{
	struct kGhostUserParamTest_s
	{
		int value;
	};
	kGhostUserParamTest_s user_param = {0};
	std::string			  request =
		"POST /api/system/manage HTTP/1.1\r\n"
		"Host: localhost\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: 0\r\n"
		"\r\n"
		"{\"interface\": \"test_interface\"}";
	static int cbCalled = 0;
	k_ghost_io_register_interface(
		"test_interface",
		[](const cJSON *input, void *user_data_p)
		{
			cbCalled++;
			auto user_param_p = static_cast<kGhostUserParamTest_s *>(user_data_p);
			user_param_p->value++;
			return 0;
		},
		[]() {}, &user_param);
	EXPECT_EQ(user_param.value, 0);
	k_ghost_io_manage_rest_request(5, request.c_str());
	EXPECT_EQ(send_fake.call_count, 1);
	EXPECT_EQ(close_fake.call_count, 1);
	EXPECT_EQ(send_fake.arg0_val, 5);
	EXPECT_STREQ((char *)send_fake.arg1_val, "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
	EXPECT_EQ(send_fake.arg2_val, strlen("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n"));
	EXPECT_EQ(cbCalled, 1);
	EXPECT_EQ(user_param.value, 1);
}