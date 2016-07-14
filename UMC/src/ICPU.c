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

int leerBytes(void **buffer, stPosicion* posLogica, uint16_t pid){
	uint16_t frameBuscado = 0;
	uint16_t frameNuevo = 0;
	stRegistroTLB stTLB;

	/* si esta disponible cache*/
	if (estaActivadaTLB()== OK){
		buscarEnTLB(pid, posLogica->pagina, &frameBuscado);
	}
	// no hay TLB o es un TLB miss
	if(estaActivadaTLB()==ERROR || frameBuscado==0)
		buscarEnTabla(pid, posLogica->pagina, &frameBuscado);

	// leo el frame desde memoria si estan en tabla o TLB
	if(frameBuscado != 0){

		// acceder a memoria con el resultado encontrado en cache
		if(leerMemoria(buffer, frameBuscado, *posLogica))
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}

	if(frameBuscado == 0){

		ejecutarPageFault(pid, posLogica->pagina, &frameNuevo);

		// cargo en TLB la pagina obtenida ya que si esta activa no la encontro
		if(estaActivadaTLB()== OK){
			stTLB.pid = pid;
			stTLB.pagina = posLogica->pagina;
			stTLB.marco = frameNuevo;
			reemplazarValorTLB(stTLB);
		}

		if(frameNuevo!=0){

			// con la pagina obtenida separo los bytes que se pidieron leer
			if(leerMemoria(buffer, frameNuevo, *posLogica))
				return EXIT_FAILURE;

			return EXIT_SUCCESS;
		}else
			return EXIT_FAILURE;
	}

	return EXIT_FAILURE;
}

