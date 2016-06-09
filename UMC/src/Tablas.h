/*
 * Tablas.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TABLAS_H_
#define TABLAS_H_

typedef struct{
	uint16_t pid;
	uint16_t pagina;
	uint16_t marco;
}stRegistroTablas;

/*
 * TLB -> directo MP
 * TLB miss -> TP -> MP
 * TLB miss -> TP page fault-> Swap -> MP, TP, TLB
 *
 */

#endif /* TABLAS_H_ */
