/*
 * pcb.c
 *
 *  Created on: 17/5/2016
 *      Author: utnso
 */
#include "pcb.h"

stPCB * crearPCB(t_metadata_program *unPrograma, int socketConsola) {

	stPCB * unPCB = malloc(sizeof(stPCB));
	nuevoPID(unPCB->pid);
	unPCB->pc = 0;
	unPCB->socketConsola = 0;
	unPCB->socketCPU = 0;
	unPCB->socketConsola = 0;
	unPCB->paginaInicial = 0;
	unPCB->tamanioPaginas = 0;
	unPCB->cantidadPaginas = 0;
	return (unPCB);
}

void liberarPCB(stPCB *unPCB) {
	free(unPCB);
}

void nuevoPID(char *id) {
	char i;

	srand(time(NULL));
	for (i = 0; i < 15; i++) {
		*(id + i) = (char) rand();
	}
	id[15] = '\0';
}