int escribirBytes(stEscrituraPagina* unaEscritura, uint16_t pid){
	stRegistroTP *registro;  // Si pagefault
	uint16_t frameBuscado = 0;
	uint16_t frameNuevo = 0;
	stRegistroTLB stTLB;

	/* si esta disponible cache*/
	if (estaActivadaTLB()== OK){
		buscarEnTLB(pid, unaEscritura->nroPagina, &frameBuscado);
	}
	// no hay TLB o es un TLB miss
	if(estaActivadaTLB()==ERROR || frameBuscado==0)
		buscarEnTabla(pid, unaEscritura->nroPagina, &frameBuscado);

	// leo el frame desde memoria si estan en tabla o TLB
	if(frameBuscado != 0){

		// en el marco obtenido indico posicion para escribir los bytes pedidos
		if(escribirMemoria(unaEscritura->buffer, frameBuscado, unaEscritura->offset, unaEscritura->tamanio))
			return EXIT_FAILURE;

		// acceder a memoria con el resultado encontrado en cache
		return EXIT_SUCCESS;
	}

	// Page fault Escritura
	if(frameBuscado == 0){

		if(ejecutarPageFault(pid, unaEscritura->nroPagina, &frameNuevo)){
			log_error("Pid [%d] Pagina[%d]: Error al ejecutar page fault.", pid, unaEscritura->nroPagina);
			return EXIT_FAILURE;
		}

		// cargo en TLB la pagina obtenida ya que si esta activa no la encontro
		if(estaActivadaTLB()== OK){
			stTLB.pid = pid;
			stTLB.pagina = unaEscritura->nroPagina;
			stTLB.marco = frameNuevo;
			reemplazarValorTLB(stTLB);
		}

		// en la pagina obtenida escribo los bytes que se pidieron
		if(escribirMemoria(unaEscritura->buffer, frameNuevo, unaEscritura->offset, unaEscritura->tamanio))
			return EXIT_FAILURE;

		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

int ejecutarPageFault(uint16_t pid, uint16_t pagina, uint16_t *pframeNuevo){
	uint16_t frameNuevo = 0;
	int presencias=0;
	stNodoListaTP *tablaPaginas;
	void *paginaLeidaSwap;

	// acceder a swap con las pagina que necesito
	paginaLeidaSwap = recibirPagina(pid, pagina);

	if(paginaLeidaSwap==NULL){
		log_info("Page Fault (Swap envia una pagina vacia)");
		paginaLeidaSwap = calloc(1,losParametros.frameSize);
		loguear_buffer(paginaLeidaSwap, losParametros.frameSize);
	}
	else{
		log_info("Page Fault");
		loguear_buffer(paginaLeidaSwap, losParametros.frameSize);
	}

	/* Obtiene Tabla de Paginas de PID */
	tablaPaginas = buscarPID(pid);
	if(tablaPaginas==NULL) // valido que exista
		return EXIT_FAILURE;

	/* Obtengo presencias en MP de la tabla de PID */
	presencias = obtenerPresenciasTabladePaginas(tablaPaginas);

	// - si es un nuevo proceso, tengo que reemplazarlo y no hay memoria-> rechazo el pedido.
	if(0 == presencias && 0 == hayMarcoLibre())
		return EXIT_FAILURE;

	// Reemplazo dentro de los marcos
	if(presencias < losParametros.frameByProc && 0 != hayMarcoLibre()){
		frameNuevo = obtenerMarcoLibre();
		agregarFrameATablaMarcos(frameNuevo, tablaPaginas, pagina);
	}
	// Nunca va a llegar al limite de marcos por proceso por falta de memoria
	else if(presencias < losParametros.frameByProc && 0 == hayMarcoLibre()){
		reemplazarValorTabla(&frameNuevo, tablaPaginas, pagina);
	}
	// Me las arreglo con los marcos que tengo (que no puede ser 0 )
	else if(presencias >= losParametros.frameByProc){
		if(0 == presencias )  // por las dudas avisarlo... (error de configuracion)
			return EXIT_FAILURE;
		reemplazarValorTabla(&frameNuevo, tablaPaginas, pagina);
	}

	if(0 == frameNuevo)
		return EXIT_FAILURE;

	// cargo en memoria la pagina obtenida
	escribirMemoria(paginaLeidaSwap, frameNuevo, 0, losParametros.frameSize);
	free(paginaLeidaSwap);

	// Devuelvo la referencia del frame nuevo que necesito
	*pframeNuevo = frameNuevo;

	return EXIT_SUCCESS;
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
	stHeaderIPC *unHeader;
	uint32_t pidActivo;
	stPosicion posR;
	void *buffer = NULL;
	stEscrituraPagina posW;

	/* Inicializo pidActivo */
	pidActivo = 0;   /* pid == 0 , CPU libre */

	while(1){

		if(!recibirHeaderIPC(unSocket, &unMensaje.header)){
			log_error("Thread[ID] No se recibe respuesta del CPU Socket[%d], Ultimo Pid[%d]: %d", unSocket, pidActivo);
			close(unSocket);
			return;//pthread_exit(NULL);
		}

		switch(unMensaje.header.tipo){

		case READ_BTYES_PAGE:

			recv(unSocket, &(posR.pagina), sizeof(uint16_t),0);
			recv(unSocket, &(posR.offset), sizeof(uint16_t),0);
			recv(unSocket, &(posR.size), sizeof(uint16_t),0);

			reservarPosicion((void*)&buffer, posR.size);
			if(leerBytes(&buffer, &posR, pidActivo)){
				unHeader = nuevoHeaderIPC(ERROR);
				unHeader->largo = 0;
				log_info("Error al leer bytes");
			}
			else{
				unHeader = nuevoHeaderIPC(OK);
				unHeader->largo = posR.size;
				log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
			}

			// Logueo lo que llego para escribir
			log_info("Leer bytes");
			loguear_buffer(buffer, posR.size);

			//envio la respuesta de la lectura a la CPU
			if(!enviarMensajeIPC(unSocket,unHeader,buffer)){
				log_error("No se pudo enviar el READ_BTYES_PAGE");
				return;
			}


			imprimirMemoriaPrincipal();

			break;

		case WRITE_BYTES_PAGE:

			recv(unSocket, &(posW.nroPagina), sizeof(uint16_t),0);
			recv(unSocket, &(posW.offset), sizeof(uint16_t),0);
			recv(unSocket, &(posW.tamanio), sizeof(uint16_t),0);
			posW.buffer = malloc(posW.tamanio);
			recv(unSocket, posW.buffer, posW.tamanio,0);

			// Logueo lo que llego para escribir
			log_info("Escribir bytes");
			loguear_buffer(posW.buffer, posW.tamanio);

			if(escribirBytes(&posW, pidActivo)){
				unHeader = nuevoHeaderIPC(ERROR);
				unHeader->largo = 0;
				log_info("Error al escribir bytes");
			}
			else{
				unHeader = nuevoHeaderIPC(OK);
				unHeader->largo = posW.tamanio;
				log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
			}
			limpiarEscrituraPagina(posW.buffer, &posW);

			//envio la respuesta de la Escritura a la CPU
			if(!enviarHeaderIPC(unSocket,unHeader)){
				log_error("No se pudo enviar el WRITE_BYTES_PAGE");
				return;
			}

			imprimirMemoriaPrincipal();

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

void limpiarPosicion(void *buffer, stPosicion *pPos){
	if(pPos){
		pPos->offset=0;
		pPos->pagina=0;
		pPos->size=0;
	}
	// liberado antes con liberar buffer
	free(buffer);
	buffer = NULL;
}

void limpiarEscrituraPagina(void *buffer, stEscrituraPagina *pPos){
	if(pPos){
		pPos->offset=0;
		pPos->nroPagina=0;
		pPos->offset=0;
	}
	// liberado antes con liberar buffer
	free(buffer);
	buffer = NULL;
}

void reservarPosicion(void **buffer, uint16_t size){
	*buffer = malloc(size);
	strcpy(*buffer, "puis");
}

void loguear_buffer(void *buffer, uint16_t size){
	char *buffer_log = malloc(size + 1);
	memcpy(buffer_log, buffer, size);
	buffer_log[size]='\0';
	log_info("Buffer[%s]\n", buffer_log);
	free(buffer_log);
}
