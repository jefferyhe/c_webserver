//
//  int_ring.h
//  new_webserver
//
//  Created by Jianfei He on 9/14/13.
//  Copyright (c) 2013 Jianfei He. All rights reserved.
//

#ifndef INT_RING_H
#define INT_RING_H

/* Ring buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    int   *elems;  /* vector of elements                   */
} Int_ring;

void irInit(Int_ring *ir, int size);
void irFree(Int_ring *ir);
int irIsFull(Int_ring *ir);
int irIsEmpty(Int_ring *ir);
void irWrite(Int_ring *ir, int *elem);
void irRead(Int_ring *ir, int *elem);
int numOfItems(Int_ring *ir);


#endif
