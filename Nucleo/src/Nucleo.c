/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Jose Maria Suarez
 Version     : 0.1
 Description : Elestac - Nucleo
 ============================================================================
 */

#include "includes/Nucleo.h"
#include "includes/consumidor_cpu.h"
#include "includes/nucleo_config.h"
#include "includes/planificador.h"
#include "tests/test_nucleo.h"

/*
 ============================================================================
 Estructuras y definiciones
 ============================================================================
 */

pthread_mutex_t mutex_estado = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_listaBlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_lista_consolas = PTHREAD_MUTEX_INITIALIZER;

fd_set fds_master; /* Lista de todos mis sockets.*/
fd_set read_fds; /* Sublista de fds_master.*/
stEstado elEstadoActual; /* Estado con toda la configuracion del Nucleo*/

static t_list *listaBlock; /* Lista de todos los PCB bloqueados*/
static t_list *lista_consolas; /* Lista de todas las consolas conectadas*/
/*
 ============================================================================
 Funciones
 ============================================================================
 */
/* Devuelve el pid de la desconectada y la saca de la lista, 0 en caso de no hallarla */
void agregar_consola(int unSocket, uint32_t pid){
	stConsola *consola = malloc(sizeof(stConsola));
	consola->pid = pid;
	consola->socket = unSocket;
	pthread_mutex_lock(&mutex_lista_consolas);
	list_add(lista_consolas, consola);
	pthread_mutex_unlock(&mutex_lista_consolas);
}

uint32_t borrar_consola(int unSocket) {
	stConsola *consola = NULL;
	int i = 0;
	pthread_mutex_lock(&mutex_lista_consolas);
	int size = list_size(lista_consolas);

	for (i = 0; i < size; i++) {
		consola = list_get(lista_consolas, i);
		if (consola->socket == unSocket) {
			list_remove(lista_consolas, i);
			pthread_mutex_unlock(&mutex_lista_consolas);
			return consola->pid;
		}
	}
	return 0;  // No encontrada
}


uint32_t buscar_consola(int pid) {
	stConsola *consola = NULL;
	int i = 0;
	pthread_mutex_lock(&mutex_lista_consolas);
	int size = list_size(lista_consolas);

	for (i = 0; i < size; i++) {
		consola = list_get(lista_consolas, i);
		if (consola->pid == pid) {
			pthread_mutex_unlock(&mutex_lista_consolas);
			return 1;
		}
	}
	pthread_mutex_unlock(&mutex_lista_consolas);
	return 0;  // No encontrada
}

/**
 * Busco el socket de la consola que me cargo el pid pasado como parametro
 *
 */
stConsola * obtenerSocketConsolaPorPID(uint32_t pid) {
	stConsola *consola = NULL;
	int i = 0;
	pthread_mutex_lock(&mutex_lista_consolas);
	int size = list_size(lista_consolas);

	for (i = 0; i < size; i++) {
		consola = list_get(lista_consolas, i);
		if (consola->pid == pid) {
			list_remove(lista_consolas, i); //Elimino el elemento de la lista de consolas
			pthread_mutex_unlock(&mutex_lista_consolas);
			return consola;
		}
	}
	pthread_mutex_unlock(&mutex_lista_consolas);
	return NULL;  // No encontrada
}

void consola_crear_lista() {
	lista_consolas = list_create();
}

void consola_destruir_lista() {
	list_destroy(lista_consolas);
}

void agregar_master(int un_socket, int maximo_ant) {
	FD_SET(un_socket, &(fds_master));
	if (un_socket > obtenerEstadoActual().fdMax) {
		maximo_ant = elEstadoActual.fdMax;
		elEstadoActual.fdMax = un_socket;
	}
}

void quitar_master(int un_socket, int maximo_ant) {
	FD_CLR(un_socket, &fds_master);
	if (un_socket > elEstadoActual.fdMax) {
		maximo_ant = elEstadoActual.fdMax;
		elEstadoActual.fdMax = un_socket;
	}
}

int calcular_cantidad_paginas(int size_programa, int tamanio_paginas) {
	int cant = 0;
	if (size_programa % tamanio_paginas > 0)
		cant++;
	return ((int) (size_programa / tamanio_paginas) + cant);
}

stEstado obtenerEstadoActual() {
	stEstado unEstado;
	pthread_mutex_lock(&mutex_estado);
	unEstado = elEstadoActual;
	pthread_mutex_unlock(&mutex_estado);
	return unEstado;
}

stDispositivo *buscar_dispositivo_io(char *dispositivo_name) {
	stDispositivo *unDispositivo = NULL;
	/*Busqueda de dispositivo de I/O*/
	int _es_el_dispositivo(stDispositivo *d) {
		return string_equals_ignore_case(d->nombre, dispositivo_name);
	}
	unDispositivo = list_find(obtenerEstadoActual().dispositivos, (void*) _es_el_dispositivo);
	return unDispositivo;
}

