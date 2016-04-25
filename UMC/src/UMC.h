/*
 * UMC.h
 *
 *  Created on: 21/4/2016
 *      Author: utnso
 */

#ifndef UMC_H_
#define UMC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sockets.h"
#include "socketsIPCIRC.h"
#include "config.h"

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

typedef struct{
	int miPuerto;		/* Puerto por el que escucho. */
	char* ipSwap;
	int puertoSwap;
	int frames;			/* Socket con el que escucho. */
	int frameSize;      /* Numero que representa al mayor socket de fds_master. */
	int frameByProc;    /* Numero que representa al mayor socket de fds_master. */
	int entradasTLB;    /* Numero que representa al mayor socket de fds_master. */
	int delay;          /* Indica si debo o no salir de la aplicacion. */
	int sockEscuchador;		/* Socket con el que escucho. */
	int fdMax;              /* Numero que representa al mayor socket de fds_master. */
	int salir;              /* Indica si debo o no salir de la aplicacion. */
} stParametro;


/*Archivos de Configuracion*/
//#define CFGFILE		"configuracion.conf"
/*Definicion de Parametros de Conexiones*/

#define ERROR			101
#define OK				103
#define QUIENSOS		105
#define CONNECTCLIENTE	116


/*Definicion de Parametros de Respuesta*/

//#define CONNECT		0x01	    /* Usa este tipo para el connect*/
/*#define SEARCH		0x04

#define NOTFOUND	60
#define ADD		0x07
#define EXIT		0x14
*/
/*Definicion de Parametros para el Archivo Log*/

#define OK_LOG		51
#define ERROR_LOG	52
#define WARNING_LOG	53
#define INFO_LOG	54
#define DEBUG_LOG	55

/*Definicion de MACROS*/

//#define TAMANIOMAXPALABRAS 2048
#define TAMDATOS 100
//#define TAMBUFFER 255
#define LONGITUD_MAX_DE_CONTENIDO 1024
/*#define UNLARGO 2048
#define TAMSECTOR 512
#define TAMID 10
*/
/*--------------------------------------------Estructuras----------------------------------------*/
/*												 */

//typedef struct {
//	int socket;	/* Socket con el que me comunico con él. */
//	char dispositivo[TAMDATOS];
//	int estado;
//}stConectados;
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

int verificarNombreArchivo(char* );
void loadInfo (stParametro*, char*);
void cerrarSockets(stParametro* );
void finalizarSistema(stMensajeIPC*, int, stParametro* );

#endif /* UMC_H_ */
