/**
 * @file        xnet_threads.h
 * @author      Kameryn Gaige Knight
 * @brief       Responsible for the creation, destruction, and handling of
 *              an XNet threadpool.
 * @version     1.0
 * @date        2022-10-06
 *
 * @copyright   Copyright (c) 2022 Kameryn Gaige Knight
 * License      MIT
 */
#ifndef XNET_THREADS_H
#define XNET_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "xnet_base.h"
#include "xnet_utils.h"

void xnet_create_pool(xnet_box_t *xnet);

void *xnet_thread_worker(void *arg);

void xnet_work_push(xnet_box_t *xnet, xnet_task_t *task);

xnet_task_t *xnet_work_pop(xnet_box_t *xnet);

void xnet_destroy_pool(xnet_box_t *xnet);

#ifdef __cplusplus
}
#endif

#endif // KAMERYN GAIGE KNIGHT