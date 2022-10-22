#include "xnet_base.h"

/* 
Contains all necessary functions and structs related to a base xnet server

Uses a overarching struct containing everything related to various attributes

server
    General (is_running, ip, port, backlog, max_clients, on_client_connect)
    Network (server socket, hints, result)
    Thread (Info for all active threads)

    Connections (Tracks all socket fd's for connected clients)
        > ActiveConnection struct
            -client socket
            -userbase pointer
            -session_id

    M_Userbase (Addon for xnet. Adds support for a runtime userbase)
    M_Chat (Addon for xnet. Adds support for chat capabilities.)
    M_FTP (Addon for xnet. Adds support for ftp capabilities.)
*/

/**
 * @brief Static function that's responsible for allocating all necessary memory in a xnet_box.
 * 
 * @return xnet_box* A pointer to a xnet_box.
 */
static xnet_box *initialize_xnet_box(void);

/**
 * @brief Static function that's responsible for configuriung an XNet server and to assign required
 * values.
 * 
 * @param xnet A pointer to a xnet_box.
 * @return int Returns 0 on success. Returns 1 or greater on failure.
 */
static int xnet_configure(xnet_box *xnet);

static int epoll_ctl_add(int epoll_fd, struct epoll_event *an_event, int fd, uint32_t event_list);

xnet_box *xnet_create(const char *ip, size_t port, size_t backlog, size_t timeout)
{
    int err = 0;

    /* Null Check */
    if (NULL == ip) {
        err = E_SRV_INVALID_IP;
        g_show_err(err, "! ASSIGNING DEFAULT IP !");
        ip = XNET_IP_DEFAULT;
    }

    /* Is given port within a valid range? */
    if (XNET_PORT_MIN >= port) {
        err = E_SRV_INVALID_PORT;
        g_show_err(err, "! ASSIGNING DEFAULT PORT !");
        port = XNET_PORT_DEFAULT;
    }

    if (XNET_PORT_MAX < port) {
        err = E_SRV_INVALID_PORT;
        g_show_err(err, "! ASSIGNING DEFAULT PORT !");
        port = XNET_PORT_DEFAULT;
    }

    /* Is backlog valid? */
    if (XNET_BACKLOG_MAX < backlog) {
        err = E_SRV_INVALID_BACKLOG;
        g_show_err(err, "! ASSIGNING DEFAULT BACKLOG !");
        backlog = XNET_BACKLOG_DEFAULT;
    }

    /* Is timeout valid? */
    if (XNET_TIMEOUT_MAX < timeout) {
        err = E_SRV_INVALID_BACKLOG;
        g_show_err(err, "! ASSIGNING DEFAULT TIMEOUT !");
        timeout = XNET_TIMEOUT_DEFAULT;
    }

    /* Allocate space for the new instance of XNet. */
    xnet_box *new_xnet = initialize_xnet_box();
    if (NULL == new_xnet) {
        err = E_SRV_FAIL_CREATE;
        goto handle_err;
    }

    /* Assign general data to XNet. */
    new_xnet->general->ip                   = ip;
    new_xnet->general->port                 = port;
    new_xnet->general->backlog              = backlog;
    new_xnet->general->connection_timeout   = timeout;

    /* Configure XNet to user-specified values. */
    err = xnet_configure(new_xnet);
    if (0 != err) {
        err = E_SRV_FAIL_CREATE;
        xnet_destroy(new_xnet);
        goto handle_err;
    }

    /* Return completed XNet. */
    return new_xnet;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_create()");
    return NULL;
}

