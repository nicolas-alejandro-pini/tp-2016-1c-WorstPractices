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
#include "Parametros.h"
#include "commons/ipctypes.h"
#include <commons/collections/queue.h>
#include <string.h>

void *memoriaPrincipal;
void *marcosLibres;

void* inicializarMemoriaDisponible(long tamanio, long cantidad);
void* leerMemoria(void *posicion, uint16_t size);
void* escribirMemoria(void *posicion, uint16_t size, void* buffer);
uint16_t obtenerMarcoLibre();
uint16_t liberarMarco(uint16_t marco);

#endif /* MEMORIA_H_ */
