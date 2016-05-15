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


#include "commons/elestaclibrary.h"
#include "commons/sockets.h"
#include "commons/socketsIPCIRC.h"
#include <commons/config.h>

#include "Pagina.h"

typedef struct{
	char* miIP;             /* Mi direccion de IP. Ej: <"127.0.0.1"> */
	int puertoNucleo;		/* Puerto de escucha Nucleo */
	int sockNucleo;			/* Socket de escucha Nucleo */
	int puertoCpu;			/* Puerto de escucha CPU */
	int sockCpu;			/* Socket de escucha CPU */
	int fdMax;              /* Numero que representa al mayor socket de fds_master. */
	int fdMax2;             /* Numero que representa al mayor socket de fds_master. */
	int salir;              /* Indica si debo o no salir de la aplicacion. */
} stEstado;

fd_set fds_master;			/* Lista de todos mis sockets.*/
fd_set read_fds;	  		/* Sublista de fds_master.*/
int sockSwap;
void *memoriaPrincipal;

typedef struct{
	int miPuerto;		/* Puerto por el que escucho. */
	char* ipSwap;		/* ip para conectarse a Swap. */
	int puertoSwap;		/* Puerto para conectarse a Swap. */
	int frames;			/* Cantidad de marcos a usar. */
	int frameSize;      /* Tamaño de marcos a usar. */
	int frameByProc;    /* Numero de marcos por proceso. */
	int entradasTLB;    /* Numero de entradas en cache. */
	int delay;          /* Retardo para la respuesta de UMC. */
	int sockEscuchador;		/* Socket con el que escucho. */
	int sockSwap;			/* Socket con el que hablo con Swap. */
	int fdMax;              /* Numero que representa al mayor socket de fds_master. */
	int salir;              /* Indica si debo o no salir de la aplicacion. */
} stParametro;


/*Archivos de Configuracion*/
/*define CFGFILE		"configuracion.conf"*/


/*Definicion de Parametros de Respuesta*/
/*
#define CONNECTSWAP	501
#define CPUREQ		602
*/


/*Definicion de MACROS*/


#define TAMDATOS 100
#define LONGITUD_MAX_DE_CONTENIDO 1024


/*--------------------------------------------Estructuras----------------------------------------*/
/*												 */

/*typedef struct {
/*	int socket;	*//* Socket con el que me comunico con él. */
/*	char dispositivo[TAMDATOS];
//	int estado;
//}stConectados;
*/
/*
typedef struct {
	int unServidor;
	int tipoDeAccion;
	char elResultado[LONGITUD_MAX_DE_CONTENIDO];
}stResultados;
*/

/*
typedef struct{

	int idSector;
}stSectorOFT;
*/

void loadInfo (stParametro*, char*);
void cerrarSockets(stParametro* );
void finalizarSistema(stMensajeIPC*, int, stParametro* );
int cpuHandShake (int socket, char* mensaje, int tipoHeader);
int inicializarMemoriaDisponible(long tamanio, long cantidad);

#endif /* UMC_H_ */
