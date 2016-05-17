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
#define SIGUSR1				126ul

/*Definicion de tipos de mensaje enre la UMC y el Swap*/
#define SOYUMC				130ul

/*Definicion de tipos de mensaje de interfaz UMC */
#define INICIALIZAR_PROGRAMA	140ul
#define READ_BTYES_PAGE			141ul
#define WRITE_BYTES_PAGE		142ul
#define FIN_PROGRAMA			143ul

#endif /* COMMONS_IPCTYPES_H_ */
