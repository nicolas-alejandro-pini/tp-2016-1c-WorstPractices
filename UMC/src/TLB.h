/*
 * TLB.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TLB_H_
#define TLB_H_

#include <stdint.h>

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
char *TLB;

int buscarEnTLB(uint16_t paginaBuscada);
int reemplazarValorTLB(uint16_t Pagina, stRegistroTablas registro);

int estaActivadaTLB();


#endif /* TLB_H_ */
