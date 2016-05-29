/*
 * pcb.c
 *
 *  Created on: 17/5/2016
 *      Author: utnso
 */
#include "pcb.h"

int serializar_pcb(t_paquete *paquete, stPCB *self) {

	// Iniciar en 0
	int32_t offset = 0;

	//Serializamos los campos estaticos
	serializar_campo(paquete, &offset, &self->pid, sizeof(self->pid));
//	serializar_campo(paquete, &offset, &self->pc, sizeof(self->pc));
//	serializar_campo(paquete, &offset, &self->paginaInicial, sizeof(self->pc));
//	serializar_campo(paquete, &offset, &self->cantidadPaginas, sizeof(self->cantidadPaginas));
//	serializar_campo(paquete, &offset, &self->tamanioPaginas, sizeof(self->tamanioPaginas));
//	serializar_campo(paquete, &offset, &self->socketConsola, sizeof(self->socketConsola));
//	serializar_campo(paquete, &offset, &self->socketCPU, sizeof(self->socketCPU));

	// Serializo la estructura stPCB , debe tener el __attribute__((packed))
	serializar_campo(paquete, &offset, self, sizeof(stPCB));

	//Serializacion del t_metadata
//	t_metadata_program* miMetadata = self->metadata_program;
//
//	serializar_campo(paquete, &offset, miMetadata, sizeof(t_metadata_program));
//
//	int i, j;
//
//	for (i = 0; i < miMetadata->instrucciones_size; ++i) {
//		serializar_campo(paquete, &offset, &miMetadata->instrucciones_serializado[i], sizeof(t_intructions));
//	}
//
//	for (j = 0; j < miMetadata->etiquetas_size; ++j) {
//		serializar_campo(paquete, &offset, &miMetadata->etiquetas[j], sizeof(char));
//	}

	serializar_header(paquete);

	return offset;
}

int deserializar_pcb(stPCB *self,t_paquete *paquete) {
	int offset = 0;

	deserializar_campo(paquete, &offset, &self->pid, sizeof(self->pid));
//	deserializar_campo(paquete, &offset, &self->pc, sizeof(self->pc));
//	deserializar_campo(paquete, &offset, &self->paginaInicial, sizeof(self->pc));
//	deserializar_campo(paquete, &offset, &self->cantidadPaginas, sizeof(self->cantidadPaginas));
//	deserializar_campo(paquete, &offset, &self->tamanioPaginas, sizeof(self->tamanioPaginas));
//	deserializar_campo(paquete, &offset, &self->socketConsola, sizeof(self->socketConsola));
//	deserializar_campo(paquete, &offset, &self->socketCPU, sizeof(self->socketCPU));

	deserializar_campo(paquete, &offset, &self, sizeof(stPCB));
//	self->metadata_program = malloc(sizeof(t_metadata_program));
//	deserializar_campo(paquete, &offset, &self->metadata_program, sizeof(t_metadata_program));
//
//	int i, j;
//
//	for (i = 0; i < self->metadata_program->instrucciones_size; ++i) {
//		deserializar_campo(paquete, &offset, &self->metadata_program->instrucciones_serializado[i], sizeof(t_intructions));
//	}
//
//	for (j = 0; j < self->metadata_program->etiquetas_size; ++j) {
//		deserializar_campo(paquete, &offset, &self->metadata_program->etiquetas[j], sizeof(char));
//	}

	return EXIT_SUCCESS;

}
