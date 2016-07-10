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
	uint16_t longitud_tabla;

	// Crea tabla con como maximo marcos_x_proceso registros ---> NO, siempre asigno lo que piden, limito la asignacion de marcos
	//if(ini->sPI->cantidadPaginas > ini->marcos_x_proceso)
		//longitud_tabla = ini->marcos_x_proceso;
//	else
		longitud_tabla = ini->sPI->cantidadPaginas;

	crearTabla(ini->sPI->processId, longitud_tabla);

#undef TEST_SIN_SWAP

#ifndef TEST_SIN_SWAP

	if(inicializarSwap(ini->sPI) == EXIT_FAILURE){
		log_error("No se pudo enviar el codigo a Swap");
		unHeader=nuevoHeaderIPC(ERROR);
		enviarHeaderIPC(ini->socketResp, unHeader);
		pthread_exit(NULL);
	}
#else

	uint16_t marco;
	void *posicion;
	stRegistroTP* regTP = NULL;
	stNodoListaTP* nodoListaTP = NULL;
	// TODO Simulo pedido del CPU para cargar en memoria el codigo del programa

	nodoListaTP = buscarPID(ini->sPI->processId);

	int largo_programa = (strlen(ini->sPI->programa) / losParametros.frameSize) +1;
	char *programa_paginado = calloc(1,sizeof(char)*(losParametros.frameSize*(largo_programa)));
	memcpy(programa_paginado, ini->sPI->programa, losParametros.frameSize);
	int pagina = 0;

	while(pagina < nodoListaTP->size && pagina < largo_programa){
		marco = obtenerMarcoLibre();
		regTP = nodoListaTP->tabla + sizeof(stRegistroTP)*pagina;
		regTP->marco = marco;
		regTP->bit2ndChance=0;
		regTP->bitModificado=0;
		regTP->bitPresencia=1;
		posicion = memoriaPrincipal+((marco-1)*losParametros.frameSize);
		escribirMemoria(posicion, (uint16_t) losParametros.frameSize, (void*) programa_paginado + (losParametros.frameSize*pagina));
		pagina++;
	}

#endif
	/*Se informa al nucleo que el programa se inicializo OK*/
	unHeader=nuevoHeaderIPC(OK);
	enviarHeaderIPC(ini->socketResp, unHeader);
	if (!enviarHeaderIPC(ini->socketResp, unHeader)) {
		log_error("Hubo un problema al escribir OK de inicalizacion al nucleo - pid %d", ini->sPI->processId);
		close(ini->socketResp);
	}
	liberarHeaderIPC(unHeader);

	/* no guardo en memoria */

	/*Cierro este thread porque este es creado por Nucleo y voy a trabajar con el CPU*/
	return NULL;
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
	stHeaderIPC *unHeader;

	// liberar tabla de paginas para el pid
	liberarTablaPid(pid);

	// liberar TLB
	flushTLB(pid);

	// liberar de swap del pid
    destruirPrograma(pid);

    unHeader = nuevoHeaderIPC(OK);
    enviarHeaderIPC(socketCPU, unHeader);
    liberarHeaderIPC(unHeader);

    return;
}
void *finalizarProgramaNucleo(stEnd *fin){

	finalizarPrograma(fin->pid, fin->socketResp);
	return NULL;
}
void cambiarContexto(uint16_t pid){

	stRegistroTP *data;
	data = buscarPID(pid);
	if(data==NULL)
		log_error("pid %d no encontrado en el cambio de contexto - programa no inicializado", pid);

	// No se actuliza tabla de pagina del otro proceso asi qeu no es necesario actualizar swap con paginas que tienen byte modificado

	// TODO es necesario hacer un flush del pid en TLB ??? creo que no

	return;
}

void realizarAccionCPU(uint16_t socket){

	stMensajeIPC unMensaje;
	uint16_t pidActivo, pagina;
	stEnd *end;
	stPosicion *posR;
	stEscrituraPagina *posW;

	while(1){

		if(!recibirMensajeIPC(socket, &unMensaje)){
			log_error("Thread Error - No se pudo recibir mensaje de respuesta - socket: %d", socket);
			//liberarHeaderIPC(unMensaje->header);
			close(socket);
			return;//pthread_exit(NULL);
		}

		switch(unMensaje.header.tipo){

		case READ_BTYES_PAGE:

			posR =(stPosicion*)(unMensaje.contenido);

			leerBytes(posR, pidActivo, socket);

			break;

		case WRITE_BYTES_PAGE:

			posW =(stEscrituraPagina*)(unMensaje.contenido);

			escribirBytes(posW, pidActivo, socket);

			break;

		case FINPROGRAMA:

			finalizarPrograma(pidActivo, socket);
			break;

		case CAMBIOCONTEXTO:

			pidActivo = (uint16_t)*(unMensaje.contenido);
			cambiarContexto(pidActivo);

			break;

		default:
			log_info("Se recibio una peticion con un codigo desconocido...%i\n", unMensaje.header.tipo);
			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
			break;

		}/*Cierro switch(unMensaje.header.tipo)*/
	}/*Cierro while*/
}
