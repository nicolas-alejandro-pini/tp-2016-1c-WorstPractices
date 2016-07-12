/*
 * semaforos.c
 *
 *  Created on: 26/6/2016
 *      Author: utnso
 */
#include "includes/semaforos.h"

pthread_mutex_t mutex_lista_sem = PTHREAD_MUTEX_INITIALIZER;	// mutual exclusion lock
t_list *lista_semaforos;

void inicializar_semaforos(){
	lista_semaforos = list_create();
}

void crear_semaforo(char *nombre, char* valor) {
	stSemaforo *new = malloc(sizeof(stSemaforo));/*TODO: liberar estos semaforos al final*/
	new->nombre = strdup(nombre);
	new->count = atoi(valor);
	pthread_mutex_init(&(new->mutex), 0);
	pthread_cond_init(&(new->cond),0);
	list_add(lista_semaforos,new);
}

stSemaforo *buscar_semaforo(char *nombre ){
	stSemaforo *unSemaforo;
	/*Busqueda del semaforo*/
	int _es_el_semaforo(stSemaforo *s) {
		return string_equals_ignore_case(s->nombre, nombre);
	}
	pthread_mutex_lock(&mutex_lista_sem);
	unSemaforo = list_remove_by_condition(lista_semaforos, (void*) _es_el_semaforo);
	pthread_mutex_unlock(&mutex_lista_sem);

	return unSemaforo;
}

int wait_semaforo(char* nombre_semaforo) {
	int count;
	stSemaforo *unSemaforo = buscar_semaforo(nombre_semaforo);

	if(unSemaforo==NULL){
		/*No se encontro el semaforo buscado*/
		free(unSemaforo);
		return EXIT_FAILURE;
	}

	pthread_mutex_lock(&(unSemaforo->mutex));
	count = unSemaforo->count -1;
	printf("Valor de count [%d]",count);
	if(count<0){
		pthread_cond_wait(&unSemaforo->cond, &unSemaforo->mutex);
	}
	unSemaforo->count = count;
	pthread_mutex_lock(&mutex_lista_sem);
	list_add(lista_semaforos,unSemaforo);
	pthread_mutex_unlock(&mutex_lista_sem);
	pthread_mutex_unlock(&(unSemaforo->mutex));
	return EXIT_SUCCESS;
}

int signal_semaforo(char* nombre_semaforo){
	stSemaforo *unSemaforo = buscar_semaforo(nombre_semaforo);

	if (unSemaforo == NULL) {
		/*No se encontro el semaforo buscado*/
		free(unSemaforo);
		return EXIT_FAILURE;
	}
	pthread_mutex_lock(&unSemaforo->mutex);
	unSemaforo->count++;
	printf("Valor de count [%d]",unSemaforo->count);
	pthread_cond_broadcast(&unSemaforo->cond);
	pthread_mutex_unlock(&unSemaforo->mutex);
	return EXIT_SUCCESS;
}
