/*
 * TLB.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "TLB.h"

int buscarEnTLB(uint16_t pid, uint16_t paginaBuscada, uint16_t frame){
	/* TODO buscarEnTLB */
	return 0;
}
int reemplazarValorTLB(stRegistroTLB registro){
	/* TODO reemplazarValorTLB */
	return 0;
}

int crearTLB(uint16_t cantidadRegistros){
	if(cantidadRegistros == 0){
		TLB = NULL;
		return 0;
	}

	/* TODO crearTLB */
	return 0;
}

int estaActivadaTLB(){
	if(TLB==NULL)
		return ERROR;
	return OK;
}
