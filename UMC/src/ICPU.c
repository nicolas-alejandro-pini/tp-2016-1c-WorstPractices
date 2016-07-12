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


int inicializarPrograma(int unCliente){
	stPageIni *unPageIni = NULL;
	stHeaderIPC *unHeader = NULL;
	t_paquete paquete_stPageIni;

	if (recibir_paquete(unCliente, &paquete_stPageIni)) {
		log_error("No se pudo recibir paquete de inicio de programa");
		close(unCliente);
		return EXIT_FAILURE;
	}

	unPageIni = (stPageIni*)malloc(sizeof(stPageIni));
	deserializar_inicializar_programa(unPageIni,&paquete_stPageIni);

	/* Crea Tabla de Paginas , Copia los valores, se puede liberar unPageIni */
	crearTabla(unPageIni->processId, unPageIni->cantidadPaginas);

	if(inicializarSwap(unPageIni) == EXIT_FAILURE){
		log_error("No se pudo enviar el codigo a Swap");
		unHeader=nuevoHeaderIPC(ERROR);
		enviarHeaderIPC(unCliente, unHeader);
		return EXIT_FAILURE;
	}

	/*Se informa al nucleo que el programa se inicializo OK*/
	unHeader=nuevoHeaderIPC(OK);
	if (!enviarHeaderIPC(unCliente, unHeader)) {
		log_error("Hubo un problema al escribir OK de inicalizacion al nucleo - pid %d", unPageIni->processId);
		close(unCliente);
		return EXIT_FAILURE;
	}

	liberarHeaderIPC(unHeader);
	free(unPageIni->programa);
	free(unPageIni);

	return EXIT_SUCCESS;
}

void leerBytes(stPosicion* unaLectura, uint16_t pid, uint16_t socketCPU){

	uint16_t resTLB = 0, resTabla = 0, frameBuscado = 0;
	void *leido = NULL, *bytesLeidos = NULL, *pos = NULL;
	int hayTLB = 0, ret = 0;
	stHeaderIPC *unHeader;

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
			pos = memoriaPrincipal+((frameBuscado-1)*losParametros.frameSize);
			leido = leerMemoria(pos, losParametros.frameSize);

			//envio la respuesta de la lectura a la CPU
			unHeader = nuevoHeaderIPC(OK);
			unHeader->largo=unaLectura->size;
			if(!enviarMensajeIPC(socketCPU,unHeader,leido+unaLectura->offset)){
				log_error("No se pudo enviar el MensajeIPC");
				return;
			}

			//libero memoria
			free(leido);
			liberarHeaderIPC(unHeader);

	}
	// Page fault Lectura
	else{

		leido = ejecutarPageFault(pid, unaLectura->pagina, estaActivadaTLB()==OK && resTLB==0);

		if(leido!=NULL){

			// con la pagina obtenida separo los bytes que se pidieron leer
			bytesLeidos = calloc(1, unaLectura->size);
			memcpy(bytesLeidos,leido+(unaLectura->offset),unaLectura->size);
			ret=OK;
		}else
			ret=ERROR;
		//envio la respuesta de la lectura a la CPU
		unHeader = nuevoHeaderIPC(OK);
		unHeader->largo = unaLectura->size;
		if(!enviarMensajeIPC(socketCPU,unHeader,bytesLeidos)){
			log_error("No se pudo enviar el MensajeIPC");
			return;
		}
		// libero lo enviado
		liberarHeaderIPC(unHeader);
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

		leido = ejecutarPageFault(pid, unaEscritura->nroPagina, estaActivadaTLB()==OK && resTLB==0);

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

uint32_t cambiarContexto(stMensajeIPC *unMensaje){

	stNodoListaTP *data = NULL;
	uint32_t pid = 0;

	/* Valido que lleguen los 4 bytes del int pid */
	if(unMensaje->header.largo != sizeof(uint32_t))
	{
		return pid; // PID 0 (disponible)
	}

	memcpy(&(pid), unMensaje->contenido, sizeof(uint32_t));
	data = buscarPID(pid);

	if(data==NULL)
	{
		log_error("pid %d no encontrado en el cambio de contexto - programa no inicializado", pid);
		pid = 0;  // PID 0 (disponible)
	}

	// No se actuliza tabla de pagina del otro proceso asi qeu no es necesario actualizar swap con paginas que tienen byte modificado
	// TODO es necesario hacer un flush del pid en TLB ??? creo que no
	return pid;
}

void realizarAccionCPU(uint16_t unSocket){

	stMensajeIPC unMensaje;
	uint32_t pidActivo;
	stPosicion posR;
	stEscrituraPagina posW;

	/* Inicializo pidActivo */
	pidActivo = 0;   /* pid == 0 , CPU libre */

	while(1){

		if(!recibirHeaderIPC(unSocket, &unMensaje.header)){
			log_error("Thread[ID] No se recibe respuesta del CPU Socket[%d], Ultimo Pid[%d]: %d", unSocket, pidActivo);
			//liberarHeaderIPC(unMensaje->header);
			close(unSocket);
			return;//pthread_exit(NULL);
		}

		switch(unMensaje.header.tipo){

		case READ_BTYES_PAGE:

			recv(unSocket, &(posR.pagina), sizeof(uint16_t),0);
			recv(unSocket, &(posR.offset), sizeof(uint16_t),0);
			recv(unSocket, &(posR.size), sizeof(uint16_t),0);

			leerBytes(&posR, pidActivo, unSocket);

			break;

		case WRITE_BYTES_PAGE:

			recv(unSocket, &(posW.nroPagina), sizeof(uint16_t),0);
			recv(unSocket, &(posW.offset), sizeof(uint16_t),0);
			recv(unSocket, &(posW.tamanio), sizeof(uint16_t),0);
			recv(unSocket, posW.buffer, posR.size,0);
			//posW =(stEscrituraPagina*)(unMensaje.contenido);

			escribirBytes(&posW, pidActivo, unSocket);

			break;

		case FINPROGRAMA:

			finalizarPrograma(pidActivo, unSocket);
			break;

		case CAMBIOCONTEXTO:

			// ToDO: Del lado del CPU esta definido int (4 bytes), pero se podria poner como
			//       int32_t para asegurar los 4 bytes

			/* recibo el contenido */
			unMensaje.contenido = malloc(unMensaje.header.largo);
			recibirContenido(unSocket, (char*) unMensaje.contenido, unMensaje.header.largo);

			pidActivo = cambiarContexto(&unMensaje);

			if(0 == pidActivo)
			{
				log_error("Thread[ID]: PID erroneo, UMC espera por el nuevo cambio de contexto");
				// TodO: Responder al CPU , Thread libre.
				free(unMensaje.contenido);
				break;
			}
			// Todo: Confirmar al CPU con PID.
			log_info("Thread[ID]: Cambio de PID[%d]", pidActivo);
			free(unMensaje.contenido);
			break;

		default:
			log_info("Se recibio una peticion con un codigo desconocido...%i\n", unMensaje.header.tipo);
			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
			break;

		}/*Cierro switch(unMensaje.header.tipo)*/
	}/*Cierro while*/
}
