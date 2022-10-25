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

/**
 * @brief Set @param sockfd to non-blocking.
 * Source https://github.com/Menghongli/C-Web-Server/blob/master/epoll-server.c
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

xnet_active_connection_t *xnet_get_conn_by_socket(xnet_box_t *xnet, int socket);

xnet_active_connection_t *xnet_create_connection(xnet_box_t *xnet, int socket);

int xnet_close_connection(xnet_box_t *xnet, xnet_active_connection_t *client);

void xnet_debug_connections(xnet_box_t *xnet);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT