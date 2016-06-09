/*
 * config.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef NUCLEO_CONFIG_H_
#define NUCLEO_CONFIG_H_
#include "Nucleo.h"

void loadInfo(stEstado* info);
void cargar_dipositivos(stEstado *info,char** ioIds, char** ioSleep);
void cargar_semaforos(stEstado *info,char** semIds, char** semInit);
void cargar_sharedVars(stEstado *info,char** sharedVars);
stDispositivo *crear_dispositivo(char *nombre, char *retardo);
stSemaforo *crear_semaforo(char *nombre, char* valor);
stSharedVar *crear_sharedVar(char *nombre);
void monitor_configuracion(stEstado* info);

#endif /* NUCLEO_CONFIG_H_ */
