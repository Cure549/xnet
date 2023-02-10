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
#include "xnet_threads.h"
#include "xnet_userbase.h"

#define MAX_USERS_IN_ROOM 5
#define MAX_ROOM_COUNT 5
#define MAX_ROOM_NAME_LEN 32
#define MAX_MESSAGE_LENGTH 256

/* Return Codes */
#define RC_ACTION_SUCCESS 0
#define RC_FAILED_LOGIN 1
#define RC_FAILED_JOIN_ROOM 2
#define RC_FAILED_WHISPER 3
#define RC_FAILED_SHOUT 4

typedef struct chat_data_room {
    const char *name;
    xnet_active_connection_t *users[MAX_USERS_IN_ROOM];
} chat_room_t ;

typedef struct chat_data_main {
    chat_room_t rooms[MAX_ROOM_COUNT];
} chat_main_t ;

struct __attribute__((__packed__)) chat_whisper_tc {
    short return_code;
};

struct __attribute__((__packed__)) chat_whisper_fc {
    int to_username_length;
    char to_username[XNET_MAX_USERNAME_LEN];
    int msg_length;
    char msg[MAX_MESSAGE_LENGTH];
};

struct __attribute__((__packed__)) chat_whisper_tt {
    int from_username_length;
    char from_username[XNET_MAX_USERNAME_LEN];
    int msg_length;
    char msg[MAX_MESSAGE_LENGTH];
};

typedef struct chat_whisper_packet {
    struct chat_whisper_tc to_client;
    struct chat_whisper_fc from_client;
    struct chat_whisper_tt to_target;
} chat_whisper_packet_t ;

struct __attribute__((__packed__)) chat_login_tc {
    short return_code;
};

struct __attribute__((__packed__)) chat_login_fc {
    int username_length;
    char username[XNET_MAX_USERNAME_LEN];
    int password_length;
    char password[XNET_MAX_PASSWD_LEN];
};

typedef struct chat_login_packet {
    struct chat_login_tc to_client;
    struct chat_login_fc from_client;
} chat_login_packet_t ;

/**
 * @brief Responsible for integrating the chat addon into a XNet server.
 *        This must be called in order for the chat addon to be recognized by XNet.
 * 
 * @param xnet A pointer to an XNet server.
 * @return int Returns 0 on success. Returns non-zero on failure.
 */
int xnet_integrate_chat_addon(xnet_box_t *xnet);

int chat_perform_login(xnet_box_t *xnet, xnet_active_connection_t *client);

/**
 * @brief Feature that allows a client to send a message to another client.
 * 
 * @param xnet 
 * @param client 
 * @return int 
 */
int chat_perform_whisper(xnet_box_t *xnet, xnet_active_connection_t *client);

int chat_perform_join_room(xnet_box_t *xnet, xnet_active_connection_t *client);

int chat_perform_debug(xnet_box_t *xnet, xnet_active_connection_t *client);

int chat_create_room(const char *room_name);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT