/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Jose Maria Suarez
 Version     : 0.1
 Description : Elestac - Nucleo
 ============================================================================
 */

#include "Nucleo.h"

/*
 ============================================================================
 Estructuras y definiciones
 ============================================================================
 */

int pidIncrementer;

/* Listas globales */
fd_set fds_master; /* Lista de todos mis sockets.*/
fd_set read_fds; /* Sublista de fds_master.*/

t_queue *colaReady; /*Cola de todos los PCB listos para ejecutar*/
t_queue *colaExit; /*Cola de todos los PCB listos para liberar*/
t_queue *colaBlock; /*Cola de todos los PCB listos para liberar*/


pthread_mutex_t mutexColaReady;

/*
 ============================================================================
 Funciones
 ============================================================================
 */
void loadInfo(stEstado* info) {

	t_config* miConf = config_create(CFGFILE); /*Estructura de configuracion*/
	info->dispositivos = list_create();
	info->semaforos = list_create();
	info->sharedVars = list_create();

	if (miConf == NULL) {
		log_error("Error iniciando la configuracion...\n");
		return exit(-2);;
	}

	if (config_has_property(miConf, "IP")) {
		info->miIP = config_get_string_value(miConf, "IP");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "IP");
		exit(-2);
	}

	if (config_has_property(miConf, "PUERTO")) {
		info->miPuerto = config_get_int_value(miConf, "PUERTO");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "PUERTO");
		exit(-2);
	}

	if (config_has_property(miConf, "IP_UMC")) {
		info->ipUmc = config_get_string_value(miConf, "IP_UMC");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "IP_UMC");
		exit(-2);
	}

	if (config_has_property(miConf, "PUERTO_UMC")) {
		info->puertoUmc = config_get_int_value(miConf, "PUERTO_UMC");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "PUERTO_UMC");
		exit(-2);
	}

	if (config_has_property(miConf, "QUANTUM")) {
		info->quantum = config_get_int_value(miConf, "QUANTUM");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "QUANTUM");
		exit(-2);
	}

	if (config_has_property(miConf, "QUANTUM_SLEEP")) {
		info->quantumSleep = config_get_int_value(miConf, "QUANTUM_SLEEP");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "QUANTUM_SLEEP");
		exit(-2);
	}

	if (!config_has_property(miConf, "SEM_IDS")||!config_has_property(miConf, "SEM_INIT")) {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "SEM_IDS");
		exit(-2);
	} else {
		cargar_semaforos(info,config_get_array_value(miConf, "SEM_IDS"), config_get_array_value(miConf, "SEM_INIT"));
	}

	if (!config_has_property(miConf, "IO_IDS")||!config_has_property(miConf, "IO_SLEEP")) {
		log_error("Parametros de dispositivos no cargados en el archivo de configuracion");
		exit(-2);
	} else {
		cargar_dipositivos(info,config_get_array_value(miConf, "IO_IDS"), config_get_array_value(miConf, "IO_SLEEP"));
	}

	if (config_has_property(miConf, "SHARED_VARS")) {
		cargar_sharedVars(info,config_get_array_value(miConf, "SHARED_VARS"));
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "SHARED_VARS");
		exit(-2);
	}
}

void cargar_dipositivos(stEstado *info,char** ioIds, char** ioSleep) {
	int iterator = 0;
	stDispositivo *unDispositivo;

	while (ioIds[iterator] != NULL) {
		unDispositivo = crear_dispositivo(ioIds[iterator],ioSleep[iterator]);
		list_add(info->dispositivos,unDispositivo);
		iterator++;
	}
}

void cargar_semaforos(stEstado *info,char** semIds, char** semInit) {
	int iterator = 0;
	stSemaforo *unSemadoro;

	while (semIds[iterator] != NULL) {
		unSemadoro = crear_semaforo(semIds[iterator],semInit[iterator]);
		list_add(info->semaforos,unSemadoro);
		iterator++;
	}
}

void cargar_sharedVars(stEstado *info,char** sharedVars) {
	int iterator = 0;
	stSharedVar *unaSharedVar;

	while (sharedVars[iterator] != NULL) {
		unaSharedVar = crear_sharedVar(sharedVars[iterator]);
		list_add(info->sharedVars,unaSharedVar);
		iterator++;
	}
}

stDispositivo *crear_dispositivo(char *nombre, char *retardo) {
	stDispositivo *new = malloc(sizeof(stDispositivo));/*TODO: liberar estos dispositivos al final*/
	new->nombre = strdup(nombre);
	new->retardo = retardo;
	return new;
}

