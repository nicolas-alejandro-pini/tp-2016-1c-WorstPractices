/*
 * semaforos.c
 *
 *  Created on: 26/6/2016
 *      Author: utnso
 */
#include "includes/semaforos.h"

static pthread_mutex_t diccionario_semaforos_mutex = PTHREAD_MUTEX_INITIALIZER;	// mutual exclusion lock
t_dictionary *diccionario_semaforos;


void inicializar_semaforos(){
	diccionario_semaforos = dictionary_create();
}

void crear_semaforo(char *nombre, char* valor) {
	stSemaforo *semaforo = malloc(sizeof(stSemaforo));/*TODO: liberar estos semaforos al final*/
	semaforo->nombre = strdup(nombre);
	semaforo->valor = atoi(valor);
	semaforo->bloqueados = queue_create();
	pthread_mutex_lock(&diccionario_semaforos_mutex);
	dictionary_put(diccionario_semaforos,nombre,semaforo);
	pthread_mutex_unlock(&diccionario_semaforos_mutex);
}

stSemaforo *buscar_semaforo(char *nombre_semaforo){
	pthread_mutex_lock(&diccionario_semaforos_mutex);
	stSemaforo* semaforo = dictionary_get(diccionario_semaforos, nombre_semaforo);
	pthread_mutex_unlock(&diccionario_semaforos_mutex);
	return semaforo;
}

int wait_semaforo(char* nombre_semaforo) {
	stSemaforo* semaforo = buscar_semaforo(nombre_semaforo);
	semaforo->valor--;

	if (semaforo->valor < 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int signal_semaforo(char* nombre_semaforo) {
	stSemaforo *semaforo = buscar_semaforo(nombre_semaforo);
	semaforo->valor++;

	if (semaforo->valor <= 0) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
