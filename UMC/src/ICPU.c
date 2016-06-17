/*
 * Pagina.c
 *
 *  Created on: 4/5/2016
 *      Author: utnso
 */

#include "ICPU.h"

void *inicializarPrograma(stIni* ini){
	stHeaderIPC *unHeader;

	if(inicializarSwap(ini->sPI) == EXIT_FAILURE){
		log_error("No se pudo enviar el codigo a Swap");
		unHeader=nuevoHeaderIPC(ERROR);
		enviarHeaderIPC(ini->socketResp, unHeader);
		pthread_exit(NULL);
	}
	/* TODO crearTabla */
	crearTabla(ini->sPI->processId, ini->sPI->cantidadPaginas);

	/* no guardo en memoria */

	pthread_exit(NULL);
}
void *leerBytes(stRead* unaLectura){
	/* Si NO esta la pagina disponible */
	if (estaActivadaTLB() && buscarEnTLB(unaLectura->sPos->pagina)!=0){
		//pid = elegirReemplazo();
		cambiarContexto(unaLectura->sPos->pagina);
	}


	void *leido;
	leido = leerMemoria(unaLectura->sPos->pagina,unaLectura->sPos->offset, unaLectura->sPos->size);
	if(!enviarMensajeIPC(unaLectura->socketResp,nuevoHeaderIPC(OK),leido)){
		log_error("No se pudo enviar el MensajeIPC");
		return (-1);
	}
	free(leido);

	/* TODO devolver por socket */

	pthread_exit(NULL);
}
void *escribirBytes(stWrite* unaEscritura){
	/* TODO escribirBytes */
	return 0;
}
void *finalizarPrograma(stEnd *fin){
	/* TODO finalizarPrograma */
	pthread_exit(NULL);
}
int cambiarContexto(uint16_t pagina){
	/* TODO cambiar contexto */

	return 0;
}
int elegirReemplazo(int cantidad){
	return 0;
}
int hayMarcoslibres(int cantidad){
	return 0;
}
int estaPaginaDisponible(uint16_t pagina){
	return 0;
}


void realizarAccionCPU(uint16_t socket){

	stWrite *wr;
	stRead *read;
	stEnd *end;
	stMensajeIPC *unMensaje;

	while(1){

		if(!recibirMensajeIPC(socket, unMensaje)){
			log_error("Thread Error - No se pudo recibir mensaje de respuesta - socket: %d", socket);
			liberarHeaderIPC(unMensaje->header);
			liberarHeaderIPC(unMensaje->contenido);
			close(socket);
			pthread_exit(NULL);
		}

		switch(unMensaje->header.tipo){

		case READ_BTYES_PAGE:

			read = (stRead*)calloc(1,sizeof(stRead));
			read->socketResp = socket;
			read->sPos = (stPosicion*)unMensaje->contenido;

			leerBytes(read);

			break;

		case WRITE_BYTES_PAGE:

			wr = (stWrite*)calloc(1,sizeof(stWrite));
			wr->socketResp = socket;
			wr->sEP = (stEscrituraPagina*)unMensaje->contenido;

			escribirBytes(wr);

			break;

		case FINPROGRAMA:

			end = calloc(1,sizeof(stEnd));
			end->socketResp = socket;
			end->pid = atoi(unMensaje->contenido);

			finalizarPrograma(end);
			break;

		case CAMBIOCONTEXTO:
			/* TODO actualizo tabla de marcos con el pid y TLB flush de ese pid */

			break;

		default:
			log_info("Se recibio una peticion con un codigo desconocido...%i\n", unMensaje->header.tipo);
			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
			break;

		}/*Cierro switch(unMensaje.header.tipo)*/
	}/*Cierro while*/
}
