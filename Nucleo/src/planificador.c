/*
 * ready.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */
#include "includes/planificador.h"

static t_queue* colaReady;
static int numInQ = 0;										// number of items in the queue
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	// mutual exclusion lock
static pthread_mutex_t empty = PTHREAD_MUTEX_INITIALIZER;	// synchronization lock

void inicializar_cola_ready(){
	colaReady = queue_create();
}

void *ready_productor(void* arg) {
	stPCB *pcb_to_produce = arg;
	pthread_mutex_lock(&mutex);		// Se lockea el acceso a la cola
	queue_push(colaReady, pcb_to_produce);
	printf("READY size [%d]\n",queue_size(colaReady));
	numInQ++;
	pthread_mutex_unlock(&mutex);	// Se desbloquea el acceso a la cola
	pthread_mutex_unlock(&empty);	// Comienzo de espera de consumidor
	pcb_destroy(pcb_to_produce);
	fflush(stdout);
	return NULL;
}

stPCB *ready_consumidor() {
	stPCB *pcb_aux;
	while (numInQ == 0) pthread_mutex_lock(&empty);
	pthread_mutex_lock(&mutex);		// Se lockea el acceso a la cola
	pcb_aux = queue_pop(colaReady);
	printf("READY size [%d]\n",queue_size(colaReady));
	numInQ--;
	pthread_mutex_unlock(&mutex);	// Se desbloquea el acceso a la cola
	return pcb_aux;
}
void eliminar_pcb_ready(int pid){
	stPCB *unPCB;
	int i;
	pthread_mutex_lock(&mutex);
	for (i = 0; i < queue_size(colaReady) ; ++i) {
		unPCB = list_get(colaReady->elements,i);
		if(unPCB->pid == pid){
			list_remove(colaReady->elements,i);
		}
	}
	printf("READY size [%d]\n",queue_size(colaReady));
	pthread_mutex_unlock(&mutex);
}

void destruir_cola_ready(){
	list_destroy_and_destroy_elements(colaReady->elements,(void*)pcb_destroy);
}

void destruir_semaforos_ready(){
	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&empty);
}

void destruir_planificador(){
	destruir_cola_ready();
	destruir_semaforos_ready();
}
