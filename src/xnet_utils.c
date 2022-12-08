#include "xnet_utils.h"
#include <fcntl.h>

/* 
Any general utilities associated with aiding in server configuration and customization.

// Informs xnet server which operations from an addon should be allowed/are supported.
xnet_add_op_to_whitelist(server *, operation enum value);

// Informs xnet server of a specific addon to include in its suite of options.
xnet_give_addon(server *, XNET_FTP_ID);

// Generates a random session id, and returns it.
xnet_session_new(void);

// Should be used in client work, tells server when to begin session timeout for a client.
xnet_session_begin();

// Compares a given session id, to a active user's session id.
xnet_session_is_valid(size_t session_id, user)

*/

/**
 * @brief Responsible for generating a random session ID.
 * 
 * @return size_t Random session ID.
 */
static size_t xnet_new_session(void);

int set_non_blocking(int sockfd)
{
	int err = 0;

	/* Attempt to set socket to non blocking. */
	err = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);

	/* If this condition is met, there is something seriously wrong. */
	if (-1 == err) {
		err = E_SRV_FAIL_SOCK_OPT;
		goto handle_err;
	}

	return err;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "set_non_blocking()");
    return -1;
}

void nfree(void **ptr)
{
    /* Avoids double free. */
    if (NULL == *ptr) {
        return;
    }

	free(*ptr);
	*ptr = NULL;
}

xnet_active_connection_t *xnet_get_conn_by_socket(xnet_box_t *xnet, int socket)
{
	int err = 0;

	/* Null Check */
	if (NULL == xnet) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	/* Loop through all active connections and find potential match. */
	xnet_active_connection_t *needle = NULL;
	for (size_t n = 0; n < xnet->general->max_connections; n++) {
		if (socket == xnet->connections->clients[n].socket) {
			needle = &xnet->connections->clients[n];
		}
	}

	return needle;


/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_get_conn_by_socket()");
    return NULL;
}

xnet_active_connection_t *xnet_create_connection(xnet_box_t *xnet, int socket)
{
	int err = 0;

	/* Null Check */
	if (NULL == xnet) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	if (-1 == socket) {
		err = E_SRV_BAD_SOCKET;
		goto handle_err;
	}

	/* Ensure connection count is not exceeded before doing search. */
	if (xnet->general->max_connections <= xnet->connections->connection_count) {
		err = E_SRV_CLIENT_MAX_REACHED;
		goto handle_err;
	}

	/* Start at first client, and step through until one is inactive. */
	xnet_active_connection_t *new_client = NULL;
	for (size_t n = 0; n < xnet->general->max_connections; n++) {
		if (false == xnet->connections->clients[n].active) {
			new_client = &xnet->connections->clients[n];
			break;
		}
	}

	/* Don't proceed with a NULL pointer. */
	if (NULL == new_client) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	int set_result = set_non_blocking(socket);
	if (-1 == set_result) {
		err = E_SRV_BAD_SOCKET;
		goto handle_err;
	}

	new_client->active = true;
	new_client->socket = socket;
	new_client->session_id = xnet_new_session(); // TODO: Make this an actual session.
	xnet->connections->connection_count++;

	return new_client;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_create_connection()");
    return NULL;
}

int xnet_close_connection(xnet_box_t *xnet, xnet_active_connection_t *client)
{
	int err = 0;

	/* Null Check */
	if (NULL == xnet) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	if (NULL == client) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	client->active = false;
	close(client->socket);
	client->session_id = 0;
	xnet->connections->connection_count--;

	return 0;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_close_connection()");
    return err;
}

void xnet_debug_connections(xnet_box_t *xnet)
{
	/* NULL Check */
	if (NULL == xnet) {
		return;
	}

	puts("[XNET CONNECTION DEBUG]");
	printf("Max Connections: %ld\n", xnet->general->max_connections);
	printf("Active Connections: %ld\n", xnet->connections->connection_count);
	for (size_t n = 0; n < xnet->general->max_connections; n++) {
		if (true == xnet->connections->clients[n].active) {
			printf("---------------\n");
			printf("Connection #: %ld\n", n+1);
			printf("Index position: %ld\n", n);
			printf("Active: %d\n", xnet->connections->clients[n].active);
			printf("Socket: %d\n", xnet->connections->clients[n].socket);
			printf("SessionID: %ld\n", xnet->connections->clients[n].session_id);
			printf("---------------\n");
		}
	}

	return;
}

short xnet_get_opcode(xnet_box_t *xnet, int client_fd)
{
	short err = 0;

	ssize_t bytes_read = read(client_fd, &err, sizeof(short));
	
	/* EOF Check for client disconnect. */
	if (0 == bytes_read) {
		xnet_active_connection_t *this_client = xnet_get_conn_by_socket(xnet, client_fd);
		if (NULL == this_client) {
			err = E_GEN_NULL_PTR;
			goto handle_err;
		}

		int close_status = xnet_close_connection(xnet, this_client);
		if (0 != close_status) {
			err = E_GEN_NON_ZERO;
			goto handle_err;
		}

		xnet_debug_connections(xnet);
	}

	/* Network to host byte order. */
	return ntohs(err);

	/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_get_opcode()");
    return err;
}

static size_t xnet_new_session(void)
{
	int new_session = rand() / 100;

	return (size_t)new_session;
}