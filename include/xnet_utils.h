/**
 * @file        xnet_utils.h
 * @author      Kameryn Gaige Knight
 * @brief       Various utility functions to aid with readability and
 *              consolidation for all things related to XNet.
 * @version     1.0
 * @date        2022-10-06
 * 
 * @copyright   Copyright (c) 2022 Kameryn Gaige Knight
 * License      MIT
 */
#ifndef XNET_UTILS_H
#define XNET_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "xnet_base.h"

typedef enum xnet_opcodes
{
    CORE_LOGIN = 10,
    CORE_DISCONNECT = 11,
    FTP_CREATE_FILE = 100,
    FTP_MAKE_DIR = 101,
    FTP_SEARCH_DIR = 102,
    FTP_GET_FILE = 103,
    FTP_PUT_FILE = 104,
    FTP_DELETE_FILE = 105,
} xnet_opcodes_t;

/**
 * @brief Set @param sockfd to non-blocking.
 * 
 * @param sockfd File descriptor for a socket.
 * @return int -1 on failure. 0 on success.
 */
int set_non_blocking(int sockfd);

/**
 * @brief NULL's the pointer it free's.
 * 
 * @param ptr A pointer to the allocated memory space.
 */
void nfree(void **ptr);

xnet_active_connection_t *xnet_get_conn_by_session(xnet_box_t *xnet, int timer_fd);

xnet_active_connection_t *xnet_get_conn_by_socket(xnet_box_t *xnet, int socket);

xnet_active_connection_t *xnet_create_connection(xnet_box_t *xnet, int socket);

/**
 * @brief Closes a connection gracefully. Handles closing client socket along with their session.
 * 
 * @param xnet 
 * @param client 
 * @return int 
 */
int xnet_close_connection(xnet_box_t *xnet, xnet_active_connection_t *client);

void xnet_debug_connections(xnet_box_t *xnet);

/**
 * @brief Extracts the opcode from a inbound packet.
 * 
 * @return unsigned short 
 */
short xnet_get_opcode(xnet_box_t *xnet, int client_fd);

int epoll_ctl_add(int epoll_fd, struct epoll_event *an_event, int fd, uint32_t event_list);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT