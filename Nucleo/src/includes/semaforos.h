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
#include <commons/collections/list.h>

typedef struct {
	char* 			nombre; 	/*Nombre del semaforo*/
	int 			count;
	pthread_mutex_t mutex;
	pthread_cond_t 	cond;
} __attribute__((packed)) stSemaforo;

stSemaforo *crear_semaforo(char *nombre, char* valor);
stSemaforo *buscar_semaforo(t_list *lista_semaforos,char *nombre );
int wait_semaforo(t_list *lista_semaforos, char* nombre_semaforo);
int signal_semaforo(t_list *lista_semaforos, char* nombre_semaforo);


#endif /* SEMAFOROS_H_ */