stSemaforo *crear_semaforo(char *nombre, char* valor) {
	stSemaforo *new = malloc(sizeof(stSemaforo));/*TODO: liberar estos semaforos al final*/
	new->nombre = strdup(nombre);
	new->valor = strdup(valor);
	return new;
}

stSharedVar *crear_sharedVar(char *nombre) {
	stSharedVar *new = malloc(sizeof(stSharedVar));/*TODO: liberar estas sharedVar al final*/
	new->nombre = strdup(nombre);
	return new;
}


void monitoreoConfiguracion(stEstado* info) {
	char buffer[BUF_LEN];

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	int watch_descriptor = inotify_add_watch(file_descriptor, CFGFILE,
	IN_MODIFY | IN_CREATE | IN_DELETE);

	int length = read(file_descriptor, buffer, BUF_LEN);
	if (length < 0) {
		perror("read");
	}
	loadInfo(info);
	printf("\nEl archivo de configuracion se ha modificado\n");
	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
	monitoreoConfiguracion(info);
	pthread_exit(NULL);
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

int pid_incrementer(){
	pidIncrementer = pidIncrementer + 1;
	return pidIncrementer;
}

void threadCPU(int unCpu, stEstado* info) {
	stHeaderIPC *stHeaderIPC;
	stPCB *stPCB;
	int error = 0;

	while (!error) {
		if (queue_size(colaReady) == 0) {
			continue;
		}

		pthread_mutex_lock(&mutexColaReady);
		stPCB = queue_pop(colaReady);
		pthread_mutex_unlock(&mutexColaReady);

		stHeaderIPC = nuevoHeaderIPC(EXECANSISOP);

		/*TODO: Esto no funciona, hay que implementar la serializacion para mandarlo por enviarMensajeIPC*/
		if (!enviarMensajeIPC(unCpu, stHeaderIPC, "pcbSerializado")) {
			log_error("CPU error - No se pudo enviar el PCB[%d]", stPCB->pid);
			error = 1;
			close(unCpu);
			continue;
		}

		if (!recibirHeaderIPC(unCpu, stHeaderIPC)) {
			log_error("CPU error - No se pudo recibir el mensaje");
			error = 1;
			close(unCpu);
			continue;
		} else {
			if (stHeaderIPC->tipo == QUANTUM) {
				char* quantum;
				quantum = malloc(sizeof(info->quantum));
				sprintf(quantum, "%d", info->quantum);
				if (!enviarMensajeIPC(unCpu, stHeaderIPC, quantum)) {
					log_error("CPU error - No se pudo enviar el quantum");
					error = 1;
					close(unCpu);
					continue;
				}

				free(quantum);
			}
		}

		if (!recibirHeaderIPC(unCpu, stHeaderIPC)) {
			log_error("CPU error - No se pudo recibir el mensaje");
			error = 1;
			close(unCpu);
			continue;
		} else {
			if (stHeaderIPC->tipo == QUANTUMSLEEP) {
				char* sleepQuantum;
				sleepQuantum = malloc(sizeof(info->quantumSleep));
				sprintf(sleepQuantum, "%d", info->quantum);
				if (!enviarMensajeIPC(unCpu, stHeaderIPC, sleepQuantum)) {
					log_error("CPU error - No se pudo enviar el quantum sleep");
					error = 1;
					close(unCpu);
					continue;
				}

				free(sleepQuantum);
			}
		}

		if (!recibirHeaderIPC(unCpu, stHeaderIPC)) {
			log_error("CPU error - No se pudo recibir confirmacion de recepcion de PCB[%d]\n", stPCB->pid);
			error = 1;
			close(unCpu);
			continue;
		} else {
			switch (stHeaderIPC->tipo) {
			case IOANSISOP:
				/*Pide I/O*/
				/*Deberia poner el PCB en ponerlo en la cola de bloqueado del correspondiente dispositivo (TODO: hacer que la configuracion los lea)
				 * y hacer un continue para que el CPU existente tome otro PCB de la cola de listos*/
				break;
			case FINANSISOP:
				/*Termina de ejecutar el PCB, en este caso deberia moverlo a la cola de EXIT para que luego sea liberada la memoria*/
				break;
			case QUANTUMFIN:
				/*Termina de ejecutar su quantum hay que hacer un push en la cola de ready nuevamente*/
				break;
			case EXECERROR:
				/*Se produjo una excepcion por acceso a una posicion de memoria invalida (segmentation fault), imprimir error
				 * y bajar la consola tambien close (cliente)*/
				break;
			case SIGUSR1CPU:
				/*Se cayo el CPU, se debe replanificar, (continue) */
				break;
			case OBTENERVALOR:
				/*Valor de la variable compartida, devolver el valor para que el CPU siga ejecutando*/
				break;
			case GRABARVALOR:
				/*Me pasa la variable compartida y el valor*/
				break;
			case WAIT:
				/*Bloquea con el semaforo que pasa por parametro*/
				break;
			case SIGNAL:
				/*libera con el semaforo que pasa por parametro*/
				break;

			}

		}

	}

	if (error) {
		/*Lo ponemos en la cola de Ready para que otro CPU lo vuelva a tomar*/
		pthread_mutex_lock(&mutexColaReady);
		queue_push(colaReady, stPCB);
		pthread_mutex_unlock(&mutexColaReady);
		log_info("PCB[%d] vuelve a ingresar a la cola de Ready \n", stPCB->pid);
	}

	liberarHeaderIPC(stHeaderIPC);
	pthread_exit(NULL);
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
	stPCB unPCB;
	stPCB unPCBDes;
	t_UMCConfig UMCConfig;
	t_paquete paquete;

	int bytesLeidos = 0;
	char* temp_file = "nucleo.log";

	/*Inicializamos cola de ready*/
	colaReady = queue_create();

	int unCliente = 0, unSocket;
	int maximoAnterior = 0;
	struct sockaddr addressAceptado;

	int agregarSock;

	pthread_t p_thread;

	printf("----------------------------------Elestac------------------------------------\n");
	printf("-----------------------------------Nucleo------------------------------------\n");
	printf("------------------------------------v0.1-------------------------------------\n\n");
	fflush(stdout);

	/*Logger*/
	t_log* logger = log_create(temp_file, "NUCLEO", -1, LOG_LEVEL_INFO);

	/*Carga del archivo de configuracion*/
	printf("Obteniendo configuracion...");
	loadInfo(&elEstadoActual);
	printf("OK\n");
	log_info("Configuracion cargada satisfactoriamente...");

	/*Se lanza el thread para identificar cambios en el archivo de configuracion*/
	pthread_create(&p_thread, NULL, (void*) &monitoreoConfiguracion, (void*) &elEstadoActual);

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
	printf("Estableciendo conexion con la UMC...");
	elEstadoActual.sockUmc = conectar(elEstadoActual.ipUmc, elEstadoActual.puertoUmc);

	if (elEstadoActual.sockUmc != -1) {
		FD_SET(elEstadoActual.sockUmc, &(fds_master));

		stHeaderIPC = nuevoHeaderIPC(ERROR);
		if (!recibirHeaderIPC(elEstadoActual.sockUmc, stHeaderIPC)) {
			log_error("UMC handshake error - No se pudo recibir mensaje de respuesta");
			liberarHeaderIPC(stHeaderIPC);
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

		if (stHeaderIPC->tipo == OK) {
			elEstadoActual.fdMax = elEstadoActual.sockUmc;
			maximoAnterior = elEstadoActual.fdMax;
			log_info("UMC Conectada...");

		} else {

			log_error("UMC handshake error - Tipo de mensaje de confirmacion incorrecto");
		}

	} else {

		log_error("UMC connection error - No se pudo establecer el enlace");
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

								/*TODO: Calcular paginas y pedirlas a la UMC*/

								unPrograma = metadata_desde_literal(unMensaje.contenido);
								unPCB.pid = 1;
//								unPCB->pc = 0;
//								unPCB->paginaInicial = 0;/*TODO: Hacer intercambio con la UMC*/
//								unPCB->cantidadPaginas = 3; /*TODO: Hacer intercambio con la UMC*/
//								unPCB->socketConsola = unCliente;
//								unPCB->socketCPU = 0;
//								unPCB->metadata_program = unPrograma;

								crear_paquete(&paquete, EXECANSISOP);
								serializar_pcb(&paquete, &unPCB);

								deserializar_pcb(&unPCBDes, &paquete);

//
//								recibirConfigUMC(elEstadoActual.sockUmc, &UMCConfig);
//								printf("PaginasXProc[%d] Tamaño pagina[%d]\n", UMCConfig.paginasXProceso, UMCConfig.tamanioPagina);
//
//								/*Lo almaceno en la cola de PCB listo para ejecutar*/
//								/*Lo almaceno en la cola de PCB listo para ejecutar*/
//								queue_push(colaReady, unPCB);
								log_info("PCB[PID:%s] - ha ingresado a la cola de Ready", unPCBDes.pid);
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

						printf("Nuevo CPU conectado...\n");
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