int xnet_start(xnet_box *xnet)
{
    int err = 0;
    
    /* Null Check */
    if (NULL == xnet) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Prepare XNet to accept connections */
    err = listen(xnet->network->xnet_socket, xnet->general->backlog);
    if (0 != err) {
        err = E_SRV_FAIL_LISTEN;
        goto handle_err;
    }

    /* XNet start sequence */
    printf("[XNet]\nIP: %s\nPort: %ld\n", xnet->general->ip, xnet->general->port);
    xnet->general->is_running = true;

    /* Addrinfo no longer needed. */
    freeaddrinfo(xnet->network->result);

    /* Setup initial state for epoll. */
    struct epoll_event ep_events[XNET_EPOLL_MAX_EVENTS] = {0};
    int epoll_fd = epoll_create1(0);

    /* Create epoll event for XNet's listening socket. */
    struct epoll_event xnet_event = {0};
    epoll_ctl_add(epoll_fd, &xnet_event, xnet->network->xnet_socket, EPOLLIN);

    /* Create epoll event for XNet's connection handling socket. */
    struct epoll_event xnet_monitor_event = {0};
    struct itimerspec monitor_ts = {0};
    int monitor_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    monitor_ts.it_interval.tv_sec = XNET_MONITOR_TIMEOUT / 1000;
    monitor_ts.it_interval.tv_nsec = (XNET_MONITOR_TIMEOUT % 1000) * 1000000;
    monitor_ts.it_value.tv_sec = XNET_MONITOR_TIMEOUT / 1000;
    monitor_ts.it_value.tv_nsec = (XNET_MONITOR_TIMEOUT % 1000) * 1000000;
    if (0 > timerfd_settime(monitor_fd, 0, &monitor_ts, NULL)) {
        fprintf(stderr, "Connection monitor failed creation.\n");
        close(monitor_fd);
    }

    epoll_ctl_add(epoll_fd, &xnet_monitor_event, monitor_fd, EPOLLIN);

    set_non_blocking(xnet->network->xnet_socket);

    /* XNet connection loop */
    while (xnet->general->is_running) {
        /* Yield for next event. */
        /*  Ideally, XNET_MONITOR_TIMOUT should not be -1, as this disrupts connection monitoring. Having the value
            of -1 set to XNET_MONITOR_TIMOUT tells epoll_wait to yield indefinitely, which does not allow the monitor event
            to be triggered.
        */
        int event_count = epoll_wait(epoll_fd, ep_events, XNET_EPOLL_MAX_EVENTS, XNET_MONITOR_TIMEOUT);
    
        for (int i = 0; i < event_count; i++) {
            /* If event triggers on listening socket, a connection is being attempted. */
            if (xnet->network->xnet_socket == ep_events[i].data.fd) {

                /* Don't accept connections, if client count is maxxed. */
                if (xnet->general->max_connections <= xnet->connections->connection_count) {
                    /* Retry */
                    continue;
                }

                puts("Connection attempt...");
                int client_socket = accept(xnet->network->xnet_socket, NULL, NULL);
                if (-1 == client_socket) {
                    fprintf(stderr, "Failed to accept client connection.\n");
                    continue;
                }
                
                struct epoll_event client_event = {0};
                int event_status = epoll_ctl_add(epoll_fd, &client_event, client_socket, EPOLLIN);
                if (-1 == event_status) {
                    fprintf(stderr, "Failed to add socket fd to epoll event. Dropping connection.\n");
                    close(client_socket);
                    continue;
                }
                xnet->connections->connection_count++;
                xnet->connections->client[0].socket = client_socket;
                set_non_blocking(client_socket);
                
            } else if (ep_events[i].data.fd == monitor_fd) {
                if (0 == xnet->connections->connection_count) {
                    continue;
                }

                /* Check if client connection is still alive. */
                puts("Checking clients");
                char time_buffer[2048] = {0};
                read(ep_events[i].data.fd, time_buffer, sizeof(time_buffer)); // Clear timer buffer
                
                // struct sockaddr_in peeraddr = {0};
                // socklen_t peer_len = sizeof(peeraddr);
                // int alive_res = getsockname(xnet->connections->client[0].socket, &peeraddr, &peer_len);
                // printf("Client socket's ip : %s\n", (char *)inet_ntoa(peeraddr.sin_addr));
                // printf("client socket's port %d\n", ntohs(peeraddr.sin_port));

                const char *keep_alive = "_xnet_keepalive";
                int alive_res = send(xnet->connections->client[0].socket, keep_alive, sizeof(keep_alive), 0);
                printf("Sent %d bytes.\n", alive_res);
                if (-1 == alive_res) {
                    puts("Client is no longer connected. Closing socket.");
                    close(xnet->connections->client[0].socket);
                }

            } else {
                puts("Client sent something.");
                printf("Reading file descriptor '%d' -- ", ep_events[i].data.fd);

                char temp_buffer[51];
                ssize_t bytes_read = read(ep_events[i].data.fd, temp_buffer, sizeof(temp_buffer));
                if (0 == bytes_read) {
                    puts("closing");
                    close(ep_events[i].data.fd);
                }

                printf("%zd bytes read.\n", bytes_read);
                
                temp_buffer[bytes_read] = '\0';
                printf("Read '%s'\n", temp_buffer);

                /* Thread off to do some client work. */
            }
        }
    }

    return 0;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_start()");
    return err;
}

