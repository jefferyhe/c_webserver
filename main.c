/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Jianfei He, jeffery@gwu.edu, 2013; Gabriel Parmer, gparmer@gwu.edu, 2012
 */
/* 
 * This is a HTTP server.  It accepts connections on port 8080, and
 * serves a local static document.
 *
 * The clients you can use are 
 * - httperf (e.g., httperf --port=8080),
 * - wget (e.g. wget localhost:8080 /), 
 * - or even your browser.  
 *
 * To measure the efficiency and concurrency of your server, use
 * httperf and explore its options using the manual pages (man
 * httperf) to see the maximum number of connections per second you
 * can maintain over, for example, a 10 second period.
 *
 * Example usage:
 * # make test1
 * # make test2
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>

#include <util.h> 		/* client_process */
#include "server.h"		/* server_accept and server_create */
#include "cas.h"
#include "ringbuffer.h"
#include "int_ring.h"

#define MAX_DATA_SZ 1024
#define MAX_CONCURRENCY 4

RingBuffer rb; /* initialize a ringbuffer for mode 1 (storing pthread_t) */
Int_ring ir;   /* initialize a ringbuffer for mode 2 (storing file descriptor) */

int client_fd;/* fd for client to process*/

void* client_process_thread();/*mode1 worker thread function*/
void* client_process_thread2();/*mode2 worker thread function*/

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buff_not_full_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t buff_not_empty_cond = PTHREAD_COND_INITIALIZER;

/* 
 * This is the function for handling a _single_ request.  Understand
 * what each of the steps in this function do, so that you can handle
 * _multiple_ requests.  Use this function as an _example_ of the
 * basic functionality.  As you increase the server in functionality,
 * you will want to probably keep all of the functions called in this
 * function, but define different code to use them.
 */
void
server_single_request(int accept_fd)
{
	int fd;
    
	/* 
	 * The server thread will always want to be doing the accept.
	 * That main thread will want to hand off the new fd to the
	 * new threads/processes/thread pool.
	 */ 
        fd = server_accept(accept_fd);
        client_process(fd);
	

	/* 
	 * A loop around these two lines will result in multiple
	 * documents being served.
	 */

	return;
}

/* 
 * The following implementation creates a new thread per client
 * request using the pthread API, and that thread is removed/killed
 * when the request is completed.
 */
void
server_thread_per_req(int accept_fd)
{
	    while (1)
        {
            client_fd = server_accept(accept_fd);
        
            //create client thread for each request
            int res_client;
            pthread_t client_thread;
        
            res_client = pthread_create(&client_thread, NULL, client_process_thread, NULL);
            if (res_client != 0)
            {
                printf("Main Thread creation failed\n");
                exit(EXIT_FAILURE);
            }        
        
            if (!rbIsFull(&rb))
            {
                rbWrite(&rb, &client_thread);            
            }
            else
            {
                /*printf("buffer is full!!!");*/
                rbRead(&rb, &client_thread);
                res_client = pthread_join(client_thread, NULL);
            }
        }
    rbFree(&rb);
    return;
}

/*mode1 worker thread function*/
void*
client_process_thread()
{
    int fd = client_fd;
    client_process(fd);
    return NULL;
}

/* 
 * The following implementations use a thread pool.  This collection
 * of threads is of maximum size MAX_CONCURRENCY, and is created by
 * pthread_create.  These threads retrieve data from a shared
 * data-structure with the main thread.  The synchronization around
 * this shared data-structure is either done using mutexes + condition
 * variables (for a bounded structure), or compare and swap (__cas in
 * cas.h) to do lock-free synchronization on a stack or ring buffer.
 */

