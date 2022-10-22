#ifndef GERR_H
#define GERR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_ERR_MSG_LENGTH 2048

/*
 * Uses values 1001-2500
*/
enum GERR_General {
    E_GEN_POSITIVE_NUM = 1001,
    E_GEN_NEGATIVE_NUM = 1002,
    E_GEN_OUT_RANGE = 1003,
    E_GEN_NULL_PTR = 1004,
    E_GEN_FAIL_ALLOC = 1005,
    E_GEN_FAIL_STRNDUP = 1006,
    E_GEN_FAIL_STR_LENGTH = 1007,
    E_GEN_NON_ZERO = 1008,
    E_GEN_SINGLE_CHAR = 1009,
    E_GEN_FAIL_MEMSET = 1010,
};

/*
 * Uses values 2501-3500
*/
enum GERR_XNet {
    E_SRV_BAD_ADDR_INFO = 2501,
    E_SRV_FAIL_SOCK = 2502,
    E_SRV_FAIL_SOCK_OPT = 2503,
    E_SRV_FAIL_BIND = 2504,
    E_SRV_FAIL_LISTEN = 2505,
    E_SRV_FAIL_ACCEPT = 2506,
    E_SRV_REACHED_CLT_LIMIT = 2507,
    E_SRV_INVALID_PORT = 2508,
    E_SRV_FAIL_CREATE = 2509,
    E_SRV_INVALID_IP = 2510,
    E_SRV_INVALID_BACKLOG = 2511,
    E_SRV_INVALID_TIMEOUT = 2512,
};

// Perror style support for GErrors.
void g_show_err(int err_val, const char *msg);

#endif
