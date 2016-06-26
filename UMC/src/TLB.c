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

uint16_t buscarEnTLB(uint16_t pid, uint16_t paginaBuscada, uint16_t *frame){
	stRegistroTLB *nodoSearch = NULL;
	stRegistroTLB nodoIndex;
	nodoIndex.pagina = paginaBuscada;
	nodoIndex.pid = pid;

	// LRU (Last Recently Used) Search
	void _last_recently_used(stRegistroTLB *list_nodo){
		if(list_nodo->pid == nodoIndex.pid && list_nodo->pagina == nodoIndex.pagina){
			// Copio referencia de memoria
			nodoSearch = list_nodo;
		}
		// sumo a todos los nodos una unidad de tiempo
		if(list_nodo->lastUsed < MAX_LAST_RECENTLY_USED)
			list_nodo->lastUsed++;
	}

	// Busco en la TLB atomicamente
	pthread_mutex_lock(&TLB->mutex);
	list_iterate(TLB->lista,(void*)_last_recently_used);

	// En caso de encontrarlo
	if(nodoSearch)
	{
		// Seteo su lastUsed a tiempo 0
		nodoSearch->lastUsed=0;

		// Copio direccion fisica
		if(nodoSearch->marco)
			*frame = nodoSearch->marco;
	}
	pthread_mutex_unlock(&TLB->mutex);

	// Retorno 0 TLB MISS // frame
	if(!nodoSearch)
		return 0;

	return *frame;
}

int reemplazarValorTLB(stRegistroTLB registro){
	stRegistroTLB *lastNodeUsed = NULL;
	int32_t lastUsed = -1;

	// Implemento LRU (Last Recently Used)
	void _last_recently_used(stRegistroTLB *list_nodo){
		if(lastUsed < list_nodo->lastUsed){
			lastUsed = list_nodo->lastUsed;
			lastNodeUsed = list_nodo;
		}
		// sumo a todos los nodos una unidad de tiempo
		if(list_nodo->lastUsed < MAX_LAST_RECENTLY_USED)
			list_nodo->lastUsed++;
	}

	// Busco en la TLB atomicamente
	pthread_mutex_lock(&TLB->mutex);
	list_iterate(TLB->lista,(void*)_last_recently_used);

	// Reemplazo registro
	if(lastNodeUsed)
	{
		memcpy(lastNodeUsed, &registro, sizeof(stRegistroTLB));
		lastNodeUsed->lastUsed=0;  // Reemplazo inicializa en 0
	}
	pthread_mutex_unlock(&TLB->mutex);

	return 0;
}

void flushTLB(uint16_t pid){

	// Elimino paginas asociadas a pid. Sin eliminar el nodo de la lista
	void _flush_nodo(stRegistroTLB *list_nodo){
		if(pid == list_nodo->pid){
			list_nodo->pid = 0;      //
			list_nodo->pagina = 0;   // necesarios
			list_nodo->lastUsed = 0; //
			list_nodo->marco = 0;
		}
	}
	// Itero lista
	pthread_mutex_lock(&TLB->mutex);
	list_iterate(TLB->lista,(void*)_flush_nodo);
	pthread_mutex_unlock(&TLB->mutex);
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
