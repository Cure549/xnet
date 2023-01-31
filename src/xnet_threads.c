#include "xnet_threads.h"

void xnet_create_pool(xnet_box_t *xnet)
{
    pthread_mutex_init(&xnet->thread->queue_mutex, NULL);
    pthread_cond_init(&xnet->thread->queue_condition, NULL);
    xnet->thread->shutdown = false;

    /* Spawn threads */
    for (size_t n = 0; n < XNET_THREAD_COUNT; n++) {
        if (pthread_create(&xnet->thread->threads[n], NULL, &xnet_begin_thread, (void *)xnet) != 0) {
            perror("Failed to create thread.");
        }
    }
}

void *xnet_begin_thread(void *arg)
{
    xnet_box_t *xnet = arg;

    while (true)
    {
        pthread_mutex_lock(&xnet->thread->queue_mutex);
        while (xnet->thread->task_count == 0) {
            pthread_cond_wait(&xnet->thread->queue_condition, &xnet->thread->queue_mutex);

            /* Allow shutdown when NO tasks are in queue. */
            if (xnet->thread->shutdown) {
                pthread_mutex_unlock(&xnet->thread->queue_mutex);
                return NULL;
            }
        }

        /* Allow shutdown when tasks ARE in queue. */
        if (xnet->thread->shutdown) {
            pthread_mutex_unlock(&xnet->thread->queue_mutex);
            return NULL;
        }

        xnet_task_t task = xnet->thread->task_queue[0];
        for (int i = 0; i < xnet->thread->task_count - 1; i++) {
            xnet->thread->task_queue[i] = xnet->thread->task_queue[i + 1];
        }

        xnet->thread->task_count--;
        pthread_mutex_unlock(&xnet->thread->queue_mutex);

        xnet_do_work(&task);

    }
}

void xnet_submit_work(xnet_box_t *xnet, xnet_task_t task)
{
    pthread_mutex_lock(&xnet->thread->queue_mutex);
    xnet->thread->task_queue[xnet->thread->task_count] = task;
    xnet->thread->task_count++;
    pthread_mutex_unlock(&xnet->thread->queue_mutex);
    pthread_cond_signal(&xnet->thread->queue_condition);
}

void xnet_do_work(xnet_task_t *task)
{
    if (NULL == task) {
        return;
    }

    task->me->is_working = true;
    puts("Started Work");
    task->task_function(task->xnet, task->me);
    puts("Finished Work");
    task->me->is_working = false;
}

void xnet_destroy_pool(xnet_box_t *xnet)
{
    if (NULL == xnet) {
        return;
    }

    xnet->thread->shutdown = true;
    pthread_cond_broadcast(&xnet->thread->queue_condition);

    /* Join threads */
    for (size_t n = 0; n < XNET_THREAD_COUNT; n++) {
        if (pthread_join(xnet->thread->threads[n], NULL) != 0) {
            perror("Failed to join the thread");
        }
    }

    pthread_mutex_destroy(&xnet->thread->queue_mutex);
    pthread_cond_destroy(&xnet->thread->queue_condition);
}