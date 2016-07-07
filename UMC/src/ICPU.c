/*
 * Pagina.c
 *
 *  Created on: 4/5/2016
 *      Author: utnso
 */

#include "ICPU.h"


/*
 *  TODO casos especiales:
 *  	- el proceso no tiene marcos asignados y no tiene marcos disponibles-> rechazar pedido de memoria
 *  	- el proceso tiene marcos asignados,
 *  		no llego al maximo, pero no hay marcos libres en la memoria->aplicar reemplazo sobre los marcos que tenga en su tabla
 */


void *inicializarPrograma(stIni* ini){
	stHeaderIPC *unHeader;

	crearTabla(ini->sPI->processId, ini->sPI->cantidadPaginas);

#undef TEST_SIN_SWAP

#ifndef TEST_SIN_SWAP

	if(inicializarSwap(ini->sPI) == EXIT_FAILURE){
		log_error("No se pudo enviar el codigo a Swap");
		unHeader=nuevoHeaderIPC(ERROR);
		enviarHeaderIPC(ini->socketResp, unHeader);
		pthread_exit(NULL);
	}
#else
	// falta agregar rutina para paginar el codigo enviado y asi guardarlo en memoria
	uint16_t resTLB, resTabla, frameBuscado, marco;
	void *posicion;
	stRegistroTLB stTLB;
	stRegistroTP regTP;
	int hayTLB;

	marco = obtenerMarcoLibre();
	if(marco == 0)
		marco = reemplazarValorTabla(pid, pagina, regTP, REEMPLAZAR_MARCO);
	else{
		regTP.marco = marco;
		marco = reemplazarValorTabla(pid, pagina, regTP, NULL);
	}

	// cargo en memoria la pagina obtenida
	posicion = memoriaPrincipal+((marco-1)*losParametros.frameSize);
	escribirMemoria(posicion, losParametros.frameSize, leido);

	// cargo en TLB la pagina obtenida aplicando algoritmo de reemplazo de ser necesario
	if(usarTLB != 0){
		stTLB.pid = pid;
		stTLB.pagina = pagina;
		stTLB.marco = regTP.marco;
		reemplazarValorTLB(stTLB);
	}

#endif
	/*Se informa al nucleo que el programa se inicializo OK*/
	unHeader=nuevoHeaderIPC(OK);
	enviarHeaderIPC(ini->socketResp, unHeader);
	
	/* no guardo en memoria */

	/*Cierro este thread porque este es creado por Nucleo y voy a trabajar con el CPU*/
	pthread_exit(NULL);
}
void leerBytes(stPosicion* unaLectura, uint16_t pid, uint16_t socketCPU){

	uint16_t resTLB, resTabla, *frameBuscado;
	void *leido, *bytesLeidos, *pos;
	stRegistroTLB stTLB;
	stRegistroTP regTP;
	int hayTLB, ret;

	/* si esta disponible cache*/
	if ((hayTLB = estaActivadaTLB())== OK){
		resTLB =  buscarEnTLB(pid, unaLectura->pagina, &frameBuscado);
	}
	// no hay TLB o es un TLB miss
	if(hayTLB==ERROR || resTLB==0)
		resTabla = buscarEnTabla(pid, unaLectura->pagina, &frameBuscado);

	// leo el frame desde memoria si estan en tabla o TLB
	if(resTLB != 0 || resTabla != 0){

			// acceder a memoria con el resultado encontrado en cache
			pos = memoriaPrincipal+((*frameBuscado-1)*losParametros.frameSize);
			leido = leerMemoria(pos, unaLectura->size);

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
	// Page fault Lectura
	else{

		leido = ejecutarPageFault(pid, unaLectura->pagina, hayTLB && resTLB!=0);

		if(leido!=NULL){

			// con la pagina obtenida separo los bytes que se pidieron leer
			bytesLeidos = calloc(1, unaLectura->size);
			memcpy(bytesLeidos,leido+(unaLectura->offset),unaLectura->size);
			ret=OK;
		}else
			ret=ERROR;
		//envio la respuesta de la lectura a la CPU
		if(!enviarMensajeIPC(socketCPU,nuevoHeaderIPC(OK),bytesLeidos)){
			log_error("No se pudo enviar el MensajeIPC");
			return;
		}
		// libero lo enviado
		free(bytesLeidos);
		// libero pagina obtenida
		free(leido);
	}

}
void escribirBytes(stEscrituraPagina* unaEscritura, uint16_t pid, uint16_t socketCPU){
	uint16_t resTLB, resTabla, *frameBuscado;
	void *leido;
	uint16_t *posicion;
	stRegistroTP *registro;
	int hayTLB, ret;

	/* si esta disponible cache*/
	if ((hayTLB = estaActivadaTLB())== OK){
		resTLB = buscarEnTLB(pid, unaEscritura->nroPagina, &frameBuscado);
	}
	// no hay TLB o es un TLB miss
	if(hayTLB==ERROR || resTLB==0)
		resTabla = buscarEnTabla(pid, unaEscritura->nroPagina, &frameBuscado);

	// escribo el frame desde memoria si estan en tabla o TLB
	if(resTLB != 0 || resTabla != 0){

		// en el marco obtenido indico posicion para escribir los bytes pedidos
		posicion = ((*frameBuscado)*losParametros.frameSize)+unaEscritura->offset;
		escribirMemoria(posicion, unaEscritura->tamanio, unaEscritura->buffer);

		//envio la respuesta de la Escritura a la CPU
		if(!enviarHeaderIPC(socketCPU,nuevoHeaderIPC(OK))){
			log_error("No se pudo enviar el MensajeIPC");
			return;
		}

	}
	// Page fault Escritura
	else{

		leido = ejecutarPageFault(pid, unaEscritura->nroPagina, estaActivadaTLB() && resTLB!=0);

		if(leido!=NULL){


		// busco en TLB, deberia estar porque page fault actulizo
		resTLB = buscarEnTLB(pid, unaEscritura->nroPagina, &frameBuscado);

		// prendo el bit de modificado en Tabla
		registro = buscarRegistroEnTabla(pid, unaEscritura->nroPagina);
		registro->bitModificado=1;

		// en la pagina obtenida escribo los bytes que se pidieron
		posicion = ((*frameBuscado)*losParametros.frameSize)+unaEscritura->offset;
		escribirMemoria(posicion, unaEscritura->tamanio, unaEscritura->buffer);
		ret=OK;
		}
		else
			ret=ERROR;

		//envio la respuesta de la lectura a la CPU
		if(!enviarHeaderIPC(socketCPU,nuevoHeaderIPC(ERROR))){
			log_error("No se pudo enviar el MensajeIPC");
			return;
		}

		// libero pagina obtenida
		free(leido);
	}
}
void* ejecutarPageFault(uint16_t pid, uint16_t pagina, uint16_t usarTLB){
	uint16_t marco;
	void *leido, *posicion;
	stRegistroTLB stTLB;
	stRegistroTP regTP, *registro;

	// acceder a swap con las pagina que necesito
	leido = recibirPagina(pid, pagina);

	// cargo en Tabla la pagina obtenida aplicando algoritmo de reemplazo de ser necesario
	regTP.bit2ndChance=0;
	regTP.bitModificado=0;
	regTP.bitPresencia=1;
	marco = obtenerMarcoLibre();
	if(marco == 0)
		registro = reemplazarValorTabla(pid, pagina, regTP, REEMPLAZAR_MARCO);
	else{
		regTP.marco = marco;
		registro = reemplazarValorTabla(pid, pagina, regTP, NULL);
	}

	if (registro==NULL)
		// Se rechaza por no haber memoria
		return registro;

	// cargo en memoria la pagina obtenida
	posicion = memoriaPrincipal+((registro->marco-1)*losParametros.frameSize);
	escribirMemoria(posicion, losParametros.frameSize, leido);

	// cargo en TLB la pagina obtenida aplicando algoritmo de reemplazo de ser necesario
	if(usarTLB != 0){
		stTLB.pid = pid;
		stTLB.pagina = pagina;
		stTLB.marco = regTP.marco;
		reemplazarValorTLB(stTLB);
	}
	return leido;
}
void finalizarPrograma(uint16_t pid, uint16_t socketCPU){

	liberarTablaPid(pid);

	// TODO liberarTLB ?
	//liberarTLB();
    destruirPrograma(pid);


}
void *finalizarProgramaNucleo(stEnd *fin){

	finalizarPrograma(fin->pid, fin->socketResp);
	//pthread_exit(NULL);
}
void cambiarContexto(uint16_t pid){

	stRegistroTP *data;
	data = buscarPID(pid);

	// TODO es necesario actualizar swap con paginas que tienen byte modificado ??

	// TODO es necesario hacer un flush del pid en TLB ???


}

void realizarAccionCPU(uint16_t socket){

	stMensajeIPC *unMensaje;
	uint16_t pidActivo, pagina;
	stEnd *end;
	stPosicion *posR;
	stEscrituraPagina *posW;

	while(1){

		if(!recibirMensajeIPC(socket, unMensaje)){
			log_error("Thread Error - No se pudo recibir mensaje de respuesta - socket: %d", socket);
			//liberarHeaderIPC(unMensaje->header);
			free(unMensaje->contenido);
			free(unMensaje);
			close(socket);
			pthread_exit(NULL);
		}

		switch(unMensaje->header.tipo){

		case READ_BTYES_PAGE:

			posR =(stPosicion*)(unMensaje->contenido);

			leerBytes(posR, pidActivo, socket);

			break;

		case WRITE_BYTES_PAGE:

			posW =(stEscrituraPagina*)(unMensaje->contenido);

			escribirBytes(posW, pidActivo, socket);

			break;

		case FINPROGRAMA:

			finalizarPrograma(pidActivo, socket);
			break;

		case CAMBIOCONTEXTO:

			pidActivo = (uint16_t)*(unMensaje->contenido);
			//cambiarContexto(pidActivo);

			break;

		default:
			log_info("Se recibio una peticion con un codigo desconocido...%i\n", unMensaje->header.tipo);
			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
			break;

		}/*Cierro switch(unMensaje.header.tipo)*/
	}/*Cierro while*/
}
