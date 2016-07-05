/*
 * config.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef NUCLEO_CONFIG_H_
#define NUCLEO_CONFIG_H_
#include "Nucleo.h"
#include <pthread.h>

void loadInfo(stEstado* info,t_list* lista_semaforos, t_list* lista_shared_vars);
void cargar_dipositivos(stEstado *info,char** ioIds, char** ioSleep);
void cargar_semaforos(char** semIds, char** semInit);
void cargar_sharedVars(char** sharedVars);
stDispositivo *crear_dispositivo(char *nombre, char *retardo);
void monitor_configuracion(stEstado* info);

#endif /* NUCLEO_CONFIG_H_ */
