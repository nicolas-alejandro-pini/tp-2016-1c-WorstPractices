/*
 * TablaMarcos.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "TablaMarcos.h"

int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t frame){
	/* TODO buscarEnTabla */
	return 0;
}
int reemplazarValorTabla(uint16_t Pagina, stRegistroTP registro){
	/* TODO reemplazarValorTabla */
	return 0;
}
int crearTabla(uint16_t processId, uint16_t cantidadPaginas){

	stNodoListaTP *nodo;
	stRegistroTP *tabla;
	int i;

	tabla = calloc(cantidadPaginas,sizeof(stRegistroTP));

	//recorro la tabla para inicializarla
	for(i=0;i<cantidadPaginas;i++){
		(tabla+(sizeof(stRegistroTP)*i))->bit2ndChance=0;
		(tabla+(sizeof(stRegistroTP)*i))->bitModificado=0;
		(tabla+(sizeof(stRegistroTP)*i))->bitPresencia=0;
	}

	nodo = calloc(1,sizeof(stNodoListaTP));

	nodo->pid=processId;
	nodo->tabla=tabla;

	//creo tabla sino existe
	if(TablaMarcos==NULL)
		TablaMarcos = list_create();

	//enlazo en la lista
	list_add(TablaMarcos, nodo);

	return 0;
}
int buscarPID(uint16_t pid){
	/* TODO buscarPID */
	return 0;
}
