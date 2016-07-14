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
	int punteroClock;
	void *tabla;
}stNodoListaTP;


/*
 * TLB -> directo MP
 * TLB miss -> TP -> MP
 * TLB miss -> TP page fault-> Swap -> MP, TP, TLB
 *
 */


int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t *frame);
int reemplazarValorTabla(uint16_t *frameNuevo, stNodoListaTP *tablaPaginas, uint16_t pagina);
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
stRegistroTP *EjecutarClock(stNodoListaTP *tablaPaginas, uint16_t pagina);
stRegistroTP *EjecutarClockModificado(stNodoListaTP *tablaPaginas, uint16_t pagina);
stRegistroTP *buscarRegistroEnTabla(uint16_t pid, uint16_t paginaBuscada);
int obtenerPresenciasTabladePaginas(stNodoListaTP* nodo);
stRegistroTP* obtenerRegistroTabladePaginas(stNodoListaTP* nodo, int pagina);
int grabarEnSwap(uint16_t pid, uint16_t marco, uint16_t pagina);
int agregarFrameATablaMarcos(uint16_t frameNuevo, stNodoListaTP *tablaPaginas, uint16_t pagina);
#endif /* TABLAMARCOS_H_ */
