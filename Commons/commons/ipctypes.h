/*
 * ipctypes.h
 *
 *  Created on: Apr 20, 2016
 *      Author: nico
 */

#ifndef COMMONS_IPCTYPES_H_
#define COMMONS_IPCTYPES_H_

/*Definicion de Parametros de Conexiones comunes*/
#define OK					100ul
#define ERROR				101ul
#define QUIENSOS			102ul

/*Definicion de Parametros de Consola - Nucleo*/
#define CONNECTCONSOLA		103ul
#define SENDANSISOP         104ul
#define IMPRIMIR			105ul
#define IMPRIMIRTEXTO		106ul
#define SENDPID				107ul
#define KILLPID				108ul

/*Definicion de Parametros de Nucleo - UMC*/
#define CONNECTNUCLEO		115ui

/*Definicion de Parametros de Nucleo - CPU*/
#define CONNECTCPU			104ui
#define EXECANSISOP			105ui

#endif /* COMMONS_IPCTYPES_H_ */
