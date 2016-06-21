/*
 * TablaMarcos.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "TablaMarcos.h"

stRegistroTP *buscarRegistroEnTabla(uint16_t pid, uint16_t paginaBuscada){

	stNodoListaTP *nodo;
	stRegistroTP *registro;

	nodo = buscarPID(pid);
	registro = nodo->tabla+(sizeof(stRegistroTP)*paginaBuscada);

	return registro;
}

int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t **frame){
	stNodoListaTP *nodo;
	stRegistroTP *registro;

	registro = buscarRegistroEnTabla(pid, paginaBuscada);
	*frame = malloc(sizeof(uint16_t));
	**frame = registro->marco;
	return registro->marco;
}
stRegistroTP *EjecutarClock(stNodoListaTP *nodo, uint16_t pagina, stRegistroTP registro, uint8_t flag){
	stRegistroTP *regTP;
	int i;

	// TODO EjecutarClock
	i=0;
	//for(i=0;i<nodo->size;i++){
		regTP = nodo->tabla+(sizeof(stRegistroTP)*i);
		//if(regTP->bit2ndChance==1){
			regTP->bit2ndChance=registro.bit2ndChance;
			regTP->bitModificado=registro.bitModificado;
			regTP->bitPresencia=registro.bitPresencia;
			regTP->marco=registro.marco;
			//break;
		//}
	//}
	//if(flag == REEMPLAZAR_MARCO)
	return regTP;
}
stRegistroTP *EjecutarClockModificado(stNodoListaTP *nodo, uint16_t pagina, stRegistroTP registro, uint8_t flag){

	//TODO EjecutarClockModificado
	return EjecutarClock(nodo, pagina, registro,flag);
}
stRegistroTP *reemplazarValorTabla(uint16_t pid, uint16_t pagina, stRegistroTP registro, uint8_t flag){

	stNodoListaTP *nodo;
	stRegistroTP *retorno;
	int i;

	nodo = buscarPID(pid);

	if(nodo->size < losParametros.frameByProc){
		for(i=0;i<nodo->size;i++){
			retorno = nodo->tabla+(sizeof(stRegistroTP)*i);
			if(retorno->bitPresencia==0){
				retorno->bit2ndChance=registro.bit2ndChance;
				retorno->bitModificado=registro.bitModificado;
				retorno->bitPresencia=registro.bitPresencia;
				retorno->marco=registro.marco;
				break;
			}
		}
		return retorno;
	}else{
		if(string_equals_ignore_case(losParametros.algoritmo,"CLOCK"))
			retorno = EjecutarClock(nodo, pagina, registro,flag);
		else
			retorno = EjecutarClockModificado(nodo, pagina, registro,flag);
	}

	return retorno;
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

	nodo->size=cantidadPaginas;
	nodo->tabla=tabla;

	//creo tabla sino existe
	if(TablaMarcos==NULL)
		TablaMarcos = list_create();

	//enlazo en la lista
	list_add_in_index(TablaMarcos,processId, nodo);

	return 0;
}
stNodoListaTP *buscarPID(uint16_t pid){
	return (stNodoListaTP*)list_get(TablaMarcos,pid);
}
void liberarTablaPid(uint16_t pid){
	stNodoListaTP *nodo;
	stRegistroTP *registro;
	int i;

	nodo = buscarPID(pid);
	for(i=0;i<nodo->size;i++){
		registro = nodo->tabla+(sizeof(stRegistroTP)*i);
		liberarMarco(registro->marco);
	}
	free(registro);
	list_remove(TablaMarcos,pid);

}
