/*
 * shared_vars.c
 *
 *  Created on: 27/6/2016
 *      Author: utnso
 */
#include "includes/shared_vars.h"

pthread_mutex_t mutex_lista_shared_var = PTHREAD_MUTEX_INITIALIZER;

stSharedVar *crear_shared_var(char *nombre) {
	stSharedVar *new = malloc(sizeof(stSharedVar));/*TODO: liberar estas sharedVar al final*/
	new->nombre = strdup(nombre);
	new->valor = 0;
	return new;
}

stSharedVar *buscar_shared_var(t_list *lista_shared_vars,char *nombre){
	stSharedVar *unaSharedVar;
	unaSharedVar = malloc(sizeof(stSharedVar));
	/*Busqueda del semaforo*/
	int _es_la_shared_var(stSharedVar *s) {
		return string_equals_ignore_case(s->nombre, nombre);
	}
	pthread_mutex_lock(&mutex_lista_shared_var);
	unaSharedVar = list_remove_by_condition(lista_shared_vars, (void*) _es_la_shared_var);
	pthread_mutex_unlock(&mutex_lista_shared_var);
	return unaSharedVar;
}

void grabar_shared_var(t_list *lista_shared_vars,char *nombre,int *valor){
	stSharedVar *unaSharedVar = buscar_shared_var(lista_shared_vars,nombre);
	unaSharedVar->valor = valor;
	pthread_mutex_lock(&mutex_lista_shared_var);
	list_add(lista_shared_vars,unaSharedVar);
	pthread_mutex_unlock(&mutex_lista_shared_var);
	free(unaSharedVar);
}

stSharedVar *obtener_shared_var(t_list *lista_shared_vars,char *nombre){
	stSharedVar *unaSharedVar;
	unaSharedVar = (stSharedVar*)malloc(sizeof(stSharedVar));
	/*Busqueda del semaforo*/
	int _es_la_shared_var(stSharedVar *s) {
		return string_equals_ignore_case(s->nombre, nombre);
	}
	pthread_mutex_lock(&mutex_lista_shared_var);
	unaSharedVar = list_find(lista_shared_vars, (void*) _es_la_shared_var);
	pthread_mutex_unlock(&mutex_lista_shared_var);
	return unaSharedVar;
}


