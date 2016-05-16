/*
 * pcb.h
 *
 *  Created on: 16/5/2016
 *      Author: utnso
 */

#ifndef COMMONS_PCB_H_
#define COMMONS_PCB_H_

typedef struct {
	int  offset;
	int  size;
	int  pagina;

}stPosicion;						/*Representa a el Indice de Codigo propuesto por el TP*/

typedef struct {
	int   pc;
	char* etiqueta;

}stIndiceEtiquetas;

typedef struct {
	int 		pos;				/*Posicion del registro del Stack*/
	t_list 		argumentos;			/*Posiciones de memoria donde se almacenan las copias de los argumentos de la función*/
	t_list 		variables;			/*Identificadores y posiciones de memoria donde se almacenan las variables locales de la función*/
	int 		retPosicion;		/*Posición del índice de código donde se debe retornar al finalizar la ejecución de la función*/
	stPosicion 	retVar;             /*Posición de memoria donde se debe almacenar el resultado de la función provisto por la sentencia RETURN*/

}stIndiceStack;

typedef struct {
	int  pid;	 					/*Numero identificador del proceso unico en el sistema */
	int  pc; 	  					/*Numero de la próxima instrucción del Programa que se debe ejecutar*/
	int  paginas; 					/*Cantidad de páginas utilizadas por el código del Programa AnSISOP, empezando por la página cero*/
	t_list indicesCodigo;   		/*Contiene el offset del inicio y del fin de cada sentencia del Programa (lista de stPosicion)*/
	t_list etiquetas;   			/*Utilizada para conocer las líneas de código correspondientes al inicio de los procedimientos y a las etiquetas (lista de stEtiquetas)*/

}stPCB;

/**
 * @NAME: crearPCB
 * @PRE:  un puntero PCB
 * @POST: memoria reservada para la estructura PCB
 */
int crearPCB(stPCB *unPCB);

/**
 * @NAME: liberarPCB
 * @PRE:  una estructura PCB con memoria reservada
 * @POST: libera la estructura stPCB
 */
void liberarPCB(stPCB *unPCB);

#endif /* COMMONS_PCB_H_ */
