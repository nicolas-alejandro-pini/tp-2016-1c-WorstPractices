/*
 * pcb.c
 *
 *  Created on: 17/5/2016
 *      Author: utnso
 */
#include "pcb.h"

pthread_mutex_t mutex_pid = PTHREAD_MUTEX_INITIALIZER;

int pid_incrementer() {

	pthread_mutex_lock(&mutex_pid);
	pidCounter = pidCounter + 1;
	pthread_mutex_unlock(&mutex_pid);
	return pidCounter;
}

stPCB *crear_pcb(int socket_consola, int cantidad_pag_codigo, int stack_size, stMensajeIPC *unMensajeIPC) {

	stPCB *unPCB = (stPCB*)malloc(sizeof(stPCB));
	stIndiceStack *unIndiceStack;

	unPCB->pid = pid_incrementer();
	unPCB->pc = 0;
	unPCB->socketConsola = socket_consola;
	unPCB->socketCPU = 0;
	unPCB->paginaInicioStack = cantidad_pag_codigo + 1;
	unPCB->cantidadPaginas = cantidad_pag_codigo + stack_size;

	if(unMensajeIPC->contenido==NULL){
		return NULL;
	}

	unPCB->metadata_program = metadata_desde_literal(unMensajeIPC->contenido);
	unPCB->stack = list_create();
	unPCB->offsetStack = 0;


	/*Inicializo el stack con un elemento*/
	unIndiceStack = (stIndiceStack*) malloc(sizeof(stIndiceStack)); /*Alocamos memoria*/
	if (unIndiceStack != NULL) {
		unIndiceStack->argumentos = list_create();
		unIndiceStack->pos = 0;
		unIndiceStack->variables = list_create();
		unIndiceStack->retPosicion = 0;
		list_add(unPCB->stack,unIndiceStack);
	}

	return unPCB;
}

int serializar_pcb(t_paquete *paquete, stPCB *self) {

	// Iniciar en 0
	int i, j, offset = 0;
	uint32_t l, max_stack;
	stIndiceStack *stack;


	//Serializamos los campos estaticos
	serializar_campo(paquete, &offset, &self->pid, sizeof(self->pid));
	serializar_campo(paquete, &offset, &self->pc, sizeof(self->pc));
	serializar_campo(paquete, &offset, &self->paginaInicioStack, sizeof(self->paginaInicioStack));
	serializar_campo(paquete, &offset, &self->cantidadPaginas, sizeof(self->cantidadPaginas));
	serializar_campo(paquete, &offset, &self->socketConsola, sizeof(self->socketConsola));
	serializar_campo(paquete, &offset, &self->socketCPU, sizeof(self->socketCPU));
	serializar_campo(paquete, &offset, &self->quantum, sizeof(self->quantum));
	serializar_campo(paquete, &offset, &self->quantumSleep, sizeof(self->quantumSleep));
	serializar_campo(paquete, &offset, &self->offsetStack, sizeof(self->offsetStack));

	//Serializacion del t_metadata
	serializar_campo(paquete, &offset, self->metadata_program, sizeof(t_metadata_program));

	for (i = 0; i < self->metadata_program->instrucciones_size; i++) {
		serializar_campo(paquete, &offset, (self->metadata_program->instrucciones_serializado + i), sizeof(t_intructions));
	}

	for (j = 0; j < self->metadata_program->etiquetas_size; j++) {
		serializar_campo(paquete, &offset, (self->metadata_program->etiquetas + j), sizeof(char));
	}

	//Serializacion del stack
	max_stack = list_size(self->stack);
	serializar_campo(paquete, &offset, &max_stack, sizeof(max_stack));

	for (i = 0; i < max_stack; i++) {
		stack = list_get(self->stack, i);
		serializar_campo(paquete, &offset, &stack->pos, sizeof(uint32_t));
		serializar_lista(paquete, &offset, stack->argumentos, sizeof(stPosicion));
		serializar_lista(paquete, &offset, stack->variables, sizeof(stVars));
		serializar_campo(paquete, &offset, &stack->retPosicion, sizeof(uint32_t));
		serializar_campo(paquete, &offset, &stack->retVar, sizeof(stPosicion));
	}

	// Serializacion del header
	serializar_header(paquete);

	return offset;
}

