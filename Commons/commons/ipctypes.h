/*
 * ipctypes.h
 *
 *  Created on: Apr 20, 2016
 *      Author: nico
 */

#ifndef COMMONS_IPCTYPES_H_
#define COMMONS_IPCTYPES_H_

/*Definicion de Parametros de Conexiones comunes*/
#define OK					0ul
#define ERROR				1ul
#define QUIENSOS			2ul

/*Definicion de Parametros de Consola - Nucleo*/
#define CONNECTCONSOLA		103ul
#define SENDANSISOP         104ul
#define IMPRIMIR			105ul
#define IMPRIMIRTEXTO		106ul
#define SENDPID				107ul
#define KILLPID				108ul

/*Definicion de Parametros de Nucleo - UMC*/
#define CONNECTNUCLEO		115ul

/*Definicion de Parametros de Nucleo - CPU*/
#define CONNECTCPU			104ul
#define EXECANSISOP			105ul
#define IOANSISOP			125ul
#define FINANSISOP			126ul
#define QUANTUMFIN			127ul
#define EXECERROR			128ul
#define SIGUSR1				129ul

/*Definicion de tipos de mensaje enre la UMC y el Swap*/
#define SOYUMC				200ul

/*Definicion de tipos de mensaje de interfaz UMC */
#define INICIALIZAR_PROGRAMA	151ul
#define READ_BTYES_PAGE			152ul
#define WRITE_BYTES_PAGE		153ul

#endif /* COMMONS_IPCTYPES_H_ */
