/*
 * ipctypes.h
 *
 *  Created on: Apr 20, 2016
 *      Author: nico
 */

#ifndef COMMONS_IPCTYPES_H_
#define COMMONS_IPCTYPES_H_

/*Definicion de Parametros de Conexiones comunes*/
#define OK					100
#define ERROR				101
#define QUIENSOS			102

/*Definicion de Parametros de Consola - Nucleo*/
#define CONNECTCONSOLA		103
#define SENDANSISOP         104
#define IMPRIMIR			105
#define IMPRIMIRTEXTO		106
#define SENDPID				107
#define KILLPID				108

/*Definicion de Parametros de Nucleo - UMC*/
#define CONNECTNUCLEO		115

/*Definicion de Parametros de Nucleo - CPU*/
#define CONNECTCPU			104
#define EXECANSISOP			105

#endif /* COMMONS_IPCTYPES_H_ */
