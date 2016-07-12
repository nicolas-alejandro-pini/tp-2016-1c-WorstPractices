/*
 * Memoria.c
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */


#include "Memoria.h"


pthread_mutex_t memoria;
pthread_mutex_t freeFrames;



void leerMemoria(void **buffer, uint16_t frameBuscado, stPosicion* posLogica){
	void *posFisica = NULL;

	// Calculo de la memoria fisica
	posFisica = memoriaPrincipal+((frameBuscado-1)*losParametros.frameSize);

	pthread_mutex_lock(&memoria);
	// sleep(losParametros.delay);
	memcpy(*buffer, posFisica + posLogica->offset, posLogica->size);
	pthread_mutex_unlock(&memoria);

}
void* escribirMemoria(void *posicion, uint16_t size, void* buffer){

	pthread_mutex_lock(&memoria);
	sleep(losParametros.delay);
	memcpy(posicion, buffer, size);
	pthread_mutex_unlock(&memoria);

	return buffer;
}
void* inicializarMemoriaPrincipal(long tamanio, long cantidad){
	void *r;
	uint16_t i,*p;
	if((r=calloc(cantidad, tamanio))==NULL){
		printf("No hay memoria disponible...");
		exit(-1);
	}
	// inicializo la lista de marcos libres
	marcosLibres = queue_create();
	for(i=1;i<cantidad;i++){
		p=malloc(sizeof(uint16_t));
		*p = i;
		queue_push(marcosLibres, p);
	}

	return r;
}
void destruirMemoriaPrincipal(){
	free(memoriaPrincipal);  // Global
	queue_destroy(marcosLibres); // Global
}

uint16_t obtenerMarcoLibre(){
	uint16_t ret, *p;

	if(queue_size(marcosLibres)==0)
		return 0;

	pthread_mutex_lock(&freeFrames);
	p = queue_pop(marcosLibres);
	pthread_mutex_unlock(&freeFrames);

	ret = *p;
	free(p);
	return ret;
}
uint16_t liberarMarco(uint16_t marco){
	uint16_t *data;

	if(marco>losParametros.frames)
			return ERROR;
	data = malloc(sizeof(uint16_t));
	*data = marco;

	pthread_mutex_lock(&freeFrames);
	queue_push(marcosLibres, data);
	pthread_mutex_unlock(&freeFrames);

	return OK;
}
