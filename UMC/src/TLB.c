/*
 * TLB.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "TLB.h"
#include <stdbool.h>
#include <string.h>

// Funciones privadas
void destruirNodoTLB(void *nodo);
void imprimirNodoTLB(void *nodo);

// Funciones publicas
int crearTLB(uint16_t cantidadRegistros){
	uint16_t i;

	// TLB desactiva
	if(cantidadRegistros == 0){
		TLB = NULL;
		return EXIT_SUCCESS;
	}

	// Creo estructura TLB
	TLB = malloc(sizeof(tTLB));

	// Referencia a la lista
	TLB->lista = list_create();
	// Mutex
	pthread_mutex_init(&TLB->mutex, NULL);

	// Creo los nodos ( la lista referencia no alloca memoria)
	for(i = 0; i < cantidadRegistros; i++){
		stRegistroTLB *nodo = malloc(sizeof(stRegistroTLB));

		// Inicializo el nodo
		nodo->pid = 0;
		nodo->pagina = 0;
		nodo->marco = 0;
		nodo->lastUsed = 0;
		list_add(TLB->lista, nodo);
	}

	return EXIT_SUCCESS;
}

void destruirTLB(){
	list_destroy_and_destroy_elements(TLB->lista, destruirNodoTLB);
	free(TLB);
}

// +Mutex
int cantidadRegistrosTLB(){
	int size;
	pthread_mutex_lock(&TLB->mutex);
	size = list_size(TLB->lista);
	pthread_mutex_unlock(&TLB->mutex);
	return size;
}

int estaActivadaTLB(){
	if(TLB==NULL)
		return ERROR;
	return OK;
}

int buscarEnTLB(uint16_t pid, uint16_t paginaBuscada, uint16_t frame){
	stRegistroTLB *regFind = NULL;
	stRegistroTLB reg;
	reg.pagina = paginaBuscada;
	reg.pid = pid;

	int _is_this_reg(stRegistroTLB *list_nodo){
		if(list_nodo->pid == reg.pid && list_nodo->pagina == reg.pagina)
			return 1;
		return 0;
	}

	// Busco en la TLB atomicamente
	pthread_mutex_lock(&TLB->mutex);
	regFind = list_find(TLB->lista, (void*)_is_this_reg);

	// En caso de encontrarlo
	if(regFind)
	{
		// Aumento su bit de uso
		if(regFind->lastUsed < MAX_LAST_RECENTLY_USED)
			regFind->lastUsed++;

		// Copio direccion fisica
		if(regFind->marco)
			frame = regFind->marco;
	}
	pthread_mutex_unlock(&TLB->mutex);

	if(!regFind)
		return 0;

	return frame;
}

int reemplazarValorTLB(stRegistroTLB registro){
	stRegistroTLB *lastNodeUsed = NULL;
	uint16_t lastUsed = 1;

	void _last_recently_used(stRegistroTLB *list_nodo){
		if(list_nodo->lastUsed > lastUsed){
			lastUsed = list_nodo->lastUsed;
			lastNodeUsed = list_nodo;
		}

		if(list_nodo->lastUsed != 0)
			list_nodo->lastUsed++;
	}

	// Busco en la TLB atomicamente
	pthread_mutex_lock(&TLB->mutex);
	list_iterate(TLB->lista,(void*)_last_recently_used);

	// Reemplazo registro
	if(lastNodeUsed)
	{
		memcpy(lastNodeUsed, &registro, sizeof(stRegistroTLB));
		lastNodeUsed->lastUsed=1;  // El reemplazo cuenta como usado
	}
	pthread_mutex_unlock(&TLB->mutex);

	return 0;
}

void imprimirTLB(){

	printf("\nPid | Pagina | Marco | LastUsed \n");

	pthread_mutex_lock(&TLB->mutex);
	list_iterate(TLB->lista,imprimirNodoTLB);
	pthread_mutex_unlock(&TLB->mutex);
}

void imprimirNodoTLB(void *nodo){
	stRegistroTLB *list_nodo = (stRegistroTLB*) nodo;
	printf("[%d][%d][%d][%d]\n", list_nodo->pid, list_nodo->pagina, list_nodo->marco, list_nodo->lastUsed);
}

void destruirNodoTLB(void *nodo){
	free((stRegistroTLB*)nodo);
}
