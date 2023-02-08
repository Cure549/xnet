#include "xnet_threads.h"

static void task_decrement_ref_count(xnet_task_t *task);

void xnet_create_pool(xnet_box_t *xnet)
{
    pthread_mutex_init(&xnet->thread->main_lock, NULL);
    pthread_cond_init(&xnet->thread->main_condition, NULL);
    xnet->thread->queue_head = 0;
    xnet->thread->queue_size = 0;
    xnet->thread->shutdown = false;

    /* Spawn threads */
    for (size_t n = 0; n < XNET_THREAD_COUNT; n++) {
        if (pthread_create(&xnet->thread->threads[n], NULL, &xnet_thread_worker, (void *)xnet) != 0) {
            perror("Failed to create thread.");
        }
    }
}

void *xnet_thread_worker(void *arg)
{
    xnet_box_t *xnet = arg;

    while (true) {
        /* Allow shutdown when tasks available. */
        if (xnet->thread->shutdown) {
            return NULL;
        }

        /* Pop task and call it. */
        xnet_task_t *task = xnet_work_pop(xnet);
        if (NULL == task) {
            return NULL;
        }
        task->me->is_working = true;
        task->task_function(task->xnet, task->me);
        task->me->is_working = false;

        /* Reset client's file descriptor. */
        int event_status = epoll_ctl_mod(xnet->network->epoll_fd, &task->me->client_event, task->me->socket, EPOLLIN | EPOLLONESHOT);
        if (-1 == event_status) {
            fprintf(stderr, "Failed to add socket fd to epoll event. Dropping connection.\n");
            close(task->me->socket);
        }

        /* Vulnerable reference decrement. */
        task_decrement_ref_count(task);
    }
    return NULL;
}

void xnet_work_push(xnet_box_t *xnet, xnet_task_t *task)
{
    pthread_mutex_lock(&xnet->thread->main_lock);

    /* If the queue is full, wait. */
    while (XNET_THREAD_MAX_TASKS == xnet->thread->queue_size) {
        pthread_cond_wait(&xnet->thread->main_condition, &xnet->thread->main_lock);

        /* Allow shutdown when worker is idle. */
        if (xnet->thread->shutdown) {
            pthread_mutex_unlock(&xnet->thread->main_lock);
            return;
        }
    }

    /* This is a newly alloc'd task. Update its task count. */
    task->task_count = 0;

    /* Find the next available index, and insert task. Update size appropriately. */
    size_t next_task = (xnet->thread->queue_head + xnet->thread->queue_size) % XNET_THREAD_MAX_TASKS;
    xnet->thread->task_queue[next_task] = task;
    xnet->thread->queue_size++;

    /* Signal condition for newly added task. */
    pthread_cond_signal(&xnet->thread->main_condition);
    pthread_mutex_unlock(&xnet->thread->main_lock);
}

xnet_task_t *xnet_work_pop(xnet_box_t *xnet)
{
    pthread_mutex_lock(&xnet->thread->main_lock);

    /* If the queue is empty, wait. This is where workers halt on start. */
    while (0 == xnet->thread->queue_size) {
        pthread_cond_wait(&xnet->thread->main_condition, &xnet->thread->main_lock);
        /* Allow shutdown when worker is idle. */
        if (xnet->thread->shutdown) {
            pthread_mutex_unlock(&xnet->thread->main_lock);
            return NULL;
        }
    }

    /* Grab the top-most task. */
    xnet_task_t *task = xnet->thread->task_queue[xnet->thread->queue_head];
    task->task_count++;

    /* Inform queue of removed task. */
    size_t next_task = (xnet->thread->queue_head + 1) % XNET_THREAD_MAX_TASKS; 
    xnet->thread->queue_head = next_task;
    xnet->thread->queue_size--;

    /* Signal condition for removed task. */
    pthread_cond_signal(&xnet->thread->main_condition);

    pthread_mutex_unlock(&xnet->thread->main_lock);
    
    return task;
}

void xnet_destroy_pool(xnet_box_t *xnet)
{
    if (NULL == xnet) {
        return;
    }

    xnet->thread->shutdown = true;
    pthread_cond_broadcast(&xnet->thread->main_condition);

    /* Join threads */
    for (size_t n = 0; n < XNET_THREAD_COUNT; n++) {
        if (pthread_join(xnet->thread->threads[n], NULL) != 0) {
            perror("Failed to join the thread");
        }
    }

    pthread_mutex_destroy(&xnet->thread->main_lock);
    pthread_cond_destroy(&xnet->thread->main_condition);
}

static void task_decrement_ref_count(xnet_task_t *task)
{
    pthread_mutex_lock(&task->task_lock);
    task->task_count--;
    if (0 == task->task_count) {
        pthread_mutex_unlock(&task->task_lock);
        pthread_mutex_destroy(&task->task_lock);
        nfree((void **)&task);
        return;
    }
    pthread_mutex_unlock(&task->task_lock);
}