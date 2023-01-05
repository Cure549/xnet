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
static int xnet_new_session(xnet_active_connection_t *client);

/**
 * @brief Begins a sessions timeout timer.
 * 
 * @param xnet 
 * @param client 
 * @return int 
 */
static int xnet_begin_session(xnet_box_t *xnet, xnet_active_connection_t *client);

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

xnet_active_connection_t *xnet_get_conn_by_session(xnet_box_t *xnet, int timer_fd)
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
		if (timer_fd == xnet->connections->clients[n].session.timer_fd) {
			needle = &xnet->connections->clients[n];
		}
	}

	return needle;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_get_conn_by_session()");
    return NULL;
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
		if (false == xnet->connections->clients[n].is_active) {
			new_client = &xnet->connections->clients[n];
			break;
		}
	}

	/* Don't proceed with a NULL pointer. */
	if (NULL == new_client) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	/* Set socket to non-blocking. */
	int set_result = set_non_blocking(socket);
	if (-1 == set_result) {
		err = E_SRV_BAD_SOCKET;
		goto handle_err;
	}

	/* Setup session data. */
	xnet_new_session(new_client);

	new_client->session.timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (0 > new_client->session.timer_fd) {
		err = E_GEN_OUT_RANGE;
		goto handle_err;
	}

	if (0 != clock_gettime(CLOCK_MONOTONIC, &new_client->session.t_data)) {
		err = E_GEN_NON_ZERO;
		goto handle_err;
	}

	size_t interval = xnet->general->connection_timeout;

	new_client->session.t_content.it_value.tv_sec = new_client->session.t_data.tv_sec + interval;
	new_client->session.t_content.it_interval.tv_sec = interval;

	xnet_begin_session(xnet, new_client);

	new_client->is_active = true;
	new_client->socket = socket;
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

	close(client->socket);
	close(client->session.timer_fd);
	memset(&client->session.t_content, 0, sizeof(struct itimerspec));
	memset(&client->session.t_data, 0, sizeof(struct timespec));
	memset(&client->session.session_event, 0, sizeof(struct epoll_event));
	memset(&client->client_event, 0, sizeof(struct epoll_event));
	client->is_active = false;
	client->session.id = 0;
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
		if (true == xnet->connections->clients[n].is_active) {
			printf("---------------\n");
			printf("Connection #: %ld\n", n+1);
			printf("Index position: %ld\n", n);
			printf("Active: %d\n", xnet->connections->clients[n].is_active);
			printf("Socket: %d\n", xnet->connections->clients[n].socket);
			printf("Session: %d (%d)\n", xnet->connections->clients[n].session.id, xnet->connections->clients[n].session.timer_fd);
			printf("---------------\n");
		}
	}

	return;
}

short xnet_get_opcode(xnet_box_t *xnet, int client_fd)
{
	int err = 0;

	/* NULL Check */
	if (NULL == xnet) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	/* Do a partial read on buffer. */
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
	err = 0;
    return err;
}

int epoll_ctl_add(int epoll_fd, struct epoll_event *an_event, int fd, uint32_t event_list)
{
    if (NULL == an_event) {
        fprintf(stderr, "No event given.\n");
        return -1;
    }

    an_event->events = event_list;
    an_event->data.fd = fd;
    int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, an_event);
    return result;
}

int xnet_insert_feature(xnet_box_t *xnet, size_t opcode, int (*new_perform)(xnet_box_t *xnet, xnet_active_connection_t *client))
{
	int err = -1;

	/* NULL Check */
	if (NULL == xnet) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	if (NULL == new_perform) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	/* Is opcode within range? */
	if (XNET_MAX_FEATURES <= opcode) {
		err = E_GEN_OUT_RANGE;
		goto handle_err;
	}

	/* Loop until an available slot is found. */
	for (size_t n = 0; n < XNET_MAX_FEATURES; n++) {
		if (NULL == xnet->general->perform[n]) {
			xnet->general->perform[n] = new_perform;
			err = 0;
		}
	}

	return err;

	/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_insert_feature()");
    return err;
}

int xnet_blacklist_feature(xnet_box_t *xnet, size_t opcode)
{
	int err = 0;

	/* NULL Check */
	if (NULL == xnet) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	/* Is opcode within range? */
	if (XNET_MAX_FEATURES <= opcode) {
		err = E_GEN_OUT_RANGE;
		goto handle_err;
	}

	/* If feature doesn't exist, notify return value. */
	if (NULL == xnet->general->perform[opcode]) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	/* Remove support for feature. */
	xnet->general->perform[opcode] = NULL;
	return 0;


	/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_blacklist_feature()");
    return err;
}

static int xnet_new_session(xnet_active_connection_t *client)
{
	int err = 0;

	if (NULL == client) {
		err = E_GEN_NULL_PTR;
		goto handle_err;
	}

	client->session.id = rand() / 100;
	
	return 0;

	/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_new_session()");
    return err;
}

static int xnet_begin_session(xnet_box_t *xnet, xnet_active_connection_t *client)
{
	int err = 0;

	err = timerfd_settime(client->session.timer_fd, TFD_TIMER_ABSTIME, &client->session.t_content, NULL);
	if (0 != err) {
		err = E_GEN_NON_ZERO;
		goto handle_err;
	}
	epoll_ctl_add(xnet->network->epoll_fd, &client->session.session_event, client->session.timer_fd, EPOLLIN);

	return err;

	/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_begin_session()");
    return err;
}