#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg, ...)
// #define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg, ...) printf("threading ERROR: " msg "\n", ##__VA_ARGS__)

void *threadfunc(void *thread_param)
{
    struct thread_data *thread_func_args = (struct thread_data *)thread_param;
    thread_func_args->thread_complete_success = false;
    // wait, obtain mutex, wait, release mutex as described by thread_data structure
    usleep(thread_func_args->wait_to_obtain_ms * 1000);
    if (!pthread_mutex_lock(thread_func_args->mutex_ptr))
    {
        usleep(thread_func_args->wait_to_release_ms * 1000);
        if (pthread_mutex_unlock(thread_func_args->mutex_ptr))
        {
            ERROR_LOG("couldnot unlock the mutex");
        }
        else
        {
            thread_func_args->thread_complete_success = true;
        }
    }
    else
    {
        ERROR_LOG("couldnot lock the mutex");
    }

    return thread_param;
}

bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data *thread_args = (struct thread_data *)malloc(sizeof(struct thread_data));
    thread_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_args->wait_to_release_ms = wait_to_release_ms;
    thread_args->mutex_ptr = mutex;

    if (!pthread_create(&thread_args->thread, NULL, threadfunc, (void *)thread_args))
    {
        *thread = thread_args->thread;
        return true;
    }
    else
    {
        ERROR_LOG("couldnot create the Thread");
    }

    return false;
}
