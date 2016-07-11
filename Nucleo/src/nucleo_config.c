/*
 * config.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "includes/nucleo_config.h"
#include "includes/semaforos.h"
#include "includes/shared_vars.h"

void loadInfo(stEstado* info,t_list* lista_semaforos, t_list* lista_shared_vars) {

	t_config* miConf = config_create(CFGFILE); /*Estructura de configuracion*/
	info->dispositivos = list_create();
	lista_semaforos = list_create();
	lista_shared_vars = list_create();

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
		cargar_semaforos(config_get_array_value(miConf, "SEM_IDS"), config_get_array_value(miConf, "SEM_INIT"));
	}

	if (!config_has_property(miConf, "IO_IDS")||!config_has_property(miConf, "IO_SLEEP")) {
		log_error("Parametros de dispositivos no cargados en el archivo de configuracion");
		exit(-2);
	} else {
		cargar_dipositivos(info,config_get_array_value(miConf, "IO_IDS"), config_get_array_value(miConf, "IO_SLEEP"));
	}

	if (config_has_property(miConf, "SHARED_VARS")) {
		cargar_sharedVars(config_get_array_value(miConf, "SHARED_VARS"));
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "SHARED_VARS");
		exit(-2);
	}

	if (config_has_property(miConf, "STACK_SIZE")) {
		info->stackSize = config_get_int_value(miConf, "STACK_SIZE");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n", "STACK_SIZE");
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
void cargar_semaforos(char** semIds, char** semInit) {
	int iterator = 0;
	stSemaforo *unSemaforo;

	while (semIds[iterator] != NULL) {
		unSemaforo = crear_semaforo(semIds[iterator],semInit[iterator]);
		list_add(listaSem,unSemaforo);
		iterator++;
	}
}
void cargar_sharedVars(char** sharedVars) {
	int iterator = 0;
	stSharedVar *unaSharedVar;

	while (sharedVars[iterator] != NULL) {
		unaSharedVar = crear_shared_var(sharedVars[iterator]);
		list_add(listaSharedVars,unaSharedVar);
		iterator++;
	}
}
stDispositivo *crear_dispositivo(char *nombre, char *retardo) {
	stDispositivo *new = malloc(sizeof(stDispositivo));/*TODO: liberar estos dispositivos al final*/
	new->nombre = strdup(nombre);
	new->retardo = retardo;
	new->rafagas = queue_create();
	pthread_mutex_init(&(new->mutex), 0);
	pthread_cond_init(&(new->empty),0);
	/*Lanzar hilo para que haga el tratamiento de cada una da las rafagas de la cola*/

	return new;
}


void monitor_configuracion(stEstado* info) {
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
	loadInfo(info,listaSem,listaSharedVars);
	printf("\nEl archivo de configuracion se ha modificado\n");
	inotify_rm_watch(file_descriptor, watch_descriptor);
	close(file_descriptor);
	monitor_configuracion(info);
	pthread_exit(NULL);
}
