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

int pidCounter;

/*
 ============================================================================
 Funciones
 ============================================================================
 */
int pid_incrementer() {
	pidCounter = pidCounter + 1;
	return pidCounter;
}

int calcular_cantidad_paginas(int size_programa,int tamanio_paginas){
	int cant=0;
	if(size_programa%tamanio_paginas > 0)
		cant++;
	return ((int)(size_programa/tamanio_paginas) + cant);
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
void threadDispositivo(stEstado* info, stDispositivo* unDispositivo) {
	int error = 0;
	t_queue *colaRafaga;
	stRafaga *unaRafaga;
	stPCB *unPCB;
	int unidad;

	colaRafaga = unDispositivo->rafagas;

	while (!error) {
		if (queue_size(colaRafaga) == 0) {
			continue;
		}

		unaRafaga = queue_pop(colaRafaga);

		for (unidad = 0; unidad < unaRafaga->unidades; ++unidad) {
			usleep(atoi(unDispositivo->retardo));
		}

		/*Busqueda del pcb en la lista de pcb bloqueados*/
		int _es_el_pcb(stPCB *p) {
			return p->pid == unaRafaga->pid;
		}

		unPCB = list_remove_by_condition(listaBlock, (void*) _es_el_pcb);

		/*Ponemos en la cola de Ready para que lo vuelva a ejecutar un CPU*/
//		pthread_mutex_lock(&mutexColaReady);
//		queue_push(colaReady, unPCB);
//		pthread_mutex_unlock(&mutexColaReady);
		log_info("PCB[%d] vuelve a ingresar a la cola de Ready \n", unPCB->pid);

	}
}

/*
 ============================================================================
 Funcion principal
 ============================================================================
 */
int main(int argc, char *argv[]) {
	stHeaderIPC *stHeaderIPC;
	stEstado elEstadoActual;
	stMensajeIPC unMensaje;
	t_metadata_program *unPrograma;
	stPCB *unPCB;
	stIndiceStack *unIndiceStack;
	t_UMCConfig UMCConfig;
	pidCounter = 0;

	int metadataSize = 0;

	struct thread_cpu_arg_struct cpu_arg_struct;

	char* temp_file = "nucleo.log";

	/*Inicializacion de las colas del planificador*/
	colaReady = queue_create();
	listaBlock = list_create();

	/*Inicializacion de las listas del semaforos y variables compartidas*/
	listaSem = list_create();
	listaSharedVars = list_create();

	int unCliente = 0, unSocket;
	int maximoAnterior = 0;
	struct sockaddr addressAceptado;

	int agregarSock;

	pthread_t p_thread, p_threadCpu;

	printf("----------------------------------Elestac------------------------------------\n");
	printf("-----------------------------------Nucleo------------------------------------\n");
	printf("------------------------------------v0.1-------------------------------------\n\n");
	fflush(stdout);

	/*Logger*/
	t_log* logger = log_create(temp_file, "NUCLEO", -1, LOG_LEVEL_INFO);

	/*Carga del archivo de configuracion*/
	printf("Obteniendo configuracion...");
	loadInfo(&elEstadoActual, &listaSem, &listaSharedVars);
	printf("OK\n");
	log_info("Configuracion cargada satisfactoriamente...");

	/*Se lanza el thread para identificar cambios en el archivo de configuracion*/
	pthread_create(&p_thread, NULL, (void*) &monitor_configuracion, (void*) &elEstadoActual);

	/*Inicializacion de listas de socket*/
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/*Inicializacion de socket de escucha*/
	elEstadoActual.salir = 0;
	elEstadoActual.sockEscuchador = -1;

	/*Iniciando escucha en el socket escuchador de Consola*/
	elEstadoActual.sockEscuchador = escuchar(elEstadoActual.miPuerto);
	FD_SET(elEstadoActual.sockEscuchador, &(fds_master));
	log_info("Se establecio conexion con el socket de escucha...");

	/*Seteamos el maximo socket*/
	elEstadoActual.fdMax = elEstadoActual.sockEscuchador;

	/*Conexion con el proceso UMC*/
	printf("Estableciendo conexion con la UMC...\n");
	elEstadoActual.sockUmc = conectar(elEstadoActual.ipUmc, elEstadoActual.puertoUmc);

	if (elEstadoActual.sockUmc != -1) {
		FD_SET(elEstadoActual.sockUmc, &(fds_master));

		stHeaderIPC = nuevoHeaderIPC(ERROR);
		if (!recibirHeaderIPC(elEstadoActual.sockUmc, stHeaderIPC)) {
			log_error("UMC handshake error - No se pudo recibir mensaje de respuesta");
			liberarHeaderIPC(stHeaderIPC);
			/*TODO: Para la entrega si no es posible conectar la UMC hacer un exit*/
			close(unCliente);

		}

		if (stHeaderIPC->tipo == QUIENSOS) {
			stHeaderIPC = nuevoHeaderIPC(CONNECTNUCLEO);
			if (!enviarHeaderIPC(elEstadoActual.sockUmc, stHeaderIPC)) {
				log_error("UMC handshake error - No se pudo enviar mensaje de conexion");
				liberarHeaderIPC(stHeaderIPC);
				close(unCliente);
			}
		}

		stHeaderIPC = nuevoHeaderIPC(OK);
		if (!recibirHeaderIPC(elEstadoActual.sockUmc, stHeaderIPC)) {
			log_error("UMC handshake error - No se pudo recibir mensaje de confirmacion");
			liberarHeaderIPC(stHeaderIPC);
			close(unCliente);
		}

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

	} else {
		/*Cargamos datos dummy*/
		elEstadoActual.tamanio_paginas = UMCConfig.tamanioPagina;
		printf("----------------------------\n");
		printf("Tamanio de pagina dummy:[%d]\n", 1024);
		printf("----------------------------\n");

		log_error("UMC connection error - No se pudo establecer el enlace");
		/*TODO: Para la entrega si no es posible conectar la UMC hacer un exit*/
	}

	/*Ciclo Principal del Nucleo*/
	printf(".............................................................................\n");
	printf("..............................Esperando Conexion.............................\n\n");
	fflush(stdout);

	while (elEstadoActual.salir == 0) {
		read_fds = fds_master;

		if (seleccionar(elEstadoActual.fdMax, &read_fds, 1) == -1) {

			log_error("Select error - Error Preparando el Select");
			return 1;
		}

		for (unSocket = 0; unSocket <= elEstadoActual.fdMax; unSocket++) {

			if (FD_ISSET(unSocket, &read_fds)) {
				/*Nueva conexion*/
				if (unSocket == elEstadoActual.sockEscuchador) {
					unCliente = aceptar(elEstadoActual.sockEscuchador, &addressAceptado);

					log_info("Se recibe un pedido de conexion...");

					stHeaderIPC = nuevoHeaderIPC(QUIENSOS);
					if (!enviarHeaderIPC(unCliente, stHeaderIPC)) {

						log_error("Handshake error - No se puede enviar el mensaje de reconocimiento de cliente");

						liberarHeaderIPC(stHeaderIPC);
						close(unCliente);
						continue;

					}

					if (!recibirHeaderIPC(unCliente, stHeaderIPC)) {
						log_error("Handshake error - No se puede recibir el mensaje de reconocimiento de cliente");

						liberarHeaderIPC(stHeaderIPC);
						close(unCliente);
						continue;

					}

					/*Identifico quien se conecto y procedo*/
					switch (stHeaderIPC->tipo) {
					case CONNECTCONSOLA:

						stHeaderIPC = nuevoHeaderIPC(OK);
						if (!enviarHeaderIPC(unCliente, stHeaderIPC)) {
							log_error("Handshake Consola - No se puede recibir el mensaje de reconocimiento de cliente");
							liberarHeaderIPC(stHeaderIPC);
							close(unCliente);
							continue;
						}

						log_info("Nueva consola conectada");
						agregarSock = 1;

						/*Agrego el socket conectado a la lista Master*/
						if (agregarSock == 1) {
							FD_SET(unCliente, &(fds_master));
							log_info("Se agrega la consola conectada a la lista FDS_MASTER");
							if (unCliente > elEstadoActual.fdMax) {
								maximoAnterior = elEstadoActual.fdMax;
								elEstadoActual.fdMax = unCliente;
							}
							agregarSock = 0;
						}

						/* Recibo Programa */
						if (!recibirMensajeIPC(unCliente, &unMensaje)) {
							printf("Consola error - No se puede recibir el programa a procesar\n");
							log_error("No se puede recibir el programa a procesar");
							continue;
						} else {
							if (unMensaje.header.tipo == SENDANSISOP) {

								unPrograma = metadata_desde_literal(unMensaje.contenido);
								metadataSize = sizeof(t_metadata_program) + (sizeof(t_intructions) * unPrograma->instrucciones_size)
										+ (sizeof(char) * unPrograma->etiquetas_size);

								int cantidadDePaginasCodigo = calcular_cantidad_paginas(unMensaje.header.largo,UMCConfig.tamanioPagina);

								/***Creacion del PCB***/
								unPCB = (stPCB*) malloc(sizeof(stPCB));
								if (unPCB != NULL) {
									unPCB->pid = pid_incrementer();
									unPCB->pc = 0;
									unPCB->socketConsola = unCliente;
									unPCB->socketCPU = 0;
									unPCB->paginaInicioStack = cantidadDePaginasCodigo + 1;
									unPCB->cantidadPaginas = cantidadDePaginasCodigo + elEstadoActual.stackSize;
									unPCB->metadata_program = (t_metadata_program *) malloc(metadataSize);
									memcpy(unPCB->metadata_program, unPrograma, metadataSize);
									unPCB->stack = list_create();

								}
								/*Inicializo el stack con un elemento*/
								unIndiceStack = (stIndiceStack*) malloc(sizeof(stIndiceStack));
								if (unIndiceStack != NULL) {
									unIndiceStack->argumentos = list_create();
									unIndiceStack->pos = 0;
									unIndiceStack->variables = list_create();
									list_add(unPCB->stack,unIndiceStack);
								}

								if (inicializar_programa(unPCB->pid, unPCB->cantidadPaginas, unMensaje.contenido, elEstadoActual.sockUmc) == EXIT_FAILURE) {
									printf("UMC error - No se puede ejecutar el programa por falta de espacio\n");
									/*TODO: Liberar toda la memoria del pcb!*/
									FD_CLR(unCliente, &fds_master);
									close(unCliente);
									if (unCliente > elEstadoActual.fdMax) {
										maximoAnterior = elEstadoActual.fdMax;
										elEstadoActual.fdMax = unSocket;
									}
									continue;
								}

								//log_info("Ingresa PCB [%d] en estado NEW", unPCB->pid);
								ready_productor(&unPCB);
								printf("OK\n");
								fflush(stdout);
							}

						}
						break;

					case CONNECTCPU:
						stHeaderIPC = nuevoHeaderIPC(OK);
						if (!enviarHeaderIPC(unCliente, stHeaderIPC)) {
							liberarHeaderIPC(stHeaderIPC);
							printf("CPU error - No se pudo enviar confirmacion de recepcion\n");
							log_error("CPU error - No se pudo enviar confirmacion de recepcion");
							continue;
						}
						log_info("Nuevo CPU conectado");
						agregarSock = 1;
						/*Agrego el socket conectado A la lista Master*/
						if (agregarSock == 1) {
							FD_SET(unCliente, &(fds_master));
							if (unCliente > elEstadoActual.fdMax) {
								maximoAnterior = elEstadoActual.fdMax;
								elEstadoActual.fdMax = unCliente;
							}
							agregarSock = 0;
						}

						cpu_arg_struct.estado = &elEstadoActual;
						cpu_arg_struct.socketCpu = unCliente;

						printf("Lanzamiento de hilo dedicado al cpu...");

						if (pthread_create(&p_threadCpu, NULL, (void*) &consumidor_cpu, (void *) &cpu_arg_struct) != 0) {
							log_error("No se pudo lanzar el hilo correspondiente al cpu conectado");
							continue;
						}

						printf("OK\n");
						fflush(stdout);

						break;
					default:
						break;
					}
				} else {
					/*Conexion existente*/
					memset(unMensaje.contenido, '\0', LONGITUD_MAX_DE_CONTENIDO);
					if (!recibirMensajeIPC(unSocket, &unMensaje)) {
						if (unSocket == elEstadoActual.sockEscuchador) {
							printf("Se perdio conexion...\n ");
						}

						/*TODO: Sacar de la lista de cpu conectados si hay alguna desconexion.*/
						/*Saco el socket de la lista Master*/
						FD_CLR(unSocket, &fds_master);
						close(unSocket);

						if (unSocket > elEstadoActual.fdMax) {
							maximoAnterior = elEstadoActual.fdMax;
							elEstadoActual.fdMax = unSocket;

						}

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
	printf("\nNUCLEO: Fin del programa\n");
	return 0;
}
