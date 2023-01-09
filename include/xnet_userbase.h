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

#include "xnet_base.h"
#include "xnet_utils.h"

typedef struct xnet_user {
    char *username;
    char *password;
    int perm_level;
    xnet_user_t *next;
} xnet_user_t ;

typedef struct xnet_userbase {
    size_t count;
    xnet_user_t *head;
} xnet_userbase_t ;

xnet_userbase_t *xnet_create_userbase(void);

int xnet_create_user(xnet_userbase_t *base, char *user, char *pass, int new_perm);

int xnet_delete_user(xnet_userbase_t *base, char *user);

void xnet_destroy_userbase(xnet_userbase_t *base);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT