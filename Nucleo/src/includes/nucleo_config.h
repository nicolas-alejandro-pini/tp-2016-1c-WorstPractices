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

int loadInfo(stEstado* info, t_list* lista_semaforos,t_list* lista_shared_vars);
void cargar_dipositivos(stEstado *info,char** ioIds, char** ioSleep);
void cargar_semaforos(char** semIds, char** semInit);
void cargar_sharedVars(char** sharedVars);
stDispositivo *crear_dispositivo(char *nombre, char *retardo);
void monitor_configuracion(stEstado* info);

void consola_conectada(stEstado *pEstado, int unSocket, uint32_t pid);
uint32_t consola_desconectada(stEstado *pEstado, int unSocket);
void consola_crear_lista(stEstado *pEstado);
void consola_destruir_lista(stEstado *pEstado);

#endif /* NUCLEO_CONFIG_H_ */
