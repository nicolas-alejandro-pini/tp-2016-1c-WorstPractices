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
#include "serializador.h"

typedef struct {
	uint32_t id; 					/*Variable*/
	stPosicion posicion_memoria;
} stVars; 					/*Posicion de memoria en el indice de Stack*/


typedef struct {
	uint32_t pos; 		/*Posicion del registro del Stack*/
	t_list argumentos;  /*Posiciones de memoria donde se almacenan las copias de los argumentos de la función(Listas de stPosicionIS)*/
	t_list variables;   /*Identificadores y posiciones de memoria donde se almacenan las variables locales de la función(Listas de stPosicionIS)*/
	uint32_t retPosicion; /*Posición del índice de código donde se debe retornar al finalizar la ejecución de la función*/
	stPosicion retVar; /*Posición de memoria donde se debe almacenar el resultado de la función provisto por la sentencia RETURN*/
} stIndiceStack;

typedef struct {
	uint32_t pid; /*Numero identificador del proceso unico en el sistema */
	uint32_t pc; /*Numero de la próxima instrucción del Programa que se debe ejecutar*/
	uint32_t paginaInicial; /*Numero de pagina inicial*/
	uint32_t cantidadPaginas; /*Numero de pagina inicial*/
	uint32_t socketConsola; /*Numero de socket de la consola a la cual le devolvemos las salidas del programa en ejecucion*/
	uint32_t socketCPU; /*Numero de socket de la CPU que esta ejecutando en ese momento el pcb*/
	uint32_t quantum; /*Quantum a ejecutar*/
	uint32_t quantumSleep; /*Retardo del quantum*/
	t_metadata_program* metadata_program;
	//t_list stack;
}__attribute__((packed)) stPCB;

int serializar_pcb(t_paquete *paquete, stPCB *self);
int deserializar_pcb(stPCB *self, t_paquete *paquete);

#endif /* COMMONS_PCB_H_ */
