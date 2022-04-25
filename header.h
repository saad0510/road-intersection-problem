#ifndef _SAAD_HEADER_H_
#define _SAAD_HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <syscall.h>

/*
 * @brief print error msg and exit process with status 1
 * @param the error message to print
 */
void err_quit(const char *err_msg)
{
    perror(err_msg);
    exit(1);
}

/*
 * @brief creates a new thread of default attributes
 * @param routine poitner to routine function
 * @param argument pointer to argument to pass in routine. NULL if none
 * @returns the thread id
 * @throws exits process if thread creation fails
 */
pthread_t Pthread_create(void *(*routine)(void *), void *argument)
{
    pthread_t thid;
    if (pthread_create(&thid, NULL, routine, (void *)argument))
        err_quit("Pthread_create() failed");
    return thid;
}

/*
 * @brief waits for a thread to complete
 * @param the thread id
 * @returns the returned value by thread. 0 if none
 * @throws exits process if thread joining fails
 */
intptr_t Pthread_join(pthread_t thid)
{
    void *status;
    if (pthread_join(thid, &status))
        err_quit("Pthread_join() failed");
    return (intptr_t)status;
}

/*
 * @brief exits the thread and cleans up resources
 * @param the return value
 */
void Pthread_exit(intptr_t retval)
{
    pthread_exit((void *)retval);
}

#endif