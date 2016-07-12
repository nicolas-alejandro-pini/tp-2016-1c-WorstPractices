/*
 * config.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "includes/Nucleo.h"
#include "includes/nucleo_config.h"
#include "includes/semaforos.h"
#include "includes/shared_vars.h"

int loadInfo(stEstado* info, t_list* lista_semaforos, t_list* lista_shared_vars, char inotify) {

	t_config* miConf, *otraConf;

	if (inotify==0) {
		miConf = config_create(info->path_conf); /*Estructura de configuracion*/
		info->dispositivos = list_create();
		lista_semaforos = list_create();
		lista_shared_vars = list_create();

		if (miConf == NULL) {
			log_error("Error iniciando la configuracion...\n");
			return EXIT_FAILURE;
		}
		if (config_has_property(miConf, "IP")) {
			info->miIP = config_get_string_value(miConf, "IP");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "IP");
			return EXIT_FAILURE;
		}

		if (config_has_property(miConf, "PUERTO")) {
			info->miPuerto = config_get_int_value(miConf, "PUERTO");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "PUERTO");
			return EXIT_FAILURE;
		}

		if (config_has_property(miConf, "IP_UMC")) {
			info->ipUmc = config_get_string_value(miConf, "IP_UMC");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "IP_UMC");
			return EXIT_FAILURE;
		}

		if (config_has_property(miConf, "PUERTO_UMC")) {
			info->puertoUmc = config_get_int_value(miConf, "PUERTO_UMC");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "PUERTO_UMC");
			return EXIT_FAILURE;
		}

		if (config_has_property(miConf, "QUANTUM")) {
			info->quantum = config_get_int_value(miConf, "QUANTUM");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "QUANTUM");
			return EXIT_FAILURE;
		}

		if (config_has_property(miConf, "QUANTUM_SLEEP")) {
			info->quantumSleep = config_get_int_value(miConf, "QUANTUM_SLEEP");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "QUANTUM_SLEEP");
			return EXIT_FAILURE;
		}

		if (!config_has_property(miConf, "SEM_IDS") || !config_has_property(miConf, "SEM_INIT")) {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "SEM_IDS");
			return EXIT_FAILURE;
		} else {
			cargar_semaforos(config_get_array_value(miConf, "SEM_IDS"), config_get_array_value(miConf, "SEM_INIT"));
		}

		if (!config_has_property(miConf, "IO_IDS") || !config_has_property(miConf, "IO_SLEEP")) {
			log_error("Parametros de dispositivos no cargados en el archivo de configuracion");
			return EXIT_FAILURE;
		} else {
			cargar_dipositivos(info, config_get_array_value(miConf, "IO_IDS"), config_get_array_value(miConf, "IO_SLEEP"));
		}

		if (config_has_property(miConf, "SHARED_VARS")) {
			cargar_sharedVars(config_get_array_value(miConf, "SHARED_VARS"));
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "SHARED_VARS");
			return EXIT_FAILURE;
		}

		if (config_has_property(miConf, "STACK_SIZE")) {
			info->stackSize = config_get_int_value(miConf, "STACK_SIZE");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "STACK_SIZE");
			return EXIT_FAILURE;
		}
		inotify++;
		return EXIT_SUCCESS;
	} else {
		otraConf = config_create(info->path_conf);
		if (config_has_property(otraConf, "QUANTUM")) {
			info->quantum = config_get_int_value(otraConf, "QUANTUM");
		} else {
			log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "QUANTUM");
			return EXIT_FAILURE;
		}
		config_destroy(otraConf);
		printf("Valor del quantum [%d]\n", info->quantum);
		return EXIT_SUCCESS;

	}
}

void consola_conectada(stEstado *pEstado, int unSocket, uint32_t pid){
	stConsola *consola = malloc(sizeof(stConsola));
	consola->pid = pid;
	consola->socket = unSocket;

	list_add(pEstado->consolas_activas, consola);
}

/* Devuelve el pid de la desconectada y la saca de la lista, 0 en caso de no hallarla */
uint32_t consola_desconectada(stEstado *pEstado, int unSocket){
	stConsola *consola = NULL;
	int i=0;
	t_list *list = pEstado->consolas_activas;
	int size = list_size(list);

	for(i=0; i<size; i++){
		consola = list_get(list, i);
		if(consola->socket == unSocket)
		{
			list_remove(list, i);
			return consola->pid;
		}
	}
	return 0;  // No encontrada
}

uint32_t buscar_consola_activa(int pid){
	stConsola *consola = NULL;
	int i=0;
	t_list *list = obtenerEstadoActual()->consolas_activas;
	int size = list_size(list);

	for(i=0; i<size; i++){
		consola = list_get(list, i);
		if(consola->pid == pid)
		{
			return 1;
		}
	}
	return 0;  // No encontrada
}

void consola_crear_lista(stEstado *pEstado){
	pEstado->consolas_activas = list_create();
}

void consola_destruir_lista(stEstado *pEstado){
	list_destroy(pEstado->consolas_activas);
}



void cargar_dipositivos(stEstado *info, char** ioIds, char** ioSleep) {
	int iterator = 0;
	stDispositivo *unDispositivo;

	while (ioIds[iterator] != NULL) {
		unDispositivo = crear_dispositivo(ioIds[iterator], ioSleep[iterator]);
		list_add(info->dispositivos, unDispositivo);
		iterator++;
	}
}
void cargar_semaforos(char** semIds, char** semInit) {
	int iterator = 0;
	stSemaforo *unSemaforo;

	while (semIds[iterator] != NULL) {
		unSemaforo = crear_semaforo(semIds[iterator], semInit[iterator]);
		list_add(listaSem, unSemaforo);
		iterator++;
	}
}
void cargar_sharedVars(char** sharedVars) {
	int iterator = 0;
	stSharedVar *unaSharedVar;

	while (sharedVars[iterator] != NULL) {
		unaSharedVar = crear_shared_var(sharedVars[iterator]);
		list_add(listaSharedVars, unaSharedVar);
		iterator++;
	}
}
stDispositivo *crear_dispositivo(char *nombre, char *retardo) {
	stDispositivo *new = malloc(sizeof(stDispositivo));/*TODO: liberar estos dispositivos al final*/
	new->nombre = strdup(nombre);
	new->retardo = retardo;
	new->rafagas = queue_create();
	pthread_mutex_init(&(new->mutex), 0);
	pthread_mutex_init(&(new->empty), 0);
	return new;
}

void monitor_configuracion(stEstado* info) {
	char buffer[BUF_LEN];
	int watch_descriptor;

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	while(1){
		watch_descriptor = inotify_add_watch(file_descriptor, info->path_conf,
		IN_MODIFY | IN_CREATE | IN_DELETE);

		log_info("");

		int length = read(file_descriptor, buffer, BUF_LEN);
		if (length < 0) {
			perror("read");
		}
		loadInfo(info, listaSem, listaSharedVars, -1);
	}

	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
	pthread_exit(NULL);
}
