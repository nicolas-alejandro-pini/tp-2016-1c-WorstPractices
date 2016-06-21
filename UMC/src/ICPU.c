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
	}else{
		crearTabla(ini->sPI->processId, ini->sPI->cantidadPaginas);
		unHeader=nuevoHeaderIPC(OK);
		enviarHeaderIPC(ini->socketResp, unHeader);
	}

	/* no guardo en memoria */

	/*Cierro este thread porque este es creado por Nucleo y voy a trabajar con el CPU*/
	pthread_exit(NULL);
}
void leerBytes(stPosicion* unaLectura, uint16_t pid, uint16_t socketCPU){

	uint16_t resTLB, resTabla, frameBuscado;
	void *leido, *bytesLeidos;
	stRegistroTLB stTLB;

	/* si esta disponible en cache*/
	if (estaActivadaTLB()== OK)
		resTLB =  buscarEnTLB(pid, unaLectura->pagina, &frameBuscado);

	// TLB miss
	else
		resTabla = buscarEnTabla(pid, unaLectura->pagina, &frameBuscado);

	// leo el frame desde memoria
	if(resTLB == OK || resTabla == OK){

			// acceder a memoria con el resultado encontrado en cache
			leido = leerMemoria(frameBuscado);

			// con el marco obtenido separo los bytes que se pidieron leer
			bytesLeidos = calloc(1, unaLectura->size);
			memcpy(bytesLeidos,leido+unaLectura->offset,unaLectura->size);

			//envio la respuesta de la lectura a la CPU
			if(!enviarMensajeIPC(socketCPU,nuevoHeaderIPC(OK),bytesLeidos)){
				log_error("No se pudo enviar el MensajeIPC");
				return;
			}

			//libero memoria
			free(leido);
			free(bytesLeidos);

	}
	// Page fault
	else{

		// acceder a swap con las pagina qeu necesito
		leido = recibirPagina(unaLectura->pagina);

		// con la pagina obtenida separo los bytes que se pidieron leer
		bytesLeidos = calloc(1, unaLectura->size);
		memcpy(bytesLeidos,leido+unaLectura->offset,unaLectura->size);

		//envio la respuesta de la lectura a la CPU
		if(!enviarMensajeIPC(socketCPU,nuevoHeaderIPC(OK),bytesLeidos)){
			log_error("No se pudo enviar el MensajeIPC");
			return;
		}
		// libero lo enviado
		free(bytesLeidos);

		// cargo en memoria la pagina obtenida


		// TODO cargo en Tabla la pagina obtenida aplicando algoritmo de reemplazo de ser necesario


		// TODO cargo en memoria la pagina obtenida aplicando algoritmo de reemplazo de ser necesario
		if(estaActivadaTLB() && resTLB==ERROR){

			stTLB.pid = pid;
			stTLB.pagina = unaLectura->pagina;
			stTLB.marco = frameBuscado;
			reemplazarValorTLB(stTLB);
		}

		// libero pagina obtenida
		free(leido);
	}

}
void *escribirBytes(stEscrituraPagina* unaEscritura){
	/* TODO escribirBytes */
	return 0;
}
void *finalizarPrograma(stEnd *fin){
	/* TODO finalizarPrograma */
	pthread_exit(NULL);
}
int cambiarContexto(uint16_t pid){
	/* TODO cambiarContexto */
	return 0;
}

int hayMarcoslibres(int cantidad){
	return 0;
}
int estaPaginaDisponible(uint16_t pagina){
	return 0;
}


void realizarAccionCPU(uint16_t socket){
//
//	stMensajeIPC *unMensaje;
//	uint16_t pidActivo;
//	stEnd end;
//
//
//	while(1){
//
//		if(!recibirMensajeIPC(socket, unMensaje)){
//			log_error("Thread Error - No se pudo recibir mensaje de respuesta - socket: %d", socket);
//			liberarHeaderIPC(unMensaje->header);
//			liberarHeaderIPC(unMensaje->contenido);
//			close(socket);
//			pthread_exit(NULL);
//		}
//
//		switch(unMensaje->header.tipo){
//
//		case READ_BTYES_PAGE:
//
////			stPosicion *posR =(stPosicion*)unMensaje->contenido;
//
//			leerBytes(posR, pidActivo, socket);
//
//			break;
//
//		case WRITE_BYTES_PAGE:
//
//			stPosicion *posW =(stPosicion*)unMensaje->contenido;
//
//			escribirBytes(posW, pidActivo, socket);
//
//			break;
//
//		case FINPROGRAMA:
//
//			end = calloc(1,sizeof(stEnd));
//			end->socketResp = socket;
//			end->pid = atoi(unMensaje->contenido);
//
//			finalizarPrograma(end);
//			break;
//
//		case CAMBIOCONTEXTO:
//			/* TODO actualizo tabla de marcos con el pid y TLB flush de ese pid */
//
//
//			break;
//
//		default:
//			log_info("Se recibio una peticion con un codigo desconocido...%i\n", unMensaje->header.tipo);
//			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
//			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
//			break;
//
//		}/*Cierro switch(unMensaje.header.tipo)*/
//	}/*Cierro while*/
}
