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
	int pid;
	int pagina;
	int marco;
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
tTLB *TLB;


int buscarEnTLB(uint16_t pid, uint16_t paginaBuscada, uint16_t **frame);
int reemplazarValorTLB(stRegistroTLB registro);
int crearTLB(uint16_t cantidadRegistros);
void destruirTLB();
int estaActivadaTLB();
int cantidadRegistrosTLB();
void imprimirTLB();

#endif /* TLB_H_ */
