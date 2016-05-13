/*
 * pcb.h
 *
 *  Created on: 12/5/2016
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>

typedef struct stPCB {
	t_list *indiceDeCodigo;
}stPCB;

/**
 * @NAME: crearPCB
 * @PRE:  un puntero PCB
 * @POST: memoria reservada para la estructura PCB
 */
int crearPCB(stPCB *unPCB);

/**
 * @NAME: liberarPCB
 * @PRE:  una estructura PCB con memoria reservada
 * @POST: libera la estructura stPCB
 */
void liberarPCB(stPCB *unPCB);

#endif /* PCB_H_ */
