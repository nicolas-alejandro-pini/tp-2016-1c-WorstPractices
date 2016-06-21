/*
 * ISwap.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#include "ISwap.h"

int inicializarSwap(stPageIni *st){

	t_paquete *paquete;
	stHeaderIPC* respuesta;
	int ret;
	/*
	 * PASAR pid, cantidad de pagina, y codigo prg
	 *
	 * devuelve OK o ERROR
	 */
	paquete = calloc(1,sizeof(t_paquete));
	paquete->data=(void*)st;
	paquete->header.type=INICIAR_PROGRAMA;
	serializar_header(paquete);

	enviar_paquete(losParametros.sockSwap, paquete);
	respuesta = (stHeaderIPC*)calloc(1,sizeof(stHeader));
	recibirHeaderIPC(losParametros.sockSwap, respuesta);

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
	t_paquete *paquete;
	stEscrituraPagina* ep;
	int ret;
		/*
		 * PASAR pid, cantidad de pagina, y codigo prg
		 *
		 * devuelve OK o ERROR
		 */
	paquete = calloc(1,sizeof(t_paquete));
	ep = calloc(1,sizeof(stEscrituraPagina));
	paquete->data=ep;
	paquete->header.type=ESCRIBIR_PAGINA;
	serializar_header(paquete);

	enviar_paquete(losParametros.sockSwap, paquete);
	//respuesta = (stHeaderIPC*)calloc(1,sizeof(stHeader));
	//recibirHeaderIPC(losParametros.sockSwap, respuesta);

//		if(respuesta->tipo==OK)
//			ret=EXIT_FAILURE;
//		else
//			ret=EXIT_SUCCESS;

		free_paquete(paquete);
		//free(respuesta);

		return EXIT_SUCCESS;
}

char* recibirPagina(uint16_t pagina){
	t_paquete *paquete;
	stPosicion * lp;
	stMensajeIPC* respuesta;
	unsigned char* paginaSwap;
	int ret;

	paquete = calloc(1,sizeof(t_paquete));
	lp = calloc(1,sizeof(stEscrituraPagina));
	lp->pagina=pagina;
	paquete->data=lp;
	paquete->header.type=LEER_PAGINA;
	serializar_header(paquete);

	enviar_paquete(losParametros.sockSwap, paquete);
	respuesta = (stMensajeIPC*)calloc(1,sizeof(stMensajeIPC));
	recibirMensajeIPC(losParametros.sockSwap, respuesta);

	if(respuesta->header.tipo==OK)
		ret=EXIT_FAILURE;
	else
		ret=EXIT_SUCCESS;

	paginaSwap = calloc(1, respuesta->header.largo);
	memcpy(paginaSwap, respuesta->contenido, respuesta->header.largo);

	free(lp);
	free_paquete(paquete);
	free(respuesta->contenido);
	free(respuesta);
	return paginaSwap;
}

int destruirPrograma(uint16_t pid){
	t_paquete *paquete;
	uint16_t* p;

	paquete = calloc(1,sizeof(t_paquete));
	p = calloc(1,sizeof(uint16_t));
	paquete->data=p;
	paquete->header.type=DESTRUIR_PROGRAMA;
	serializar_header(paquete);

	enviar_paquete(losParametros.sockSwap, paquete);

	free_paquete(paquete);
	free(p);

	return EXIT_SUCCESS;
}
