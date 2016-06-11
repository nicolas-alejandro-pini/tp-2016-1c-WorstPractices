/*
 * Memoria.c
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */


#include "Memoria.h"


pthread_mutex_t memoria;

void* leerMemoria(uint16_t marco, uint16_t offset, uint16_t lenght){
	/*TODO leer memoria */
	return NULL;
}
void* escribirMemoria(uint16_t marco, uint16_t offset, uint16_t lenght, unsigned char * buffer){
	/*TODO escribir memoria */
	return NULL;
}
void* inicializarMemoriaDisponible(long tamanio, long cantidad){
	void* r;
	if((r=calloc(cantidad, tamanio))==NULL){
		printf("No hay memoria disponible...");
		exit(-1);
	}
	return r;
}
