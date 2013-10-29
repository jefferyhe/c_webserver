//
//  ringbuffer.c
//  new_webserver
//
//  Created by Jianfei He on 9/13/13.
//  Copyright (c) 2013 Jianfei He. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


/* Ring buffer store pthread_t objects */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    pthread_t   *elems;  /* pointer of elements                   */
} RingBuffer;

void rbInit(RingBuffer *rb, int size) {
    rb->size  = size + 1; /* include empty elem */
    rb->start = 0;
    rb->end   = 0;
    rb->elems = (pthread_t *)calloc(rb->size, sizeof(pthread_t));
}

void rbFree(RingBuffer *rb) {
    free(rb->elems); /* OK if null */ }

int rbIsFull(RingBuffer *rb) {
    return (rb->end + 1) % rb->size == rb->start; }

int rbIsEmpty(RingBuffer *rb) {
    return rb->end == rb->start; }

void rbWrite(RingBuffer *rb, pthread_t *elem) {
    rb->elems[rb->end] = *elem;
    rb->end = (rb->end + 1) % rb->size;
    if (rb->end == rb->start)
        rb->start = (rb->start + 1) % rb->size; /* full, overwrite */
    
}

/* Read oldest element. */
void rbRead(RingBuffer *rb, pthread_t *elem) {
    *elem = rb->elems[rb->start];
    rb->start = (rb->start + 1) % rb->size;
    }

