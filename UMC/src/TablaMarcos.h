/*
 * TablaMarcos.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TABLAMARCOS_H_
#define TABLAMARCOS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list_mutex.h>
#include "Memoria.h"
#include "Parametros.h"
#include "ISwap.h"

// con este valor se conserva el valor del marco del la pagina a reemplazar
#define REEMPLAZAR_MARCO	97ul
#define ENCONTRADO			98ul
#define NO_ENCONTRADO		99ul

typedef struct{
	// pagina es el indice
	uint16_t marco;
	unsigned char bit2ndChance;
	unsigned char bitPresencia;
	unsigned char bitModificado;
}stRegistroTP;

typedef struct{
	int pid;
	int size;
	void *tabla;
}stNodoListaTP;


/*
 * TLB -> directo MP
 * TLB miss -> TP -> MP
 * TLB miss -> TP page fault-> Swap -> MP, TP, TLB
 *
 */

/* puntero a la tabla de Marcos */
t_list_mutex *TablaMarcos;

int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t **frame);
stRegistroTP *reemplazarValorTabla(uint16_t pid, uint16_t Pagina, stRegistroTP registro, uint8_t flag);
void creatListaDeTablas();
int crearTabla(uint16_t processId, uint16_t longitud_tabla);
stNodoListaTP *buscarPID(uint16_t pid);
void listarMemoria();
void listarMemoriaPid(uint16_t pid);
void mostrarTabla();
void mostrarTablaPid(uint16_t pid);
void liberarTablaPid(uint16_t pid);
void marcarMemoriaModificada(uint16_t pid);
stNodoListaTP* obtenerPrimerPidTabla();
stRegistroTP *EjecutarClock(stNodoListaTP *nodo, uint16_t pagina, stRegistroTP registro, uint8_t flag);
stRegistroTP *EjecutarClockModificado(stNodoListaTP *nodo, uint16_t pagina, stRegistroTP registro, uint8_t flag);
stRegistroTP *buscarRegistroEnTabla(uint16_t pid, uint16_t paginaBuscada);
#endif /* TABLAMARCOS_H_ */
