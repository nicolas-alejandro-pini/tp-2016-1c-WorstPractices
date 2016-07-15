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

int loadInfo(stEstado* info, char inotify);
void cargar_dipositivos(stEstado *info,char** ioIds, char** ioSleep);
void cargar_semaforos(char** semIds, char** semInit);
void cargar_sharedVars(char** sharedVars);
stDispositivo *crear_dispositivo(char *nombre, char *retardo);
void * monitor_configuracion(void* info);
void destruir_dispositivo(stDispositivo *unDispositivo);
void destruir_lista_dispositivos(stEstado *info);
void destruir_rafaga_io(stRafaga *unaRafaga);

#endif /* NUCLEO_CONFIG_H_ */
