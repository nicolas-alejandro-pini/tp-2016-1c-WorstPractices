/*
 * Memoria.h
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>

void *memoriaPrincipal;

void* inicializarMemoriaDisponible(long tamanio, long cantidad);
void* leerMemoria(uint16_t marco);
void* escribirMemoria(uint16_t marco, unsigned char * buffer);

#endif /* MEMORIA_H_ */