int xnet_shutdown(xnet_box *xnet)
{
    (void)xnet;
    return 0;
}

int xnet_destroy(xnet_box *xnet)
{
    int err = 0;

    /* Null Check */
    if (NULL == xnet) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    /* Free all allocations related to a XNet server. */
    nfree((void **)&xnet->general);
    nfree((void **)&xnet->network);
    nfree((void **)&xnet->thread);
    nfree((void **)&xnet->connections);
    nfree((void **)&xnet);

    return 0;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_destroy()");
    return err;
}

static int epoll_ctl_add(int epoll_fd, struct epoll_event *an_event, int fd, uint32_t event_list)
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

static xnet_box *initialize_xnet_box(void)
{
    int err = 0;

    /* Allocate space for every group. */
    /* ----------XNET CORE---------- */
    xnet_box *new_xnet = calloc(1, sizeof(*new_xnet));
    if (NULL == new_xnet) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET GENERAL---------- */
    new_xnet->general = calloc(1, sizeof(*new_xnet->general));
    if (NULL == new_xnet->general) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET NETWORK---------- */
    new_xnet->network = calloc(1, sizeof(*new_xnet->network));
    if (NULL == new_xnet->network) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET THREAD---------- */
    new_xnet->thread = calloc(1, sizeof(*new_xnet->thread));
    if (NULL == new_xnet->thread) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET CONNECTIONS---------- */
    new_xnet->connections = calloc(1, sizeof(*new_xnet->connections));
    if (NULL == new_xnet->connections) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    return new_xnet;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "initialize_xnet_box()");
    nfree((void **)&new_xnet->thread);
    nfree((void **)&new_xnet->network);
    nfree((void **)&new_xnet->general);
    nfree((void **)&new_xnet);
    return NULL;
}

static int xnet_configure(xnet_box *xnet)
{
    int err = 0;

    /* ----------GENERAL CATEGORY---------- */
    xnet->general->is_running           = false;
    xnet->general->max_connections      = XNET_MAX_CONNECTIONS_DEFAULT;
    xnet->general->on_connect           = NULL;

    /* ----------NETWORK CATEGORY---------- */
    /* Need the string representation of port for getaddrinfo()
     * 24 bytes is an arbitrarily chosen value for the stringified port to fall
     * into. 
    */
    char port_to_string[24];
    snprintf(port_to_string, sizeof(port_to_string), "%ld", xnet->general->port);

    /* Configures hint options for TCP IPv4 server */
    xnet->network->hints.ai_family   = AF_INET;
    xnet->network->hints.ai_socktype = SOCK_STREAM;

    /* Start Basic Setup */
    err = getaddrinfo(xnet->general->ip,
                      port_to_string,
                      &xnet->network->hints,
                      &xnet->network->result);

    if (0 != err) {
        err = E_SRV_BAD_ADDR_INFO;
        goto handle_err;
    }

    /* Create socket */
    xnet->network->xnet_socket = socket(xnet->network->result->ai_family,
                                        xnet->network->result->ai_socktype,
                                        xnet->network->result->ai_protocol);

    if (-1 == xnet->network->xnet_socket) {
        err = E_SRV_FAIL_SOCK;
        goto handle_err;
    }

    /* Set reuseaddr */
    int opts = 1;
    err = setsockopt(xnet->network->xnet_socket,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     &opts,
                     sizeof(opts));

    

    if (0 != err) {
        err = E_SRV_FAIL_SOCK_OPT;
        goto handle_err;
    }

    /* Bind name to socket */
    err = bind(xnet->network->xnet_socket,
               xnet->network->result->ai_addr,
               xnet->network->result->ai_addrlen);

    if (0 != err) {
        err = E_SRV_FAIL_BIND;
        goto handle_err;
    }

    /* ----------THREAD CATEGORY---------- */
    xnet->thread->unused = 1;

    /* ----------CONNECTION CATEGORY---------- */
    xnet->connections->connection_count = 0;

    return 0;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_configure()");
    return err;
}