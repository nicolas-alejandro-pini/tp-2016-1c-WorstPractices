/*
 * shared_vars.c
 *
 *  Created on: 27/6/2016
 *      Author: utnso
 */
#include "includes/shared_vars.h"

pthread_mutex_t mutex_lista_shared_var = PTHREAD_MUTEX_INITIALIZER;
t_list *lista_shared_vars;

void inicializar_lista_shared_var(){
	lista_shared_vars = list_create();
}

void crear_shared_var(char *nombre) {
	stSharedVar *new = malloc(sizeof(stSharedVar));
	new->nombre = strdup(nombre);
	new->valor = 0;
	list_add(lista_shared_vars,new);
}

stSharedVar *quitar_shared_var(char *nombre){
	stSharedVar *unaSharedVar;
	/*Busqueda del semaforo*/
	int _es_la_shared_var(stSharedVar *s) {
		return string_equals_ignore_case(s->nombre, nombre);
	}
	pthread_mutex_lock(&mutex_lista_shared_var);
	unaSharedVar = list_remove_by_condition(lista_shared_vars, (void*) _es_la_shared_var);
	pthread_mutex_unlock(&mutex_lista_shared_var);
	return unaSharedVar;
}

void grabar_shared_var(stSharedVar *unaSharedVar){
	stSharedVar *auxSharedVar = quitar_shared_var(unaSharedVar->nombre);
	auxSharedVar->valor = unaSharedVar->valor;
	pthread_mutex_lock(&mutex_lista_shared_var);
	list_add(lista_shared_vars,auxSharedVar);
	pthread_mutex_unlock(&mutex_lista_shared_var);
}

stSharedVar obtener_shared_var(char *nombre){
	stSharedVar *unaSharedVar;
	/*Busqueda de la variable compartida*/
	int _es_la_shared_var(stSharedVar *s) {
		return string_equals_ignore_case(s->nombre, nombre);
	}
	pthread_mutex_lock(&mutex_lista_shared_var);
	unaSharedVar = list_find(lista_shared_vars, (void*) _es_la_shared_var);
	pthread_mutex_unlock(&mutex_lista_shared_var);
	return *unaSharedVar;
}


