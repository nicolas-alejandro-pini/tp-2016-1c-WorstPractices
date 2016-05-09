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
#define CONNECTCONSOLA		103ui
#define SENDANSISOP         104ui
#define IMPRIMIR			105ui
#define IMPRIMIRTEXTO		106ui
#define SENDPID				107ui
#define KILLPID				108ui

/*Definicion de Parametros de Nucleo - UMC*/
#define CONNECTNUCLEO		115ui

/*Definicion de Parametros de Nucleo - CPU*/
#define CONNECTCPU			104ui
#define EXECANSISOP			105ui

#endif /* COMMONS_IPCTYPES_H_ */
