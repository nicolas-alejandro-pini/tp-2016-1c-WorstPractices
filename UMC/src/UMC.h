/*
 * UMC.h
 *
 *  Created on: 21/4/2016
 *      Author: Diego Laib
 */

#ifndef UMC_H_
#define UMC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <stddef.h>
#include <unistd.h>


#include <commons/serializador.h>
#include <commons/sockets.h>
#include <commons/socketsIPCIRC.h>
#include <commons/config.h>
#include <commons/log.h>

#include "ICPU.h"
#include "Parametros.h"
#include "Memoria.h"
#include "Consola.h"
#include "tests/test_umc.h"

fd_set fds_master;			/* Lista de todos mis sockets.*/
fd_set read_fds;	  		/* Sublista de fds_master.*/


/*Definicion de MACROS*/


#define TAMDATOS 100
#define LONGITUD_MAX_DE_CONTENIDO 1024


/*--------------------------------------------Declaraciones----------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/


void loadInfo (stParametro*, char*);
void loadInfo_destruir (stParametro* info);
void cerrarSockets(stParametro* );
void finalizarSistema(stMensajeIPC*, int, stParametro* );
int swapHandShake (int socket, char* mensaje, int tipoHeader);


#endif /* UMC_H_ */
