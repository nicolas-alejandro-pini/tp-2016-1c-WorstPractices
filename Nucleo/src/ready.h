/*
 * ready.h
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#ifndef READY_H_
#define READY_H_

#include <commons/pcb.h>
#include </usr/include/semaphore.h>

typedef struct {
    sem_t pcb_full;         /* Numero de sem contador de pcb */
    sem_t pcb_empty;        /* keep track of the number of empty spots */
    sem_t pcb_mutex;        /* enforce mutual exclusion to shared data */
} ready_t;

ready_t shared;

void push_pcb(stPCB *unPCB);
void pop_pcb(stPCB *unPCB);


#endif /* READY_H_ */
