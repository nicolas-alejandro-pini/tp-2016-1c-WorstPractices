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

int loadInfo(stEstado* info, char inotify) {

	t_config* miConf, *otraConf;

	if (inotify==0) {
		miConf = config_create(info->path_conf); /*Estructura de configuracion*/
		info->dispositivos = list_create();
		inicializar_semaforos();
		inicializar_lista_shared_var();

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
			info->quantumSleep = config_get_int_value(otraConf, "QUANTUM_SLEEP");
		} else {
			return EXIT_FAILURE;
		}
		config_destroy(otraConf);
		log_info("Quantum [%d], Quantum Sleep [%d]", info->quantum, info->quantumSleep);
		return EXIT_SUCCESS;
	}
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
	while (semIds[iterator] != NULL) {
		crear_semaforo(semIds[iterator], semInit[iterator]);
		iterator++;
	}
}
void cargar_sharedVars(char** sharedVars) {
	int iterator = 0;
	while (sharedVars[iterator] != NULL) {
		crear_shared_var(sharedVars[iterator]);
		iterator++;
	}
}
stDispositivo *crear_dispositivo(char *nombre, char *retardo) {
	stDispositivo *new = malloc(sizeof(stDispositivo));/*TODO: liberar estos dispositivos al final*/
	new->nombre = strdup(nombre);
	new->retardo = retardo;
	new->rafagas = queue_create();
	new->numInq = 0;
	pthread_mutex_init(&(new->mutex), NULL);
	pthread_mutex_init(&(new->empty), NULL);
	return new;
}

void * monitor_configuracion(void * param) {
	char buffer[BUF_LEN];
	int watch_descriptor;
	stEstado* info = (stEstado *)param;

	// Al inicializar inotify este nos devuelve un descriptor de archivo
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}

	// Creamos un monitor sobre un path indicando que eventos queremos escuchar
	while(1){
		watch_descriptor = inotify_add_watch(file_descriptor, info->path_conf,
		IN_MODIFY | IN_CREATE | IN_DELETE);

		int length = read(file_descriptor, buffer, BUF_LEN);
		if (length < 0) {
			perror("read");
		}
		loadInfo(info,-1);
	}

	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
	pthread_exit(NULL);
}

void destruir_rafaga_io(stRafaga *unaRafaga){
	free(unaRafaga);
}

void destruir_dispositivo(stDispositivo *unDispositivo){
	pthread_mutex_destroy(&unDispositivo->mutex);
	pthread_mutex_destroy(&unDispositivo->empty);
	queue_destroy_and_destroy_elements(unDispositivo->rafagas,(void*)destruir_rafaga_io);
	free(unDispositivo->nombre);
	free(unDispositivo);
}

void destruir_lista_dispositivos(stEstado *info){
	list_destroy_and_destroy_elements(info->dispositivos,(void*)destruir_dispositivo);
}