int deserializar_pcb(stPCB *self, t_paquete *paquete) {
	int i, j, offset = 0;
	uint32_t max_stack = 0;
	stIndiceStack *indiceStack;
	stPosicion *unaPosicionTest;/*TODO: borrar*/

	// paquete->data. cuando se envia el paquete recibir_header saca el header
	//                para pruebas de deserializar sin enviar el paquete hay que
	//                adelantar el offset el tamaño del header. (como si lo hubiera sacado)
	//offset = sizeof(t_header) / sizeof(t_buffer); /*Descomentar para probar sin envio*/
	deserializar_campo(paquete, &offset, &self->pid, sizeof(self->pid));
	deserializar_campo(paquete, &offset, &self->pc, sizeof(self->pc));
	deserializar_campo(paquete, &offset, &self->paginaInicioStack, sizeof(self->paginaInicioStack));
	deserializar_campo(paquete, &offset, &self->cantidadPaginas, sizeof(self->cantidadPaginas));
	deserializar_campo(paquete, &offset, &self->socketConsola, sizeof(self->socketConsola));
	deserializar_campo(paquete, &offset, &self->socketCPU, sizeof(self->socketCPU));
	deserializar_campo(paquete, &offset, &self->quantum, sizeof(self->quantum));
	deserializar_campo(paquete, &offset, &self->quantumSleep, sizeof(self->quantumSleep));
	deserializar_campo(paquete, &offset, &self->offsetStack, sizeof(self->offsetStack));


	// Reservo memoria para estructura t_metadata_program
	self->metadata_program = malloc(sizeof(t_metadata_program));
	deserializar_campo(paquete, &offset, self->metadata_program, sizeof(t_metadata_program));

	// Reservo memoria t_instruction * instrucciones_size
	self->metadata_program->instrucciones_serializado = malloc(sizeof(t_intructions) * self->metadata_program->instrucciones_size);
	for (i = 0; i < self->metadata_program->instrucciones_size; i++) {
		deserializar_campo(paquete, &offset, (self->metadata_program->instrucciones_serializado + i), sizeof(t_intructions));
	}
	// Reservo memoria char * etiquetas_size
	self->metadata_program->etiquetas = malloc(sizeof(char) * self->metadata_program->etiquetas_size);
	for (j = 0; j < self->metadata_program->etiquetas_size; j++) {
		deserializar_campo(paquete, &offset, (self->metadata_program->etiquetas + j), sizeof(char));
	}

	// Obtengo la cantidad de nodos de la lista
	deserializar_campo(paquete, &offset, &max_stack, sizeof(max_stack));
	self->stack = list_create();

	for (i = 0; i < max_stack; i++) {
		indiceStack = (stIndiceStack*) malloc(sizeof(stIndiceStack));
		indiceStack->argumentos = list_create();
		deserializar_campo(paquete, &offset, &indiceStack->pos, sizeof(uint32_t));
		deserializar_lista(paquete, &offset, indiceStack->argumentos, sizeof(stPosicion));
		indiceStack->variables = list_create();
		deserializar_lista(paquete, &offset, indiceStack->variables, sizeof(stVars));
		deserializar_campo(paquete, &offset, &indiceStack->retPosicion, sizeof(uint32_t));
		deserializar_campo(paquete, &offset, &indiceStack->retVar, sizeof(stPosicion));
		list_add(self->stack,indiceStack);
	}

	return EXIT_SUCCESS;
}

void pcb_destroy(stPCB *self) {
	metadata_destruir(self->metadata_program);
	list_destroy_and_destroy_elements(self->stack, (void*)stack_destroy);
    free(self);
}

void stack_destroy(stIndiceStack *self) {
	list_destroy_and_destroy_elements(self->variables, (void*)variables_destroy);
	list_destroy_and_destroy_elements(self->argumentos, (void*)argumentos_destroy);
	free(self);
}

void variables_destroy(stVars *self) {
    free(self);
}

void argumentos_destroy(stPosicion *self) {
    free(self);
}
