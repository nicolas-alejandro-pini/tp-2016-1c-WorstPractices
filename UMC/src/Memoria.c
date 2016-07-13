/*
 * Memoria.c
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */


#include "Memoria.h"


pthread_mutex_t memoria;
pthread_mutex_t freeFrames;



int leerMemoria(void **buffer, uint16_t frameBuscado, stPosicion posLogica){
	void *posFisica = NULL;

	if(!(*buffer))
		return EXIT_FAILURE;

	if((posLogica.offset + posLogica.size) > losParametros.frameSize)
		return EXIT_FAILURE;

	// Calculo de la memoria fisica
	posFisica = memoriaPrincipal+((frameBuscado-1)*losParametros.frameSize);

	pthread_mutex_lock(&memoria);
	// TODO sleep(losParametros.delay);
	memcpy(*buffer, posFisica + posLogica.offset, posLogica.size);
	pthread_mutex_unlock(&memoria);

	return EXIT_SUCCESS;
}

int escribirMemoria(void* buffer, uint16_t frameBuscado, uint16_t offset, uint16_t size){
	void *posFisica = NULL;

	if(offset + size > losParametros.frameSize)
		return EXIT_FAILURE;

	if(!buffer)
		return EXIT_FAILURE;

	if(frameBuscado == 0)
		return EXIT_FAILURE;

	posFisica = memoriaPrincipal+((frameBuscado-1)*losParametros.frameSize) + offset;

	pthread_mutex_lock(&memoria);
	// TODO sleep(losParametros.delay);
	memcpy(posFisica, buffer, size);
	pthread_mutex_unlock(&memoria);

	return EXIT_SUCCESS;
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
