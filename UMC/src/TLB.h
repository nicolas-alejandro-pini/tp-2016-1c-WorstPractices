/*
 * TLB.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TLB_H_
#define TLB_H_

#include <stdint.h>
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
t_list_mutex *TLB;

int buscarEnTLB(uint16_t pid, uint16_t paginaBuscada, uint16_t frame);
int reemplazarValorTLB(stRegistroTLB registro);
int crearTLB(t_list_mutex *tlb, uint16_t cantidadRegistros);
void destruirTLB(t_list_mutex *tlb);
int estaActivadaTLB();
int cantidadRegistrosTLB(t_list_mutex *tlb);
void imprimirTLB(t_list_mutex *tlb);

#endif /* TLB_H_ */
