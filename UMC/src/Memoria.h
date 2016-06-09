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

void* leerMemoria(uint16_t marco, uint16_t offset, uint16_t lenght);
void* escribirMemoria(uint16_t marco, uint16_t offset, uint16_t lenght, unsigned char * buffer);

#endif /* MEMORIA_H_ */
