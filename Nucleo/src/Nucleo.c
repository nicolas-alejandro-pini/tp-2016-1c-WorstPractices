/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Jose Maria Suarez
 Version     : 0.1
 Description : Elestac - Nucleo
 ============================================================================
 */

#include "includes/Nucleo.h"
#include "includes/servicio_memoria.h"
#include "includes/consumidor_cpu.h"
#include "includes/nucleo_config.h"
#include "includes/planificador.h"

/*
 ============================================================================
 Estructuras y definiciones
 ============================================================================
 */

pthread_mutex_t mutex_estado = PTHREAD_MUTEX_INITIALIZER;
fd_set fds_master; /* Lista de todos mis sockets.*/
fd_set read_fds; /* Sublista de fds_master.*/
stEstado elEstadoActual; /*Estado con toda la configuracion del Nucleo*/

/*
 ============================================================================
 Funciones
 ============================================================================
 */
void agregar_master(int un_socket, int maximo_ant) {
	FD_SET(un_socket, &(fds_master));
	if (un_socket > obtenerEstadoActual().fdMax) {
		maximo_ant = elEstadoActual.fdMax;
		elEstadoActual.fdMax = un_socket;
	}
}

void quitar_master(int un_socket, int maximo_ant){
	FD_CLR(un_socket, &fds_master);
	if (un_socket > elEstadoActual.fdMax) {
		maximo_ant = elEstadoActual.fdMax;
		elEstadoActual.fdMax = un_socket;
	}
}

int calcular_cantidad_paginas(int size_programa,int tamanio_paginas){
	int cant=0;
	if(size_programa%tamanio_paginas > 0)
		cant++;
	return ((int)(size_programa/tamanio_paginas) + cant);
}

stEstado obtenerEstadoActual(){
	stEstado unEstado;
	pthread_mutex_lock(&mutex_estado);
	unEstado = elEstadoActual;
	pthread_mutex_unlock(&mutex_estado);
	return unEstado;
}
void cerrarSockets(stEstado *elEstadoActual) {
	int unSocket;
	for (unSocket = 3; unSocket <= elEstadoActual->fdMax; unSocket++)
		if (FD_ISSET(unSocket, &(fds_master)))
			close(unSocket);

	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));
}
void finalizarSistema(stMensajeIPC *unMensaje, int unSocket, stEstado *unEstado) {
	unEstado->salir = 1;
	unMensaje->header.tipo = -1;
}
void inicializarThreadsDispositivos(stEstado* unEstado){
	int i = 0;
	pthread_t unThread;
	for (i = 0; i < list_size(unEstado->dispositivos); ++i) {
		stDispositivo *unDispositivo = list_get(unEstado->dispositivos, i);
		if (pthread_create(&unThread, NULL, (void*)threadDispositivo, unDispositivo) != 0) {
			log_error("No se pudo lanzar el hilo correspondiente al cpu conectado");
			continue;
		}
	}
}
void threadDispositivo(stDispositivo* unDispositivo) {
	int error = 0;
	t_queue *colaRafaga;
	stRafaga *unaRafaga;
	stPCB *unPCB;
	int unidad;

	colaRafaga = unDispositivo->rafagas;

	while (!error) {
		while (unDispositivo->numInq == 0) pthread_mutex_lock(&unDispositivo->empty);
		pthread_mutex_lock(&unDispositivo->mutex);		// Se lockea el acceso a la cola
		unaRafaga = queue_pop(colaRafaga);
		unDispositivo->numInq--;
		pthread_mutex_unlock(&unDispositivo->mutex);	// Se desbloquea el acceso a la cola

		for (unidad = 0; unidad < unaRafaga->unidades; ++unidad) {
			usleep(atoi(unDispositivo->retardo));
		}
		/*Busqueda del pcb en la lista de pcb bloqueados*/
		int _es_el_pcb(stPCB *p) {
			return p->pid == unaRafaga->pid;
		}
		unPCB = list_remove_by_condition(listaBlock, (void*) _es_el_pcb);
		/*Ponemos en la cola de Ready para que lo vuelva a ejecutar un CPU*/
		ready_productor(unPCB);
		free(unaRafaga);
		printf("PCB [PID - %d] BLOCK a READY\n", unPCB->pid);

	}
}

/*
 ============================================================================
 Funcion principal
 ============================================================================
 */
