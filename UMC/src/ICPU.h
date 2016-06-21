/*
 * Pagina.h
 *
 *  Created on: 4/5/2016
 *      Author: utnso
 */

#ifndef ICPU_H_
#define ICPU_H_

#include <commons/serializador.h>
#include <commons/ipctypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "Memoria.h"
#include "ISwap.h"
#include "TablaMarcos.h"
#include "TLB.h"

typedef struct{
	uint16_t socketResp;
	stPageIni *sPI;
}stIni;

typedef struct{
	uint16_t socketResp;
	uint16_t pid;
}stEnd;

int frames;
int frameSize;
int frameByProc;

void *inicializarPrograma(stIni*);
void leerBytes(stPosicion* unaLectura, uint16_t pid, uint16_t socketCPU);
void escribirBytes(stEscrituraPagina* unaEscritura, uint16_t pid, uint16_t socketCPU);
void finalizarPrograma(uint16_t pid, uint16_t socketCPU);
void *finalizarProgramaNucleo(stEnd *fin);
void cambiarContexto(uint16_t pagina);

void* ejecutarPageFault(uint16_t pid, uint16_t pagina, uint16_t usarTLB);
void realizarAccionCPU(uint16_t socket);

int guardarEnTabla(uint16_t cantidadPaginas);

#endif /* ICPU_H_ */
