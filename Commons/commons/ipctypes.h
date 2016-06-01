/*
 * ipctypes.h
 *
 *  Created on: Apr 20, 2016
 *      Author: nico
 */

#ifndef COMMONS_IPCTYPES_H_
#define COMMONS_IPCTYPES_H_

/*Definiciones para el manejo de paquetes */
#define CONNECTION_CLOSED 0
#define BROKENPIPE 1
#define CONFIG_UMC 2

/*Definicion de Parametros de Conexiones comunes*/
#define OK					0ul
#define ERROR				1ul
#define QUIENSOS			2ul

/*Definicion de Parametros de Consola - Nucleo*/
#define CONNECTCONSOLA		100ul
#define SENDANSISOP         101ul
#define IMPRIMIR			102ul
#define IMPRIMIRTEXTO		103ul
#define SENDPID				104ul
#define KILLPID				105ul

/*Definicion de Parametros de Nucleo - UMC*/
#define CONNECTNUCLEO		110ul

/*Definicion de Parametros de Nucleo - CPU*/
#define CONNECTCPU			120ul
#define EXECANSISOP			121ul
#define IOANSISOP			122ul
#define FINANSISOP			123ul
#define QUANTUMFIN			124ul
#define EXECERROR			125ul
#define SIGUSR1CPU			126ul
#define OBTENERVALOR		127ul
#define GRABARVALOR			128ul
#define WAIT				129ul
#define SIGNAL				130ul
#define QUANTUM				131ul
#define QUANTUMSLEEP		132ul

/*Definicion de tipos de mensaje enre la UMC y el Swap*/
#define SOYUMC				140ul

/*Definicion de tipos de mensaje de interfaz UMC */
#define INICIALIZAR_PROGRAMA		150ul
#define SIN_ESPACIO					151ul
#define READ_BTYES_PAGE				152ul
#define WRITE_BYTES_PAGE			153ul
#define FINPROGRAMA					154ul

/*Definicion de tipos de mensaje entre CPU y UMC */
#define NUEVAVARIABLE 170ul				/* Mensaje para definir nueva variable en la UMC */
#define POSICIONVARIABLE 171ul			/* Mensaje para obtener posicion de una variable */
#define VALORVARIABLE 172ul				/* Mensaje para obtener el valor de una variable */
#define ASIGNARVARIABLE 173ul			/* Mensaje para asignar un valor a una variable en una posicion de la UMC*/
#define GETINSTRUCCION 174ul			/* Mensaje para obtener una nueva instruccion */

#endif /* COMMONS_IPCTYPES_H_ */