void agregar_pcb_listaBlock(stPCB *unPCB) {
	pthread_mutex_lock(&mutex_listaBlock);
	list_add(listaBlock, unPCB);
	pthread_mutex_unlock(&mutex_listaBlock);
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
void inicializarThreadsDispositivos(stEstado* unEstado) {
	int i = 0;
	pthread_t unThread;
	for (i = 0; i < list_size(unEstado->dispositivos); ++i) {
		stDispositivo *unDispositivo = list_get(unEstado->dispositivos, i);
		if (pthread_create(&unThread, NULL, (void*) threadDispositivo, unDispositivo) != 0) {
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
		while (unDispositivo->numInq == 0)
			pthread_mutex_lock(&unDispositivo->empty);
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
		pthread_mutex_lock(&mutex_listaBlock);
		unPCB = list_remove_by_condition(listaBlock, (void*) _es_el_pcb);
		pthread_mutex_lock(&mutex_listaBlock);

		/*Ponemos en la cola de Ready para que lo vuelva a ejecutar un CPU*/
		ready_productor(unPCB);
		free(unaRafaga);
		log_info("PCB [PID - %d] BLOCK a READY\n", unPCB->pid);

	}
}

int inicializar_programa(stPCB *unPCB, char* unPrograma, int socket_umc) {

	stPageIni *unInicioUMC;
	t_paquete paquete;
	stHeaderIPC *unHeaderIPC;

	// Le indico a la UMC que inicializo el programa
	unHeaderIPC = nuevoHeaderIPC(INICIALIZAR_PROGRAMA);
	if (!enviarHeaderIPC(socket_umc, unHeaderIPC)) {
		liberarHeaderIPC(unHeaderIPC);
		close(socket_umc);
		return EXIT_FAILURE;
	}
	liberarHeaderIPC(unHeaderIPC);

	unInicioUMC = malloc(sizeof(stPageIni));
	unInicioUMC->processId = unPCB->pid;
	unInicioUMC->cantidadPaginas = unPCB->cantidadPaginas;
	unInicioUMC->programa = unPrograma;

	crear_paquete(&paquete, INICIALIZAR_PROGRAMA);
	serializar_inicializar_programa(&paquete, unInicioUMC);

	if (enviar_paquete(socket_umc, &paquete)) {
		log_info("No se pudo enviar paquete de inicio de programa para PID [%d]", unPCB->pid);
		close(socket_umc);
		return EXIT_FAILURE;
	}
	free_paquete(&paquete);
	free(unInicioUMC);

	unHeaderIPC = nuevoHeaderIPC(ERROR);	// por default reservo memoria con tipo ERROR
	if (!recibirHeaderIPC(socket_umc, unHeaderIPC)) {
		log_error("UMC handshake error - No se pudo recibir mensaje de confirmacion");
		liberarHeaderIPC(unHeaderIPC);
		close(socket_umc);
		return EXIT_FAILURE;
	}
	liberarHeaderIPC(unHeaderIPC);

	if (unHeaderIPC->tipo == OK) {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
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
	uint32_t pid_desconectado = 0;
	int unCliente = 0, maximoAnterior = 0, unSocket, agregarSock;
	struct sockaddr addressAceptado;

	/*Inicializacion de las colas del planificador*/
	inicializar_pidCounter();
	inicializar_cola_ready();
	listaBlock = list_create();
	consola_crear_lista();

	/*Logger*/
	t_log* logger = log_create(temp_file, "NUCLEO", -1, LOG_LEVEL_INFO);

	log_info("Arrancando el Nucleo");

	if (!elEstadoActual.path_conf) {
		log_error("Falta el parametro de configuracion");
		exit(-1);
	}

	// Ejecuto las pruebas
	if (argv[2])
		if (strcmp(argv[2], "--cunit") == 0)
			test_unit_nucleo();

	/*Carga del archivo de configuracion*/
	log_info("Obteniendo configuracion...");
	if (loadInfo(&elEstadoActual, 0)) {
		log_info("Error");
		exit(-2);
	}
	log_info("OK");

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
	log_info("Se establecio conexion con el socket de escucha...");

	/*Seteamos el maximo socket*/
	elEstadoActual.fdMax = elEstadoActual.sockEscuchador;

	/*Conexion con el proceso UMC*/
	log_info("Estableciendo conexion con la UMC...");
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
		} else {
			if (recibirConfigUMC(elEstadoActual.sockUmc, &UMCConfig)) {
				log_error("UMC error - No se pudo recibir la configuracion");
				close(unCliente);
				exit(-2);
			}
			log_info("----------------------------\n");
			agregar_master(elEstadoActual.sockUmc,maximoAnterior);
			log_info("Paginas por proceso:[%d]", UMCConfig.paginasXProceso);
			log_info("Tamanio de pagina:[%d]", UMCConfig.tamanioPagina);
			log_info("----------------------------\n");

			elEstadoActual.tamanio_paginas = UMCConfig.tamanioPagina;
		}
		liberarHeaderIPC(unHeaderIPC);
	} else {
		log_error("No se pudo conectar a la UMC");
		elEstadoActual.salir = 1;
	}

	/*Ciclo Principal del Nucleo*/
	log_info(".............................................................................\n");
	log_info("..............................Esperando Conexion.............................\n\n");
	fflush(stdout);

	while (elEstadoActual.salir == 0) {
		read_fds = fds_master;

		if (seleccionar(elEstadoActual.fdMax, &read_fds, 0) == -1) {
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
						log_info("Nueva consola conectada\n");
						agregarSock = 1;
						/*Agrego el socket conectado a la lista Master*/
						if (agregarSock == 1) {
							agregar_master(unCliente, maximoAnterior);
							agregarSock = 0;
						}
						/* Recibo Programa */
						if (!recibirMensajeIPC(unCliente, &unMensaje)) {
							log_error("No se puede recibir el programa a procesar");
							break;/*Sale del switch*/
						} else {
							if (unMensaje.header.tipo == SENDANSISOP) {
								/*metadata_desde_literal hace un malloc adentro*/
								int cantidadDePaginasCodigo = calcular_cantidad_paginas(unMensaje.header.largo, UMCConfig.tamanioPagina);
								/***Creacion del PCB***/
								unPCB = crear_pcb(unCliente, cantidadDePaginasCodigo, elEstadoActual.stackSize, &unMensaje);
								if (unPCB == NULL) {
									log_info("Error al crear el PCB... se cierra la consola\n");
									quitar_master(unCliente, maximoAnterior);
									close(unCliente);
									break;/*Sale del switch*/
								}
								if (inicializar_programa(unPCB, unMensaje.contenido, elEstadoActual.sockUmc) == EXIT_FAILURE) {
									log_error("No se pudo inicializar el programa");
									/*TODO: Liberar toda la memoria del pcb!*/
									quitar_master(unCliente, maximoAnterior);
									close(unCliente);
									break;/*Sale del switch*/
								}

								/* Inicializada la consola la agrego a consolas activas */
								agregar_consola(unCliente, unPCB->pid);

								/*Cuando se usa mensajeIPC liberar el contenido*/
								free(unMensaje.contenido);
								ready_productor(unPCB);
								log_info("PCB [PID - %d] NEW a READY\n", unPCB->pid);
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
						log_info("Nuevo CPU conectado, lanzamiento de hilo...");
						if (pthread_create(&p_threadCpu, NULL, (void*) consumidor_cpu, (void*) unCliente) != 0) {
							log_error("No se pudo lanzar el hilo correspondiente al cpu conectado");
							close(unCliente);
							break;/*Sale del switch*/
						}
						log_info("OK\n");
						fflush(stdout);
						break;
					default:
						break;
					}
					if (unHeaderIPC != NULL) {
						liberarHeaderIPC(unHeaderIPC);
					}
				} else {
					/*Conexion existente*/
					log_info("Recibi otro evento de un cliente ya conectado");
					if (!recibirMensajeIPC(unSocket, &unMensaje)) {
						log_info("Desconexion detectada");

						// Desconexion de una consola
   						pid_desconectado = borrar_consola(unSocket);
						if (pid_desconectado != 0) {
							log_info("Se desconecto la consola asignada al PCB [PID - %d]", pid_desconectado);
							eliminar_pcb_ready(pid_desconectado);
						}

						// Desconexion de la UMC
						if (unSocket == elEstadoActual.sockUmc) {
							log_info("Se perdio conexion con la UMC...");
							elEstadoActual.salir = 1;
							cerrarSockets(&elEstadoActual);
						}

						if (unSocket == elEstadoActual.sockEscuchador) {
							log_info("Se perdio conexion...");
						}
						/*Saco el socket de la lista Master*/
						quitar_master(unSocket, maximoAnterior);
						fflush(stdout);
					}
				}

			}

		}
	}
	destruir_planificador();
	destruir_lista_dispositivos(&elEstadoActual);
	consola_destruir_lista(&elEstadoActual);
	cerrarSockets(&elEstadoActual);
	finalizarSistema(&unMensaje, unSocket, &elEstadoActual);
	log_info("NUCLEO: Fin del programa");
	return 0;
}
