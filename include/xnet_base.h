/**
 * @file        xnet_base.h
 * @author      Kameryn Gaige Knight
 * @brief       Base library that supports the creation of a XNet server.
 * @version     1.0
 * @date        2022-10-06
 * 
 * @copyright   Copyright (c) 2022 Kameryn Gaige Knight
 * License      MIT
 */
#ifndef XNET_BASE_H
#define XNET_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "gerr.h"

#define XNET_IP_DEFAULT              "127.0.0.1"
#define XNET_PORT_DEFAULT            40777
#define XNET_PORT_MIN                1030
#define XNET_PORT_MAX                65535

#define XNET_MAX_CONNECTIONS_DEFAULT 5

#define XNET_BACKLOG_MAX             128
#define XNET_BACKLOG_DEFAULT         128

#define XNET_TIMEOUT_DEFAULT         3600
#define XNET_TIMEOUT_MAX             7200

#define XNET_EPOLL_MAX_EVENTS        10

#define XNET_MAX_PACKET_BUF_SZ       8192

typedef struct xnet_box {
    struct xnet_general_group *general;
    struct xnet_network_group *network;
    struct xnet_thread_group *thread;
    struct xnet_connection_group *connections;
    struct xnet_userbase_group *m_userbase;
    struct xnet_addon_chat *m_chat;
    struct xnet_addon_ftp *m_ftp;
} xnet_box_t ; 

typedef struct xnet_user_session {
    int id;
    time_t start;
} xnet_user_session_t ;

typedef struct xnet_active_connection {
    /* State of client, if false, connection object lacks a client. */
    bool active;
    int socket;
    struct epoll_event client_event;
    // Userbase entry here...
    xnet_user_session_t session;
} xnet_active_connection_t ;

typedef struct xnet_general_group {
    bool is_running;
    const char *ip;
    size_t port;
    size_t backlog;
    size_t connection_timeout;
    size_t max_connections;
    void (*on_connection_attempt)(xnet_box_t *xnet);
    void (*on_terminate_signal)(xnet_box_t *xnet);
    void (*on_client_send)(xnet_box_t *xnet, xnet_active_connection_t *me);
} xnet_general_group_t ;

typedef struct xnet_network_group {
    int xnet_socket;
    struct addrinfo hints;
    struct addrinfo *result;
    int epoll_fd;
    struct epoll_event ep_events[XNET_EPOLL_MAX_EVENTS];
    int signal_fd;
    struct epoll_event sfd_event;
    struct signalfd_siginfo fdsi;
    sigset_t mask;
} xnet_network_group_t ;

typedef struct xnet_thread_group {
    int unused; // TODO: Add thread correlation
} xnet_thread_group_t ;

typedef struct xnet_connection_group {
    size_t connection_count;
    xnet_active_connection_t *clients;
} xnet_connection_group_t ;

struct xnet_userbase_group {
    int unused;
};

struct xnet_addon_chat {
    int unused;
};

struct xnet_addon_ftp {
    int unused;
};

/**
 * @brief Creates a new XNet server. If invalid values are given, 
 * XNet will default to the labelled definitions that can be found at the top of xnet_base.h
 * 
 * @param ip String representation of an ip address. Must be IPv4 or the default value will be used.
 * @param port Positive integer representation of a non-privileged port number.
 * @param backlog Number of clients to place in a pending queue if connections aren't being accepted.
 * @param timeout Duration in seconds in which XNet will sustain a connection's session.
 * @return xnet_box_t* NULL on failure. Valid xnet_box_t pointer on success.
 */
xnet_box_t *xnet_create(const char *ip, size_t port, size_t backlog, size_t timeout);

int xnet_start(xnet_box_t *xnet);

/**
 * @brief Drops any active connections. Closes all sockets related to server.
 * Prevents any further connections.
 * 
 * @param xnet A pointer to a xnet_box_t.
 * @return int Returns 0 on success. Returns 1 or greater on failure.
 */
int xnet_shutdown(xnet_box_t *xnet);

/**
 * @brief Releases memory associated with a xnet_box_t.
 * 
 * @param xnet A pointer to a xnet_box_t.
 * @return int Returns 0 on success. Returns 1 or greater on failure.
 */
int xnet_destroy(xnet_box_t *xnet);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT