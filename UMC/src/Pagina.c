/*
 * Pagina.c
 *
 *  Created on: 4/5/2016
 *      Author: utnso
 */

#include "Pagina.h"

int inicializarPrograma(stPageIni* ini){
	/* TODO inicializarPrograma */
	return 0;
}
int leerBytes(stPosicion* unaLectura){
	/* TODO leerBytes */
	return 0;
}
int escribirBytes(stEscrituraPagina* unaEscritura){
	/* TODO escribirBytes */
	return 0;
}
int finalizarPrograma(uint16_t unProcessId){
	/* TODO finalizarPrograma */
	return 0;
}

void realizarAccionUMC(unsigned int tipo, char* contenido){

	pthread_t tid;
	pthread_attr_t attr;
	/*
	stPageIni* ini;
	stPosicion* unaLectura;
	stEscrituraPagina* unaEscritura;
	uint16_t unProcessId;
*/

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);

	switch(tipo){

		/* TODO incluir pedidos de la consola UMC */
		case INICIALIZAR_PROGRAMA:

			pthread_create(&tid,&attr,(void*)inicializarPrograma,contenido);
			break;

		case READ_BTYES_PAGE:
			pthread_create(&tid,&attr,(void*)leerBytes,contenido);
			break;

		case WRITE_BYTES_PAGE:
			pthread_create(&tid,&attr,(void*)escribirBytes,contenido);
			break;

		case FIN_PROGRAMA:
			pthread_create(&tid,&attr,(void*)finalizarPrograma,contenido);
			break;

		default:
			printf("\nSe recibio una peticion con un codigo desconocido...%i\n", tipo);
			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
			break;

	}
	/*Cierro switch(unMensaje.header.tipo)*/
}
