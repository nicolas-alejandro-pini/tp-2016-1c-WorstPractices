/*
 * pcb.c
 *
 *  Created on: 12/5/2016
 *      Author: utnso
 */

#include "pcb.h"

int crearPCB(stPCB *unPCB) {
	unPCB->indiceDeCodigo = list_create();
	return 0;
}

void liberarPCB(stPCB *unPCB) {
	list_destroy(unPCB->indiceDeCodigo);
}
