/*
 * interprete.h
 *
 *  Created on: 12/5/2016
 *      Author: utnso
 */

#ifndef INTERPRETE_H_
#define INTERPRETE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb.h"
#include "nucleo.h"
#define SALTO_DE_LINEA '\n'

typedef struct stNodoIndiceDeCodigo {
	int offset;
	int size;
} t_NodoICodigo;

typedef struct stInterprete {
	int posActual;
	int posMaxima;
	char *programa;
	int cantSentencias;
} t_Interprete;

/**
 * @NAME: interprete
 * @PRE:  unPCB->indiceDeCodigo vacio.
 * @POST: lista indice de codigo cargada , envio de sentencias al UMC
 */
void iniciarInterprete(t_Interprete *tInterprete, char *programa);

/**
 * @NAME: crearInterprete
 * @PRE:  estructura tInterprete vacia
 * @POST: programa cargado y contadores inicializados.
 */
int crearInterprete(t_Interprete *tInterprete, char *programa);

/**
 * @NAME: existeProxSentencia
 * @PRE:  -
 * @POST: (posicion) TRUE, 0 FALSE
 */
int existeProxSentencia(t_Interprete *tInterprete);

/**
 * @NAME: proximaSentencia
 * @PRE:  mueve puntero char a la proxima sentencia
 * @POST: devuelve 1 si pudo , 0 si fue la ultima
 */
int proximaSentencia(t_Interprete *tInterprete);

#endif /* INTERPRETE_H_ */
