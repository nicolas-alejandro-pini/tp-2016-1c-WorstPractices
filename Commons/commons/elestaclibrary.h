/*
 * elestaclibrary.h
 *
 *  Created on: 12/5/2016
 *      Author: utnso
 */

#ifndef COMMONS_ELESTACLIBRARY_H_
#define COMMONS_ELESTACLIBRARY_H_

#include <stdint.h>
#include <sys/types.h>

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
	uint16_t cantidadPaginas;/* cantidad de paginas que necesita el programa. */
} __attribute__((packed)) stPageIni;

typedef struct {
	u_int32_t pagina;
	u_int32_t offset;
	u_int32_t size;
} stPosicion;

typedef struct{
	uint16_t nroPagina;	/* numero de pagina a escribir. */
	uint16_t offset;		/* desplazamiento de la posicion inicial del pagina a escribir. */
	uint16_t tamanio;	/* cantidad de bytes a escribir. */
	void* buffer;	/* datos a escribir. */
} stEscrituraPagina;

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
}  __attribute__((packed))stPageEnd;


#endif /* COMMONS_ELESTACLIBRARY_H_ */
