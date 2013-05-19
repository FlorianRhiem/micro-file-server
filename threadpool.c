#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"

typedef struct {
    int id;
    pthread_t pthread;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    volatile int should_exit;
    threadpool_job_t job;
    volatile void *data;
    volatile int has_job;
} thread_t;

struct _threadpool_t {
    thread_t *threads;
    int num_threads;
    int last_ready;
};

static void *thread_function(void *data) {
    thread_t *thread = (thread_t *)data;
    pthread_mutex_lock(&thread->mutex);
    while (1) {
        pthread_cond_wait(&thread->condition, &thread->mutex);
        if (thread->should_exit) {
            break;
        }
        if (thread->has_job) {
            thread->job((void *)thread->data);
            thread->has_job = 0;
        }
    }
    pthread_mutex_unlock(&thread->mutex);
    pthread_exit(NULL);
}


int threadpool_init(threadpool_t *pool_p, int max_num_threads) {
    int i;
    threadpool_t pool;
    int err = 0;
    *pool_p = NULL;
    if (max_num_threads <= 0) {
        return -1;
    }
    pool = malloc(sizeof(struct _threadpool_t));
    if (!pool) {
        return -2;
    }
    pool->threads = malloc(sizeof(thread_t)*max_num_threads);
    if (!pool->threads) {
        free(pool);
        return -3;
    }
    pool->last_ready = -1;
    pool->num_threads = max_num_threads;
    for (i = 0; i < max_num_threads; i++) {
        pool->threads[i].id = i;
        pool->threads[i].should_exit = 0;
        pool->threads[i].has_job = 0;
        pool->threads[i].data = NULL;
    }
    err = 0;
    for (i = 0; i < max_num_threads; i++) {
        err = pthread_mutex_init(&pool->threads[i].mutex, NULL);
        if (err) {
            break;
        }
    }
    if (err) {
        int j;
        for (j = 0; j < i; j++) {
            pthread_mutex_destroy(&pool->threads[j].mutex);
        }
        free(pool->threads);
        free(pool);
        return -4;
    }
    err = 0;
    for (i = 0; i < max_num_threads; i++) {
        err = pthread_cond_init(&pool->threads[i].condition, NULL);
        if (err) {
            break;
        }
    }
    if (err) {
        int j;
        for (j = 0; j < max_num_threads; j++) {
            pthread_mutex_destroy(&pool->threads[j].mutex);
        }
        for (j = 0; j < i; j++) {
            pthread_cond_destroy(&pool->threads[j].condition);
        }
        free(pool->threads);
        free(pool);
        return -5;
    }
    err = 0;
    for (i = 0; i < max_num_threads; i++) {
        err = pthread_create(&pool->threads[i].pthread, NULL, &thread_function, (void *)&pool->threads[i]);
        if (err) {
            break;
        }
    }
    if (err) {
        int j;
        for (j = 0; j < i; j++) {
            void *unused;
            pthread_mutex_lock(&pool->threads[j].mutex);
            pool->threads[j].should_exit = 1;
            pthread_cond_signal(&pool->threads[j].condition);
            pthread_mutex_unlock(&pool->threads[j].mutex);
            pthread_join(pool->threads[j].pthread, &unused);
        }
        for (j = 0; j < max_num_threads; j++) {
            pthread_mutex_destroy(&pool->threads[j].mutex);
            pthread_cond_destroy(&pool->threads[j].condition);
        }
        free(pool->threads);
        free(pool);
        return -6;
    }
    *pool_p = pool;
    return 0;
}

int threadpool_destroy(threadpool_t pool) {
    int i;
    if (!pool) {
        return -1;
    }
    for (i = 0; i < pool->num_threads; i++) {
        void *unused;
        pthread_mutex_lock(&pool->threads[i].mutex);
        pool->threads[i].should_exit = 1;
        pthread_cond_signal(&pool->threads[i].condition);
        pthread_mutex_unlock(&pool->threads[i].mutex);
        pthread_join(pool->threads[i].pthread, &unused);
    }
    for (i = 0; i < pool->num_threads; i++) {
        pthread_mutex_destroy(&pool->threads[i].mutex);
        pthread_cond_destroy(&pool->threads[i].condition);
    }
    free(pool->threads);
    free(pool);
    return 0;
}

int threadpool_handle_job(threadpool_t pool, threadpool_job_t job, void *data) {
    int i;
    if (!pool) {
        return -1;
    }
    for (i = 0; i < pool->num_threads; i++) {
        int j = (i+pool->last_ready+1)%pool->num_threads;
        if (!pthread_mutex_trylock(&pool->threads[j].mutex)) {
            if (pool->threads[j].has_job) {
                pthread_mutex_unlock(&pool->threads[j].mutex);
                continue;
            }
            pool->threads[j].has_job = 1;
            pool->last_ready = j;
            pool->threads[j].job = job;
            pool->threads[j].data = data;
            pthread_cond_signal(&pool->threads[j].condition);
            pthread_mutex_unlock(&pool->threads[j].mutex);
            return 0;
        }
    }
    return -2;
}

