/*
 * Parametros.h
 *
 *  Created on: 11/6/2016
 *      Author: utnso
 */

#ifndef PARAMETROS_H_
#define PARAMETROS_H_


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
	char* algoritmo;     /*Clock o ClockModificado*/
	int fdMax;              /* Numero que representa al mayor socket de fds_master. */
	int salir;              /* Indica si debo o no salir de la aplicacion. */
} stParametro;

stParametro losParametros;

#endif /* PARAMETROS_H_ */
