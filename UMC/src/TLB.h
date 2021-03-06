/*
 * TLB.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TLB_H_
#define TLB_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/ipctypes.h>
#include <commons/collections/list_mutex.h>
#include <commons/ipctypes.h>

#define MAX_LAST_RECENTLY_USED 60000  // unsigned short int

typedef struct{
	uint16_t pid;
	uint16_t pagina;
	uint16_t marco;
	uint16_t lastUsed;
}stRegistroTLB;

/*
 * TLB -> directo MP
 * TLB miss -> TP -> MP
 * TLB miss -> TP page fault-> Swap -> MP, TP, TLB
 *
 */

/* puntero a la tabla cache TLB */
typedef struct tTLB {
	t_list *lista;
	pthread_mutex_t mutex;
} tTLB;

// global.



uint16_t buscarEnTLB(uint16_t pid, uint16_t paginaBuscada, uint16_t *frame);
void flushTLB(uint16_t pid);
void flushTLB_all();
int reemplazarValorTLB(stRegistroTLB registro);
int crearTLB(uint16_t cantidadRegistros);
void destruirTLB();
int estaActivadaTLB();
int cantidadRegistrosTLB();
void imprimirTLB();
void quitarValorTLB(stRegistroTLB registro);

#endif /* TLB_H_ */
