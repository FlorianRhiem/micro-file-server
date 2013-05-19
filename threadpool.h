/**
 * A simple thread pool API
 * @file threadpool.h
 * @author Florian Rhiem (florian.rhiem@gmail.com)
 *
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H

/**
 * Instances of this type act as an opaque handle representing a thread pool.
 *
 * @note The contents of struct _threadpool_t should not be accessed by the user and instances of this type should only be used by being passed to the \c threadpool API.
 */
typedef struct _threadpool_t *threadpool_t;

/**
 * Function pointers of this type can be passed to threadpool_handle_job().
 *
 * @param[in] data
 *  User data as passed to threadpool_handle_job().
 * @return
 *  Function pointers of this type cannot return anything, as there would be no way to retrieve the return value.
 * @note If data was dynamically allocated, it will not be freed by the threadpool, but has to be freed by the job.
 */
typedef void (*threadpool_job_t)(void *data);

/**
 * Initialize the thread pool.
 *
 * @param[in,out] pool_p
 *  Pointer to the thread pool that will be created and initialized. It will be \c NULL if this function fails.
 * @param[in] max_num_threads
 *  The number of threads that this pool is going to manage. Only \c max_num_threads jobs can be handled at the same time.
 * @return
 *  On success, threadpool_init() returns 0; on error, it returns an error number and \c *pool_p is \c NULL.
 */
int threadpool_init(threadpool_t *pool_p, int max_num_threads);

/**
 * Let one of the threads in the thread pool handle a job.
 *
 * @param[in] pool
 *  The thread pool.
 * @param[in] job
 *  The job which needs to be handled. It will only be executed on success.
 * @param[in] data
 *  A pointer of user data that will be passed as argument to the job.
 * @return
 *  On success, threadpool_handle_job() returns 0; on error, it returns an error number.
 * @note If \c pool is \c NULL, threadpool_handle_job() will fail, but it is completely safe.
 * @note This function will not block if all threads in this thread pool are busy; it will return an error number instead.
 */
int threadpool_handle_job(threadpool_t pool, threadpool_job_t job, void *data);

/**
 * Destroy the thread pool.
 *
 * @param[in] pool
 *  Pointer to the thread pool that should be destroyed.
 * @return
 *  On success, threadpool_destroy() returns 0; on error, it returns an error number and the state of \c pool is undefined.
 * @note Passing \c NULL to this function will fail, but is completely safe.
 * @note If there are still jobs being handled, threadpool_destory() will block until they are done.
 */
int threadpool_destroy(threadpool_t pool);

#endif
