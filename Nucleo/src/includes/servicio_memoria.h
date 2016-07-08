/*
 * servicio_memoria.h
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */

#ifndef SERVICIO_MEMORIA_H_
#define SERVICIO_MEMORIA_H_
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <commons/socketsIPCIRC.h>
#include <commons/serializador.h>
#include <commons/ipctypes.h>
#include <commons/log.h>

int inicializar_programa(int pid, int cantidad_paginas, char* programa, int socket_umc);

#endif /* SERVICIO_MEMORIA_H_ */