void
server_thread_pool_bounded(int accept_fd)
{

    int count_thread = 0;
    
    //create client threads under MAX_CONCURRENCY
    int res_client;
    pthread_t client_thread[MAX_CONCURRENCY];
   
    //create 4 worker threads
    while (count_thread<MAX_CONCURRENCY)
    {
        res_client = pthread_create(&client_thread[count_thread], NULL, client_process_thread2, NULL);
        if (res_client != 0)
        {
            perror("WORKER Thread creation failed");
            exit(EXIT_FAILURE);
        }
        count_thread++;
    }
    
    while (1)
    {
      
      //  printf("This is mode2!!!!!!\n");
        int status=pthread_mutex_lock(&mutex);
        if(status!=0)
        {
            perror("Create MASTER mutex_lock failed!!");
            exit(EXIT_FAILURE);
        }
        
        if (irIsFull(&ir))
        {
          //  printf("ir is full, waiting worker's signal.....\n");
            status = pthread_cond_wait(&buff_not_full_cond, &mutex);
            if(status!=0)
            {
                perror("buff_not_full_cond error!!");
                exit(EXIT_FAILURE);
            }
        }
        
        
        
        status=pthread_mutex_unlock(&mutex);
        if(status!=0)
        {
            perror("Create MASTER mutex_unlock failed!!");
            exit(EXIT_FAILURE);
        }
                
        client_fd = server_accept(accept_fd);
                
        irWrite(&ir, &client_fd); /*write to the buffer*/
        
        if (!irIsEmpty(&ir))
        {
            
//         printf("Signal WORKER!!\n");
            status=pthread_cond_signal(&buff_not_empty_cond);
            if(status!=0)
            {
                perror("MASTER buff not empty cond error!");
                exit(EXIT_FAILURE);
            }
        }
        
    }
    
	return;
}

/*mode2 worker thread function*/
void *client_process_thread2()
{
    while (1)
    {        
        int status=pthread_mutex_lock(&mutex);
        if(status!=0)
        {
            perror("Create WORKER mutex_lock failed!!");
            exit(EXIT_FAILURE);
        }
    
        int new_fd = client_fd;
    
        if (irIsEmpty(&ir))
        {
    //     printf("ir is empty, waiting master's signal.....\n");
            status = pthread_cond_wait(&buff_not_empty_cond, &mutex);
            if(status!=0)
            {
                perror("buff_not_empty_cond error!!");
                exit(EXIT_FAILURE);
            }
        }
    
        status=pthread_mutex_unlock(&mutex);
        if(status!=0)
        {
            perror("Create WORKER mutex_unlock failed!!");
            exit(EXIT_FAILURE);
        }
        
    irRead(&ir, &new_fd); /* read a fd from the buffer */

    client_process(new_fd);
        
        if (!irIsFull(&ir))
        {            
//          printf("Signal MASTER!!\n");
            status=pthread_cond_signal(&buff_not_full_cond);
            if(status!=0)
            {
                perror("MASTER buff not empty cond error!");
                exit(EXIT_FAILURE);
            }
        }

    }
    
    return NULL;
}


typedef enum {
	SERVER_TYPE_ONE = 0,
	SERVER_TYPE_THREAD_PER_REQUEST,
	SERVER_TYPE_THREAD_POOL_BOUND,
} server_type_t;

int
main(int argc, char *argv[])
{
	server_type_t server_type;
	short int port;
	int accept_fd;
    
    rbInit(&rb, MAX_CONCURRENCY);  /*Initialize the ringbuffer */
    irInit(&ir, MAX_CONCURRENCY);  /*Initialize the int_ringbuffer */

	if (argc != 3) {
		printf("Proper usage of http server is:\n%s <port> <#>\n"
		       "port is the port to serve on, # is either\n"
		       "0: serve only a single request\n"
		       "1: serve each request with a new thread\n"
		       "2: use a thread pool and a _bounded_ buffer with "
		       "mutexes + condition variables\n"
		       ,argv[0]);
		return -1;
	}

	port = atoi(argv[1]);
	accept_fd = server_create(port);
	if (accept_fd < 0) return -1;
	
	server_type = atoi(argv[2]);

	switch(server_type) {
	case SERVER_TYPE_ONE:
		server_single_request(accept_fd);
		break;
	case SERVER_TYPE_THREAD_PER_REQUEST:
		server_thread_per_req(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL_BOUND:
        server_thread_pool_bounded(accept_fd);
		break;
	}
	close(accept_fd);

	return 0;
}
