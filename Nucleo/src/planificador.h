/*
 * ready.h
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/pcb.h>
#include <commons/collections/queue.h>
#include </usr/include/semaphore.h>

typedef struct {
    sem_t pcb_full;         /* Numero de sem contador de pcb */
    sem_t pcb_empty;        /* keep track of the number of empty spots */
    sem_t pcb_mutex;        /* enforce mutual exclusion to shared data */
} ready_t;

ready_t shared;
t_list 	*listaBlock; 		/*Lista de todos los PCB bloqueados*/
t_queue *colaReady; 		/*Cola de todos los PCB listos para ejecutar*/
t_queue *colaExit; 			/*Cola de todos los PCB listos para liberar*/

void push_pcb(stPCB *unPCB);
void pop_pcb(stPCB *unPCB);
void *ready_productor(void *arg);


#endif /* PLANIFICADOR_H_ */
