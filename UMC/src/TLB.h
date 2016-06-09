/*
 * TLB.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TLB_H_
#define TLB_H_

#include <stdint.h>
#include "Tablas.h"

/* puntero a la tabla cache TLB */
char *TLB;

int buscarEnTLB(uint16_t paginaBuscada);
int reemplazarValorTLB(uint16_t Pagina, stRegistroTablas registro);

int estaActivadaTLB();


#endif /* TLB_H_ */
