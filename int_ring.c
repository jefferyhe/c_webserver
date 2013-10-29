//
//  int_ring.c
//  new_webserver
//
//  Created by Jianfei He on 9/14/13.
//  Copyright (c) 2013 Jianfei He. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    int         *elems;  /* point of elements                   */
} Int_ring;

void irInit(Int_ring *ir, int size) {
    ir->size  = size + 1; /* include empty elem */
    ir->start = 0;
    ir->end   = 0;
    ir->elems = (int *)calloc(ir->size, sizeof(int));
}

void irFree(Int_ring *ir) {
    free(ir->elems); /* OK if null */ }

int irIsFull(Int_ring *ir) {
    return (ir->end + 1) % ir->size == ir->start; }

int irIsEmpty(Int_ring *ir) {
    return ir->end == ir->start; }

void irWrite(Int_ring *ir, int *elem) {
    ir->elems[ir->end] = *elem;
    ir->end = (ir->end + 1) % ir->size;
    if (ir->end == ir->start)
        ir->start = (ir->start + 1) % ir->size; /* full, overwrite */    
}

int numOfItems(Int_ring *ir){
    int num;
    num =abs(ir->end - ir->start);
    return num;
}

/* Read oldest element. */
void irRead(Int_ring *ir, int *elem) {
    *elem = ir->elems[ir->start];
    ir->start = (ir->start + 1) % ir->size;
}


