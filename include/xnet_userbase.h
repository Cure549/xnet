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

#define XNET_MAX_USER_COUNT 128
#define XNET_MAX_USERNAME_LEN 32
#define XNET_MAX_PASSWD_LEN 32

int xnet_create_user(xnet_userbase_group_t *base, char *user, char *pass, int new_perm);

int xnet_delete_user(xnet_userbase_group_t *base, char *user);

void xnet_print_userbase(xnet_userbase_group_t *base);

void xnet_destroy_userbase(xnet_userbase_group_t *base);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT