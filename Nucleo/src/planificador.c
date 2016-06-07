/*
 * ready.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */
#include "planificador.h"

void push_pcb(stPCB *unPCB) {
	sem_wait(&shared.pcb_mutex);
	queue_push(colaReady, &unPCB);
	sem_post(&shared.pcb_mutex);
}

void pop_pcb(stPCB *unPCB) {
	sem_wait(&shared.pcb_mutex);
	unPCB = queue_pop(colaReady);
	sem_post(&shared.pcb_mutex);
}

int size_queue_ready() {
	int cantidad_pcb = 0;
	sem_wait(&shared.pcb_mutex);
	cantidad_pcb = queue_size(colaReady);
	sem_post(&shared.pcb_mutex);

	return cantidad_pcb;

}

void *ready_productor(void* arg) {

	stPCB *pcb_to_produce = arg;

	sem_wait(&shared.pcb_empty);
	push_pcb(pcb_to_produce);
	sem_post(&shared.pcb_full);
	printf("[PID-%d] Ingresa a la cola de Ready...\n", pcb_to_produce->pid);
	fflush(stdout);
	sleep(1);

	return NULL;
}

void ready_consumidor(stPCB *pcb_to_consume) {

	sem_wait(&shared.pcb_full);
	pop_pcb(pcb_to_consume);
	sem_post(&shared.pcb_empty);

}
