/**
 * @file        xnet_addon_chat.h
 * @author      Kameryn Gaige Knight
 * @brief       Addon library for a XNet server. Adds chat functionality.
 * @version     1.0
 * @date        2022-10-06
 * 
 * @copyright   Copyright (c) 2022 Kameryn Gaige Knight
 * License      MIT
 */
#ifndef XNET_ADDON_CHAT_H
#define XNET_ADDON_CHAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xnet_base.h"
#include "xnet_utils.h"

struct __attribute__((__packed__)) chat_send_msg_tc {
    int length;
    char msg[2048];
};

struct __attribute__((__packed__)) chat_send_msg_fc {
    int length;
    char msg[2048];
};

typedef struct __attribute__((__packed__)) chat_send_msg_root {
    struct chat_send_msg_tc to_client;
    struct chat_send_msg_fc from_client;
} chat_send_msg_root_t ;

/**
 * @brief Responsible for integrating the chat addon into a XNet server.
 *        This must be called in order for the chat addon to be recognized by XNet.
 * 
 * @param xnet A pointer to an XNet server.
 * @return int Returns 0 on success. Returns non-zero on failure.
 */
int xnet_integrate_chat_addon(xnet_box_t *xnet);

/**
 * @brief Feature that supports client-to-client communication.
 * 
 * @param xnet 
 * @param client 
 * @return int 
 */
int chat_perform_send_msg(xnet_box_t *xnet, xnet_active_connection_t *client);

int chat_perform_join_room(xnet_box_t *xnet, xnet_active_connection_t *client);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT