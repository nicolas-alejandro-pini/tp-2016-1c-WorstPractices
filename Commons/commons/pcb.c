/*
 * pcb.c
 *
 *  Created on: 17/5/2016
 *      Author: utnso
 */
#include "pcb.h"

int serializar_pcb(t_paquete *paquete, stPCB *self) {

	// Iniciar en 0
	int i, j, offset = 0;

	//Serializamos los campos estaticos
	serializar_campo(paquete, &offset, &self->pid, sizeof(self->pid));
	serializar_campo(paquete, &offset, &self->pc, sizeof(self->pc));
	serializar_campo(paquete, &offset, &self->paginaInicial, sizeof(self->pc));
	serializar_campo(paquete, &offset, &self->cantidadPaginas, sizeof(self->cantidadPaginas));
	serializar_campo(paquete, &offset, &self->socketConsola, sizeof(self->socketConsola));
	serializar_campo(paquete, &offset, &self->socketCPU, sizeof(self->socketCPU));
	serializar_campo(paquete, &offset, &self->quantum, sizeof(self->quantum));
	serializar_campo(paquete, &offset, &self->quantumSleep, sizeof(self->quantumSleep));
	//Serializacion del t_metadata
	t_metadata_program* miMetadata = self->metadata_program;
	serializar_campo(paquete, &offset, miMetadata, sizeof(t_metadata_program));

	for (i = 0; i < miMetadata->instrucciones_size; i++) {
		serializar_campo(paquete, &offset, (miMetadata->instrucciones_serializado + i), sizeof(t_intructions));
	}

	for (j = 0; j < miMetadata->etiquetas_size; j++) {
		serializar_campo(paquete, &offset, (miMetadata->etiquetas + j), sizeof(char));
	}

	// Serializacion del header
	serializar_header(paquete);

	return offset;
}

int deserializar_pcb(stPCB *self,t_paquete *paquete) {
	int i, j, offset = 0;
	// paquete->data. cuando se envia el paquete recibir_header saca el header
	//                para pruebas de deserializar sin enviar el paquete hay que
	//                adelantar el offset el tamaño del header. (como si lo hubiera sacado)
	//offset = sizeof(t_header) / sizeof(t_buffer); /*Descomentar para probar sin envio*/
	deserializar_campo(paquete, &offset, &self->pid, sizeof(self->pid));
	deserializar_campo(paquete, &offset, &self->pc, sizeof(self->pc));
	deserializar_campo(paquete, &offset, &self->paginaInicial, sizeof(self->pc));
	deserializar_campo(paquete, &offset, &self->cantidadPaginas, sizeof(self->cantidadPaginas));
	deserializar_campo(paquete, &offset, &self->socketConsola, sizeof(self->socketConsola));
	deserializar_campo(paquete, &offset, &self->socketCPU, sizeof(self->socketCPU));
	deserializar_campo(paquete, &offset, &self->quantum, sizeof(self->quantum));
	deserializar_campo(paquete, &offset, &self->quantumSleep, sizeof(self->quantumSleep));

	// Reservo memoria para estructura t_metadata_program
	self->metadata_program = malloc(sizeof(t_metadata_program));
	deserializar_campo(paquete, &offset, self->metadata_program, sizeof(t_metadata_program));

	// Reservo memoria t_instruction * instrucciones_size
	self->metadata_program->instrucciones_serializado = malloc(sizeof(t_intructions) * self->metadata_program->instrucciones_size);
	for (i = 0; i < self->metadata_program->instrucciones_size; i++) {
		deserializar_campo(paquete, &offset, (self->metadata_program->instrucciones_serializado + i), sizeof(t_intructions));
	}
	// Reservo memoria char * etiquetas_size
	self->metadata_program->etiquetas = malloc(sizeof(char)* self->metadata_program->etiquetas_size);
	for (j = 0; j < self->metadata_program->etiquetas_size; j++) {
		deserializar_campo(paquete, &offset, (self->metadata_program->etiquetas + j), sizeof(char));
	}

	// liberar al finalizar PCB
	// free(self->metadata_program->etiquetas);
	// free(self->metadata_program->instrucciones_serializado);
	// free(self->metadata_program);

	return EXIT_SUCCESS;
}
