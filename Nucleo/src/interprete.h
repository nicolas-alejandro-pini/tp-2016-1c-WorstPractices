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
#include <commons/elestaclibrary.h>
#include "nucleo.h"   // estructura stEstado

#define SALTO_DE_LINEA '\n'

typedef struct stNodoIndiceDeCodigo {
	int offset;
	int size;
} t_NodoICodigo;

typedef struct stInterprete {
	int longPrograma;
	char *programa;
	char *posActual;
	int cantSentencias;
} t_Interprete;

/**
 * @NAME: interprete
 * @PRE:  programa, estado Actual del UMC, un PCB
 * @POST: Carga el indice de codigo en una lista y envia el programa al UMC
 */
int interprete(stPCB *unPCB, const stEstado elEstadoActual, char *programa);

/**
 * @NAME: interprete
 * @PRE:  unPCB->indiceDeCodigo vacio.
 * @POST: lista indice de codigo cargada , envio de sentencias al UMC
 */
void iniciarInterprete(t_Interprete *tInterprete, char *programa);

/**
 * @NAME: proximaSentencia
 * @PRE:  mueve puntero char a la proxima sentencia
 * @POST: devuelve 1 si pudo , 0 si fue la ultima
 */
int proximaSentencia(t_Interprete *tInterprete);

#endif /* INTERPRETE_H_ */
