/*
 * TablaMarcos.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TABLAMARCOS_H_
#define TABLAMARCOS_H_

#include <stdint.h>
#include <stdio.h>
#include "Parametros.h"



typedef struct{
	uint16_t pagina;
	uint16_t marco;
	unsigned char bit2ndChance;
	unsigned char bitPresencia;
	unsigned char bitModificado;
}stRegistroTP;

typedef struct{
	uint16_t pid;
	void *tabla;
}stNodoListaTP;


/*
 * TLB -> directo MP
 * TLB miss -> TP -> MP
 * TLB miss -> TP page fault-> Swap -> MP, TP, TLB
 *
 */

/* puntero a la tabla de Marcos */
char *TablaMarcos;

int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t frame);
int reemplazarValorTabla(uint16_t Pagina, stRegistroTP registro);
int crearTabla(uint16_t processId, uint16_t cantidadPaginas);

#endif /* TABLAMARCOS_H_ */
