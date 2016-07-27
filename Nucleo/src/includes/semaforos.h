/*
 * semaforos.h
 *
 *  Created on: 26/6/2016
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include <pthread.h>
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/queue.h>

typedef struct {
	char *nombre;
	int32_t valor;
	pthread_mutex_t mutex_bloqueados;
	t_queue *bloqueados;
} __attribute__((packed)) stSemaforo;

void crear_semaforo(char *nombre, char* valor);
stSemaforo *buscar_semaforo(char *nombre );
void inicializar_semaforos();
int wait_semaforo(stSemaforo *semaforo);
int signal_semaforo(stSemaforo *semaforo);

#endif /* SEMAFOROS_H_ */
