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

		registro = ejecutarPageFault(pid, unaEscritura->nroPagina, &frameNuevo);

		// No se puede ejecutar pageFaul, falta memoria
		if(registro == NULL)
			return EXIT_SUCCESS;
		else
		{   // Modifica la tabla de paginas (deberia hacerlo en memoria
			registro->bitModificado=1;
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

stRegistroTP* ejecutarPageFault(uint16_t pid, uint16_t pagina, uint16_t *frameNuevo){
	uint16_t marco;
	stRegistroTP regTP, *registro;
	void *paginaLeidaSwap;

	// acceder a swap con las pagina que necesito
	// TODO Mismo socket , deberia ser mutex?
	paginaLeidaSwap = recibirPagina(pid, pagina);

	if(paginaLeidaSwap==NULL)
		return NULL;

	// cargo en Tabla la pagina obtenida aplicando algoritmo de reemplazo de ser necesario
	regTP.bit2ndChance=0;
	regTP.bitModificado=0;
	regTP.bitPresencia=1;

	marco = obtenerMarcoLibre();
	if(marco == 0)
	{
		registro = reemplazarValorTabla(pid, pagina, regTP, REEMPLAZAR_MARCO);
		*frameNuevo = registro->marco;
	}
	else
	{
		regTP.marco = marco;
		*frameNuevo = marco;
		registro = reemplazarValorTabla(pid, pagina, regTP, NULL);
	}

	if (registro==NULL)
		// Se rechaza por no haber memoria
		return registro;

	// cargo en memoria la pagina obtenida
	escribirMemoria(paginaLeidaSwap, *frameNuevo, 0, losParametros.frameSize);

	free(paginaLeidaSwap);

	return registro;
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

			if(posR.size > 0)
			{
				reservarPosicion((void*)&buffer, posR.size);
				leerBytes(&buffer, &posR, pidActivo);

				//envio la respuesta de la lectura a la CPU
				unHeader = nuevoHeaderIPC(OK);
				unHeader->largo = posR.size;
				if(!enviarMensajeIPC(unSocket,unHeader,buffer)){
					log_error("No se pudo enviar el MensajeIPC");
					return;
				}
			}

			break;

		case WRITE_BYTES_PAGE:

			recv(unSocket, &(posW.nroPagina), sizeof(uint16_t),0);
			recv(unSocket, &(posW.offset), sizeof(uint16_t),0);
			recv(unSocket, &(posW.tamanio), sizeof(uint16_t),0);
			recv(unSocket, posW.buffer, posW.tamanio,0);
			//posW =(stEscrituraPagina*)(unMensaje.contenido);
			//envio la respuesta de la Escritura a la CPU
//			if(!enviarHeaderIPC(socketCPU,nuevoHeaderIPC(OK))){
//				log_error("No se pudo enviar el MensajeIPC");
//				return;
//			}
			//envio la respuesta de la lectura a la CPU
//			if(!enviarHeaderIPC(socketCPU,nuevoHeaderIPC(ERROR))){
//				log_error("No se pudo enviar el MensajeIPC");
//				return;
//			}
			//log_info("Se pidio leer pagina %d offset %d tamaño %d");
			//escribirBytes(&posW, pidActivo, unSocket);

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