int main(int argc, char *argv[]) {
	stHeaderIPC *unHeaderIPC = NULL, *stHeaderSwitch = NULL;
	stMensajeIPC unMensaje;
	stPCB *unPCB = NULL;
	t_UMCConfig UMCConfig;
	pthread_t p_thread, p_threadCpu;
	char* temp_file = "nucleo.log";
	elEstadoActual.path_conf = argv[1];
	int unCliente = 0,maximoAnterior = 0, unSocket, agregarSock;
	struct sockaddr addressAceptado;


	/*Inicializacion de las colas del planificador*/
	colaReady = queue_create();
	listaBlock = list_create();

	/*Inicializacion de las listas del semaforos y variables compartidas*/
	listaSem = list_create();
	listaSharedVars = list_create();

	printf("----------------------------------Elestac------------------------------------\n");
	printf("-----------------------------------Nucleo------------------------------------\n");
	printf("------------------------------------v0.1-------------------------------------\n\n");
	fflush(stdout);

	/*Logger*/
	t_log* logger = log_create(temp_file, "NUCLEO", -1, LOG_LEVEL_INFO);

	if(!elEstadoActual.path_conf)
	{
		log_error("Falta el parametro de configuracion");
		exit(-1);
	}

	/*Carga del archivo de configuracion*/
	printf("Obteniendo configuracion...");
	if(loadInfo(&elEstadoActual, &listaSem, &listaSharedVars)){
		printf("Error");
		exit(-2);
	}
	printf("OK\n");

//	log_info("Configuracion cargada satisfactoriamente...\n");

	/*Se lanza el thread para identificar cambios en el archivo de configuracion*/
	pthread_create(&p_thread, NULL, (void*) &monitor_configuracion, (void*) &elEstadoActual);

	inicializarThreadsDispositivos(&elEstadoActual);

	/*Inicializacion de listas de socket*/
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/*Inicializacion de socket de escucha*/
	elEstadoActual.salir = 0;
	elEstadoActual.sockEscuchador = -1;

	/*Iniciando escucha en el socket escuchador de Consola*/
	elEstadoActual.sockEscuchador = escuchar(elEstadoActual.miPuerto);
	FD_SET(elEstadoActual.sockEscuchador, &(fds_master));
	printf("Se establecio conexion con el socket de escucha...\n");

	/*Seteamos el maximo socket*/
	elEstadoActual.fdMax = elEstadoActual.sockEscuchador;

	/*Conexion con el proceso UMC*/
	printf("Estableciendo conexion con la UMC...\n");
	elEstadoActual.sockUmc = conectar(elEstadoActual.ipUmc, elEstadoActual.puertoUmc);

	if (elEstadoActual.sockUmc != -1) {
		FD_SET(elEstadoActual.sockUmc, &(fds_master));

		unHeaderIPC = nuevoHeaderIPC(ERROR);
		if (!recibirHeaderIPC(elEstadoActual.sockUmc, unHeaderIPC)) {
			log_error("UMC handshake error - No se pudo recibir mensaje de respuesta");
			log_error("No se pudo conectar a la UMC");
			elEstadoActual.salir = 1;
		}
		if (unHeaderIPC->tipo == QUIENSOS) {
			unHeaderIPC = nuevoHeaderIPC(CONNECTNUCLEO);
			if (!enviarHeaderIPC(elEstadoActual.sockUmc, unHeaderIPC)) {
				log_error("UMC handshake error - No se pudo enviar mensaje de conexion");
				log_error("No se pudo conectar a la UMC");
				elEstadoActual.salir = 1;
			}
		}
		liberarHeaderIPC(unHeaderIPC);
		unHeaderIPC = nuevoHeaderIPC(OK);
		if (!recibirHeaderIPC(elEstadoActual.sockUmc, unHeaderIPC)) {
			log_error("UMC handshake error - No se pudo recibir mensaje de confirmacion");
			log_error("No se pudo conectar a la UMC");
			elEstadoActual.salir = 1;
		}else{
			if (recibirConfigUMC(elEstadoActual.sockUmc, &UMCConfig)) {
				log_error("UMC error - No se pudo recibir la configuracion");
				close(unCliente);
				exit(-2);
			}
			printf("----------------------------\n");
			printf("Paginas por proceso:[%d]\n", UMCConfig.paginasXProceso);
			printf("Tamanio de pagina:[%d]\n", UMCConfig.tamanioPagina);
			printf("----------------------------\n");

			elEstadoActual.tamanio_paginas = UMCConfig.tamanioPagina;
		}
		liberarHeaderIPC(unHeaderIPC);
	} else {
		log_error("No se pudo conectar a la UMC");
		elEstadoActual.salir = 1;
	}

	/*Ciclo Principal del Nucleo*/
	printf(".............................................................................\n");
	printf("..............................Esperando Conexion.............................\n\n");
	fflush(stdout);

	while (elEstadoActual.salir == 0) {
		read_fds = fds_master;

		if (seleccionar(elEstadoActual.fdMax, &read_fds, 1) == -1) {
			log_error("Error Preparando el Select");
			break;
		}

		for (unSocket = 0; unSocket <= elEstadoActual.fdMax; unSocket++) {

			if (FD_ISSET(unSocket, &read_fds)) {
				/*Nueva conexion*/
				if (unSocket == elEstadoActual.sockEscuchador) {
					unCliente = aceptar(elEstadoActual.sockEscuchador, &addressAceptado);
					unHeaderIPC = nuevoHeaderIPC(QUIENSOS);
					if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
						log_error("Cliente Handshake error - No se puede enviar el mensaje QUIENSOS");
						liberarHeaderIPC(unHeaderIPC);
						close(unCliente);
						break;/*Sale del for*/
					}
					liberarHeaderIPC(unHeaderIPC);
					unHeaderIPC = nuevoHeaderIPC(ERROR);
					if (!recibirHeaderIPC(unCliente, unHeaderIPC)) {
						log_error("Cliente Handshake error - No se puede recibir el mensaje");
						liberarHeaderIPC(unHeaderIPC);
						close(unCliente);
						break;/*Sale del for*/
					}

					/*Identifico quien se conecto y procedo*/
					switch (unHeaderIPC->tipo) {
					case CONNECTCONSOLA:
						stHeaderSwitch = nuevoHeaderIPC(OK);
						if (!enviarHeaderIPC(unCliente, stHeaderSwitch)) {
							log_error("Handshake Consola - No se pudo enviar el OK");
							liberarHeaderIPC(stHeaderSwitch);
							close(unCliente);
							break;/*Sale del switch*/
						}
						liberarHeaderIPC(stHeaderSwitch);
						printf("Nueva consola conectada\n");
						agregarSock = 1;
						/*Agrego el socket conectado a la lista Master*/
						if (agregarSock == 1) {
							agregar_master(unCliente,maximoAnterior);
							agregarSock = 0;
						}
						/* Recibo Programa */
						if (!recibirMensajeIPC(unCliente, &unMensaje)) {
							log_error("No se puede recibir el programa a procesar");
							break;/*Sale del switch*/
						} else {
							if (unMensaje.header.tipo == SENDANSISOP) {
								/*metadata_desde_literal hace un malloc adentro*/
								int cantidadDePaginasCodigo = calcular_cantidad_paginas(unMensaje.header.largo,UMCConfig.tamanioPagina);
								/***Creacion del PCB***/
								unPCB = crear_pcb(unCliente,cantidadDePaginasCodigo,elEstadoActual.stackSize,&unMensaje);
								if(unPCB==NULL){
									printf("Error al crear el PCB... se cierra la consola\n");
									quitar_master(unCliente,maximoAnterior);
									close(unCliente);
									break;/*Sale del switch*/
								}
								if (inicializar_programa(unPCB, unMensaje.contenido, elEstadoActual.sockUmc) == EXIT_FAILURE) {
									printf("No se pudo inicializar el programa\n");
									/*TODO: Liberar toda la memoria del pcb!*/
									quitar_master(unCliente,maximoAnterior);
									close(unCliente);
									break;/*Sale del switch*/
								}
								/*Cuando se usa mensajeIPC liberar el contenido*/
								free(unMensaje.contenido);
								ready_productor(unPCB);
								printf("PCB [PID - %d] NEW a READY\n", unPCB->pid);
								fflush(stdout);
							}

						}
						break;

					case CONNECTCPU:
						stHeaderSwitch = nuevoHeaderIPC(OK);
						if (!enviarHeaderIPC(unCliente, stHeaderSwitch)) {
							liberarHeaderIPC(stHeaderSwitch);
							log_error("CPU error - No se pudo enviar OK");
							close(unCliente);
							break;/*Sale del switch*/
						}
						liberarHeaderIPC(stHeaderSwitch);
						printf("Nuevo CPU conectado, lanzamiento de hilo...");
						if (pthread_create(&p_threadCpu, NULL, (void*)consumidor_cpu, unCliente) != 0) {
							log_error("No se pudo lanzar el hilo correspondiente al cpu conectado");
							close(unCliente);
							break;/*Sale del switch*/
						}
						printf("OK\n");
						fflush(stdout);
						break;
					default:
						break;
					}
					if(unHeaderIPC!=NULL){
						liberarHeaderIPC(unHeaderIPC);
					}
				} else {
					/*Conexion existente*/
					memset(unMensaje.contenido, '\0', LONGITUD_MAX_DE_CONTENIDO);
					if (!recibirMensajeIPC(unSocket, &unMensaje)) {
						if (unSocket == elEstadoActual.sockEscuchador) {
							printf("Se perdio conexion...\n ");
						}
						/*Saco el socket de la lista Master*/
						quitar_master(unSocket,maximoAnterior);
						fflush(stdout);
					} else {
						/*Recibo con mensaje de conexion existente*/

					}

				}

			}

		}
	}
	cerrarSockets(&elEstadoActual);
	finalizarSistema(&unMensaje, unSocket, &elEstadoActual);
	printf("NUCLEO: Fin del programa\n");
	return 0;
}
