/*
 * serializador.h
 *
 *  Created on: 24/5/2016
 *      Author: Nicolas Pini
 */

#ifndef COMMONS_SERIALIZADOR_H_
#define COMMONS_SERIALIZADOR_H_

#include "socketsIPCIRC.h"
#include "commons/collections/list.h"

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
	uint16_t cantidadPaginas;/* cantidad de paginas que necesita el programa. */
	char* 	 programa;	/* Programa a enviar */
} __attribute__((packed)) stPageIni;

typedef struct {
	uint16_t  offset;
	uint16_t  size;
	uint16_t  pagina;
}stPosicion;						/*Representa a el Indice de Codigo propuesto por el TP*/

typedef struct{
	uint16_t nroPagina;	/* numero de pagina a escribir. */
	uint16_t offset;		/* desplazamiento de la posicion inicial del pagina a escribir. */
	uint16_t tamanio;	/* cantidad de bytes a escribir. */
	void* buffer;	/* datos a escribir. */
} stEscrituraPagina;

typedef struct{
	uint16_t processId;		/* identificador del proceso del PCB. */
}  __attribute__((packed))stPageEnd;

typedef struct{
	int paginasXProceso;
	int tamanioPagina;
} __attribute__((packed)) t_UMCConfig;



int32_t* serializar_campo(t_paquete *paquete, int32_t *offset, void *campo, int32_t size);
int32_t* serializar_lista(t_paquete *paquete, int32_t *offset, t_list *lista, int32_t size_struct);
void serializar_header(t_paquete *paquete);

int32_t deserializar_campo(t_paquete *paquete, int32_t *offset, void *campo, int32_t size);
int32_t deserializar_lista(t_paquete *paquete, int32_t *offset, t_list *lista, int32_t size_struct);
void deserializar_header(t_header *buf_header, int32_t *offset, t_header *header);

/** Estructuras especificas **/
int serializar_ejemplo(t_paquete *paquete, t_UMCConfig *self);
int deserializar_ejemplo(t_UMCConfig *self, t_paquete *paquete);


#endif /* COMMONS_SERIALIZADOR_H_ */
