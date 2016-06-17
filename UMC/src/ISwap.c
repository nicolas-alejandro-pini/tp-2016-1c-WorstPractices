/*
 * ISwap.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#include "ISwap.h"

int inicializarSwap(stPageIni *st){

	t_paquete *paquete;
	stHeader* respuesta;
	int ret;
	/*
	 * PASAR pid, cantidad de pagina, y codigo prg
	 *
	 * devuelve OK o ERROR
	 */
	paquete->data=st;
	paquete->header.type=INICIAR_PROGRAMA;
	serializar_header(paquete);

	enviar_paquete(losParametros.sockSwap, paquete);
	respuesta = (stHeader*)calloc(1,sizeof(stHeader));
	recibirHeader(losParametros.sockSwap, respuesta);

	if(respuesta->tipo==OK)
		ret=EXIT_FAILURE;
	else
		ret=EXIT_SUCCESS;

	free_paquete(paquete);
	free(respuesta);

	return ret;

}

//define INICIAR_PROGRAMA	141ul /* pid (ui), cantidad paginas (ui), (ul), codigo prg (char*) */
//define DESTRUIR_PROGRAMA	142ul /* pid */
//define LEER_PAGINA			143ul /* pid, numero de pagina */
//define ESCRIBIR_PAGINA		144ul /* pid, numero de pagina, contenido de pagina */

/*
 * enviarPagina()<- NULL
 * recibir pagina()<-contenido de la pagina
 *
 * destruirPrograma()<-NULL
 */

int enviarPagina(uint16_t pagina, char* buffer){

}

char* recibirPagina(uint16_t pagina){

}

int destruirPrograma(uint16_t pid){

}
