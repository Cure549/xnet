/**
 * @file        xnet_userbase.h
 * @author      Kameryn Gaige Knight
 * @brief       Responsible for adding support for user's in XNet.
 * @version     1.0
 * @date        2022-10-06
 * 
 * @copyright   Copyright (c) 2022 Kameryn Gaige Knight
 * License      MIT
 */
#ifndef XNET_USERBASE_H
#define XNET_USERBASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "limits.h"
#include "xnet_base.h"
#include "xnet_utils.h"

#define XNET_MAX_USERNAME_LEN 32
#define XNET_MAX_PASSWD_LEN 32

int xnet_create_user(xnet_userbase_group_t *base, char *user, char *pass, int new_perm);

int xnet_delete_user(xnet_userbase_group_t *base, char *user);

int xnet_login_user(xnet_userbase_group_t *base, char *user, char *pass, xnet_active_connection_t *conn);

int xnet_logout_user(xnet_active_connection_t *conn);

xnet_user_t *xnet_user_exists(xnet_userbase_group_t *base, char *user);

bool xnet_user_is_logged_in(xnet_userbase_group_t *base, char *user);

xnet_active_connection_t *xnet_get_conn_by_user(xnet_box_t *xnet, char *user);

void xnet_print_userbase(xnet_userbase_group_t *base);

void xnet_destroy_userbase(xnet_userbase_group_t *base);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT