/*
 * pcb.h
 *
 *  Created on: 16/5/2016
 *      Author: utnso
 */

#ifndef COMMONS_PCB_H_
#define COMMONS_PCB_H_

#include "parser/metadata_program.h"
#include "collections/list.h"
#include "elestaclibrary.h"

typedef struct {
	u_int32_t id; /*Variable*/
	u_int32_t pagina;
	u_int32_t offset;
	u_int32_t size;
} stPosicionIS; /*Posicion de memoria en el indice de Stack*/

typedef struct {
	u_int32_t lenght;
	char *data;
}stPCBSerializado;

typedef struct {
	u_int32_t pos; /*Posicion del registro del Stack*/
	t_list argumentos; /*Posiciones de memoria donde se almacenan las copias de los argumentos de la función(Listas de stPosicionIS)*/
	t_list variables; /*Identificadores y posiciones de memoria donde se almacenan las variables locales de la función(Listas de stPosicionIS)*/
	u_int32_t retPosicion; /*Posición del índice de código donde se debe retornar al finalizar la ejecución de la función*/
	stPosicion retVar; /*Posición de memoria donde se debe almacenar el resultado de la función provisto por la sentencia RETURN*/
} stIndiceStack;

typedef struct {
	char pid[16]; /*Numero identificador del proceso unico en el sistema */
	u_int32_t pc; /*Numero de la próxima instrucción del Programa que se debe ejecutar*/
	u_int32_t paginaInicial; /*Numero de pagina inicial*/
	u_int32_t cantidadPaginas; /*Numero de pagina inicial*/
	u_int32_t tamanioPaginas; /*Tamanio de paginas*/
	u_int32_t socketConsola; /*Numero de socket de la consola a la cual le devolvemos las salidas del programa en ejecucion*/
	u_int32_t socketCPU; /*Numero de socket de la CPU que esta ejecutando en ese momento el pcb*/
	t_metadata_program* metadata_program;
	t_list stack;
} stPCB;

/**
 * @NAME: crearPCB
 * @PRE:  un puntero PCB
 * @POST: memoria reservada para la estructura PCB
 */
stPCB * crearPCB(t_metadata_program *unPrograma, int socketConsola);

/**
 * @NAME: liberarPCB
 * @PRE:  una estructura PCB con memoria reservada
 * @POST: libera la estructura stPCB
 */
void liberarPCB(stPCB *unPCB);

/**
 * @NAME: serializarPCB
 * @PRE:  una estructura PCB con memoria reservada
 * @POST: un puntero a una estructura stPCBSerializado
 */
stPCBSerializado *serializarPCB(stPCB *unPCB);

/**
 * @NAME: deserializarPCB
 * @PRE:  una estructura stPCBSerializado con memoria reservada
 * @POST: un puntero a una estructura stPCB
 */
stPCB *deserializarPCB(stPCBSerializado *unPCBSerializado);


#endif /* COMMONS_PCB_H_ */
