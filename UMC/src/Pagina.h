/*
 * Pagina.h
 *
 *  Created on: 4/5/2016
 *      Author: utnso
 */

#ifndef PAGINA_H_
#define PAGINA_H_

#include "../lib/librerias.h"

int inicializarPrograma(stPageIni*);
int leerBytes(stPageRead*);
int escribirBytes(stPageWrite*);
int finalizarPrograma(uint16_t unProcessId);
void realizarAccionUMC(unsigned int tipo, char* contenido);

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
	uint16_t cantidadPaginas;/* cantidad de paginas que necesita el programa. */
} __attribute__((packed)) stPageIni;

typedef struct{
	uint16_t nroPagina;	/* numero de pagina a leer. */
	uint16_t offset;		/* desplazamiento de la posicion inicial del pagina a leer. */
	uint16_t tamanio;	/* cantidad de bytes a leer. */
} __attribute__((packed)) stPageRead;

typedef struct{
	uint16_t nroPagina;	/* numero de pagina a escribir. */
	uint16_t offset;		/* desplazamiento de la posicion inicial del pagina a escribir. */
	uint16_t tamanio;	/* cantidad de bytes a escribir. */
	void* buffer;	/* datos a escribir. */
} stPageWrite;

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
}  __attribute__((packed))stPageEnd;

#endif /* PAGINA_H_ */
