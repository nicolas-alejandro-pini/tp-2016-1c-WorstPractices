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

int buscarEnTLB(uint16_t pid, uint16_t paginaBuscada, uint16_t frame);
int reemplazarValorTLB(stRegistroTLB registro);
int crearTLB(uint16_t cantidadRegistros);
int estaActivadaTLB();


#endif /* TLB_H_ */
