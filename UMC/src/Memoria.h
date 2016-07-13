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
#include <stdlib.h>
#include <unistd.h>
#include "Parametros.h"
#include "commons/ipctypes.h"
#include <commons/collections/queue.h>
#include <commons/serializador.h>
#include <string.h>

void *memoriaPrincipal;
void *marcosLibres;

void* inicializarMemoriaPrincipal(long tamanio, long cantidad);
void destruirMemoriaPrincipal();
int leerMemoria(void **buffer, uint16_t frameBuscado, stPosicion* posLogica);
int escribirMemoria(void* buffer, uint16_t frameBuscado, uint16_t offset, uint16_t size);


uint16_t obtenerMarcoLibre();
uint16_t liberarMarco(uint16_t marco);

#endif /* MEMORIA_H_ */
