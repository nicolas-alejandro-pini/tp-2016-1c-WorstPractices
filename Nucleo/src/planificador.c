/*
 * ready.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */
#include "includes/planificador.h"

int numInQ = 0;										// number of items in the queue
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	// mutual exclusion lock
pthread_mutex_t empty = PTHREAD_MUTEX_INITIALIZER;	// synchronization lock

void *ready_productor(void* arg) {

	stPCB *pcb_to_produce = arg;
	pthread_mutex_lock(&mutex);		// Se lockea el acceso a la cola
	queue_push(colaReady, pcb_to_produce);
	numInQ++;
	pthread_mutex_unlock(&mutex);	// Se desbloquea el acceso a la cola
	pthread_mutex_unlock(&empty);	// Comienzo de espera de consumidor
	fflush(stdout);
	return NULL;
}

stPCB *ready_consumidor() {

	stPCB *pcb_aux;
	while (numInQ == 0) pthread_mutex_lock(&empty);
	pthread_mutex_lock(&mutex);		// Se lockea el acceso a la cola
	pcb_aux = queue_pop(colaReady);
	numInQ--;
	pthread_mutex_unlock(&mutex);	// Se desbloquea el acceso a la cola
	return pcb_aux;
}
