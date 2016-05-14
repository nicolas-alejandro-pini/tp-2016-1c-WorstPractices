/*
 * elestaclibrary.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include "elestaclibrary.h"

int crearPCB(stPCB *unPCB) {
	unPCB->indicesCodigo = list_create();
	return 0;
}

void liberarPCB(stPCB *unPCB) {
	list_destroy(unPCB->indicesCodigo);
}


