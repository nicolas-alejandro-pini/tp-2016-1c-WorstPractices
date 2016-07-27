/*
 * ready.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */
#include "includes/planificador.h"

t_queue* colaReady;
int numInQ = 0;										// number of items in the queue
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;	// mutual exclusion lock
pthread_mutex_t empty = PTHREAD_MUTEX_INITIALIZER;	// synchronization lock

void inicializar_cola_ready(){
	colaReady = queue_create();
}

void *ready_productor(void* arg) {
	stPCB *pcb_to_produce = arg;

	pthread_mutex_lock(&mutex);		// Se lockea el acceso a la cola
	queue_push(colaReady, pcb_to_produce);
	log_info("Ingresa el PCB [PID - %d] a la cola de READY, Cantidad de procesos que quedan en la cola de READY [%d]",pcb_to_produce->pid,queue_size(colaReady));
	log_info(imprimir_cola());
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
	log_info("Se va a procesar el PCB [PID - %d], Cantidad de procesos que quedan en la cola de READY [%d]",pcb_aux->pid,queue_size(colaReady));
	log_info(imprimir_cola());
	numInQ--;
	pthread_mutex_unlock(&mutex);	// Se desbloquea el acceso a la cola

	return pcb_aux;
}
void eliminar_pcb_ready(int pid){
	stPCB *unPCB = NULL;
	int i;
	pthread_mutex_lock(&mutex);
	for (i = 0; i < queue_size(colaReady) ; ++i) {
		unPCB = list_get(colaReady->elements,i);
		if(unPCB->pid == pid){
			list_remove(colaReady->elements,i);
			break;
		}
	}
	if(unPCB != NULL)
		log_info("Se elimina el PCB [PID - %d], Cantidad de procesos que quedan en la cola de READY [%d]",unPCB->pid,queue_size(colaReady));
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

char *imprimir_cola(){
	int i;
	stPCB *pcb;
	char *string = string_new();
	string_append(&string, "PIDs en cola READY->[");
	for (i = 0; i < queue_size(colaReady); i++) {
		pcb = list_get(colaReady->elements,i);
		string_append(&string,string_itoa(pcb->pid));
		if((i+1)<queue_size(colaReady)){
			string_append(&string,",");
		}
	}
	string_append(&string, "]");
	return string;
}

