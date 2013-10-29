//
//  ringbuffer.h
//  new_webserver
//
//  Created by Jianfei He on 9/13/13.
//  Copyright (c) 2013 Jianfei He. All rights reserved.
//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

/* Ring buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    pthread_t   *elems;  /* vector of elements                   */
} RingBuffer;

void rbInit(RingBuffer *rb, int size);
void rbFree(RingBuffer *rb);
int rbIsFull(RingBuffer *rb);
int rbIsEmpty(RingBuffer *rb);
void rbWrite(RingBuffer *rb, pthread_t *elem);
void rbRead(RingBuffer *rb, pthread_t *elem);

#endif
