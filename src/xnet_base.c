#include "xnet_base.h"
#include "xnet_utils.h"

/* Contains all necessary functions and structs related to a base xnet server */

/**
 * @brief Static function that's responsible for allocating all necessary memory in a xnet_box_t.
 * 
 * @return xnet_box_t * A pointer to a xnet_box_t.
 */
static xnet_box_t *initialize_xnet_box(void);

/**
 * @brief Static function that's responsible for configuriung an XNet server and to assign required
 * values.
 * 
 * @param xnet A pointer to a xnet_box_t.
 * @return int Returns 0 on success. Returns 1 or greater on failure.
 */
static int xnet_configure(xnet_box_t *xnet);

static int epoll_ctl_add(int epoll_fd, struct epoll_event *an_event, int fd, uint32_t event_list);

static void xnet_default_on_connection_attempt(xnet_box_t *xnet);

static void xnet_default_on_terminate_signal(xnet_box_t *xnet);

static void xnet_default_on_client_send(xnet_box_t *xnet, xnet_active_connection_t *me);

static int xnet_signal_disposition(xnet_box_t *xnet);

xnet_box_t *xnet_create(const char *ip, size_t port, size_t backlog, size_t timeout)
{
    int err = 0;

    /* Null Check */
    if (NULL == ip) {
        err = E_SRV_INVALID_IP;
        g_show_err(err, "! ASSIGNING DEFAULT IP !");
        ip = XNET_IP_DEFAULT;
    }

    /* Is given port within a valid range? */
    if (XNET_PORT_MIN > port) {
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
    xnet_box_t *new_xnet = initialize_xnet_box();
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

    /* Addrinfo no longer needed, regardless of return on 'xnet_configure()'. */
    freeaddrinfo(new_xnet->network->result);

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

int xnet_start(xnet_box_t *xnet)
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

    /* Allocate space for connections. */
    xnet->connections->clients = calloc(xnet->general->max_connections, sizeof(xnet_active_connection_t));
    if (NULL == xnet->connections->clients) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* XNet start sequence */
    printf("[XNet]\nIP: %s\nPort: %ld\n", xnet->general->ip, xnet->general->port);
    xnet->general->is_running = true;

    /* Initialize srand, used for session id generation. */
    srand(time(NULL));

    /* Setup initial state for epoll. */
    xnet->network->epoll_fd = epoll_create1(0);

    /* Create epoll event for XNet's listening socket. */
    struct epoll_event xnet_event = {0};
    epoll_ctl_add(xnet->network->epoll_fd, &xnet_event, xnet->network->xnet_socket, EPOLLIN);
    set_non_blocking(xnet->network->xnet_socket);

    /* Create dispositions for SIGINT and SIGQUIT. */
    xnet_signal_disposition(xnet);

    /* Apply default event functions if not overridden. */
    if (NULL == xnet->general->on_connection_attempt) {
        xnet->general->on_connection_attempt = xnet_default_on_connection_attempt;
    }
    if (NULL == xnet->general->on_terminate_signal) {
        xnet->general->on_terminate_signal = xnet_default_on_terminate_signal;
    }
    if (NULL == xnet->general->on_client_send) {
        xnet->general->on_client_send = xnet_default_on_client_send;
    }

    /* XNET CONNECTION LOOP */
    while (xnet->general->is_running) {

        /* Yield for next event. */
        int event_count = epoll_wait(xnet->network->epoll_fd, xnet->network->ep_events, XNET_EPOLL_MAX_EVENTS, -1);
    
        for (int i = 0; i < event_count; i++) {
            /* If event triggers on listening socket, a connection is being attempted. */
            if (xnet->network->xnet_socket == xnet->network->ep_events[i].data.fd) {
                xnet->general->on_connection_attempt(xnet);

            /* If event triggers on signal fd, SIGINT or SIGQUIT were executed. */
            } else if (xnet->network->signal_fd == xnet->network->ep_events[i].data.fd) {
                xnet->general->on_terminate_signal(xnet);
            
            /* If event triggers on any other fd within the event array, a client is interacting with server. */
            } else {
                xnet_active_connection_t *me = xnet_get_conn_by_socket(xnet, xnet->network->ep_events[i].data.fd);
                /* Pool worker should be calling this. */
                xnet->general->on_client_send(xnet, me);
            }
        }
    }

    return 0;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_start()");
    return err;
}

int xnet_shutdown(xnet_box_t *xnet)
{
    int err = 0;

    /* Null check */
    if (NULL == xnet) {
        err = E_GEN_NULL_PTR;
        goto handle_err;
    }

    printf("Attempting to shutdown...\n");
    xnet->general->is_running = false;

    return 0;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_shutdown()");
    return err;
}

int xnet_destroy(xnet_box_t *xnet)
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
    nfree((void **)&xnet->connections->clients);
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

static xnet_box_t *initialize_xnet_box(void)
{
    int err = 0;

    /* Allocate space for every group. */
    /* ----------XNET CORE---------- */
    xnet_box_t *new_xnet = calloc(1, sizeof(xnet_box_t));
    if (NULL == new_xnet) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET GENERAL---------- */
    new_xnet->general = calloc(1, sizeof(xnet_general_group_t));
    if (NULL == new_xnet->general) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET NETWORK---------- */
    new_xnet->network = calloc(1, sizeof(xnet_network_group_t));
    if (NULL == new_xnet->network) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET THREAD---------- */
    new_xnet->thread = calloc(1, sizeof(xnet_thread_group_t));
    if (NULL == new_xnet->thread) {
        err = E_GEN_FAIL_ALLOC;
        goto handle_err;
    }

    /* ----------XNET CONNECTIONS---------- */
    new_xnet->connections = calloc(1, sizeof(xnet_connection_group_t));
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

static int xnet_configure(xnet_box_t *xnet)
{
    int err = 0;

    /* ----------GENERAL CATEGORY---------- */
    xnet->general->is_running            = false;
    xnet->general->max_connections       = XNET_MAX_CONNECTIONS_DEFAULT;
    xnet->general->on_connection_attempt = NULL;
    xnet->general->on_terminate_signal   = NULL;
    xnet->general->on_client_send        = NULL;

    /* ----------NETWORK CATEGORY---------- */
    /* Need the string representation of port for getaddrinfo()
     * 24 bytes is an arbitrarily chosen value for the 'stringified' port to fall
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

static int xnet_signal_disposition(xnet_box_t *xnet)
{
    int err = 0;

    err = sigemptyset(&xnet->network->mask);
    if (-1 == err) {
        err = E_GEN_NON_ZERO;
        goto handle_err;
    }

    err = sigaddset(&xnet->network->mask, SIGINT);
    if (-1 == err) {
        err = E_GEN_NON_ZERO;
        goto handle_err;
    }

    err = sigaddset(&xnet->network->mask, SIGQUIT);
    if (-1 == err) {
        err = E_GEN_NON_ZERO;
        goto handle_err;
    }

    /* Modify signal's default dispositions */
    err = sigprocmask(SIG_BLOCK, &xnet->network->mask, NULL);
    if (-1 == err) {
        err = E_GEN_NON_ZERO;
        goto handle_err;
    }

    xnet->network->signal_fd = signalfd(-1, &xnet->network->mask, 0);
    if (-1 == xnet->network->signal_fd) {
        err = E_GEN_NON_ZERO;
        goto handle_err;
    }

    err = epoll_ctl_add(xnet->network->epoll_fd, &xnet->network->sfd_event, xnet->network->signal_fd, EPOLLIN);
    if (-1 == err) {
        err = E_GEN_NON_ZERO;
        goto handle_err;
    }

    return 0;

/* Unreachable unless error is triggered. */
handle_err:
    g_show_err(err, "xnet_signal_disposition()");
    return err;
}

static void xnet_default_on_connection_attempt(xnet_box_t *xnet)
{
    /* Don't accept connections, if client count is maxxed. */
    /* Needs to be refactored. */
    if (xnet->general->max_connections <= xnet->connections->connection_count) {
        /* Retry */
        return;
    }
    
    /* Attempt to accept connection. */
    puts("Connection attempt being made...");
    int client_socket = accept(xnet->network->xnet_socket, NULL, NULL);
    if (-1 == client_socket) {
        fprintf(stderr, "Failed to accept client connection.\n");
        return;
    }
    
    /* Create XNet connection for client. */
    xnet_active_connection_t *new_client = xnet_create_connection(xnet, client_socket);
    if (NULL == new_client) {
        fprintf(stderr, "Failed to create connection data. Dropping connection.\n");
        close(client_socket);
        return;
    }

    /* Add client socket fd to epoll's event list. */
    int event_status = epoll_ctl_add(xnet->network->epoll_fd, &new_client->client_event, client_socket, EPOLLIN);
    if (-1 == event_status) {
        fprintf(stderr, "Failed to add socket fd to epoll event. Dropping connection.\n");
        close(client_socket);
        return;
    }

    xnet_debug_connections(xnet);
}

static void xnet_default_on_terminate_signal(xnet_box_t *xnet)
{
    ssize_t bread = read(xnet->network->signal_fd, &xnet->network->fdsi, sizeof(struct signalfd_siginfo));
    printf("%ld\n", bread);

    if (SIGINT == xnet->network->fdsi.ssi_signo || SIGQUIT == xnet->network->fdsi.ssi_signo) {
        int try_shut = xnet_shutdown(xnet);
        if (0 != try_shut) {
            fprintf(stderr, "Failed to shutdown.\n");
        }
    }
}

static void xnet_default_on_client_send(xnet_box_t *xnet, xnet_active_connection_t *me)
{
    short current_op = xnet_get_opcode(xnet, me->client_event.data.fd);
    /* Check EOF */
    if (0 == current_op) {
        return;
    }
    char packet_trash[XNET_MAX_PACKET_BUF_SZ] = {0};
    ssize_t bytes_read = 0;

    struct __attribute__((__packed__)) test_make_dir
    {
        unsigned short length : 16;
        char msg[2048];
    };

    switch(current_op) // -- Obtained from first read call
    {
        case FTP_CREATE_FILE:
            printf("Doing FTP Create File operation.\n");
            // Read into ftp_create_file struct
            break;
        case FTP_MAKE_DIR:
            printf("Doing FTP Make Dir operation.\n");
            // Read into ftp_make_dir struct
            struct test_make_dir mdir = {0};
            bytes_read = read(me->client_event.data.fd, &mdir, sizeof(struct test_make_dir));
            printf("%d (%s)\n", ntohs(mdir.length), mdir.msg);
            break;
        default:
            /* Flushes buffer if op code is not recognized. */
            bytes_read = read(me->client_event.data.fd, packet_trash, XNET_MAX_PACKET_BUF_SZ);
            while (0 < bytes_read) {
                puts("oi");
                bytes_read = read(me->client_event.data.fd, packet_trash, XNET_MAX_PACKET_BUF_SZ);
            }
            break;
    }

    return;
}