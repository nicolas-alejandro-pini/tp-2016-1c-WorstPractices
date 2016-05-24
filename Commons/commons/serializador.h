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
