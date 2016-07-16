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
	uint16_t marcos_x_proceso; // Se utiliza para definir longitud de tabla
}stIni;

typedef struct{
	uint16_t socketResp;
	uint16_t pid;
}stEnd;

int gFrames;
int gFrameSize;
int gFrameByProc;

int inicializarPrograma(int unCliente);
int leerBytes(void **buffer, stPosicion* posLogica, uint16_t pid);
int escribirBytes(stEscrituraPagina* unaEscritura, uint16_t pid);
void *finalizarProgramaNucleo(uint32_t fin);
uint32_t cambiarContexto(stMensajeIPC *unMensaje);
int ejecutarPageFault(uint16_t pid, uint16_t pagina, uint16_t *pframeNuevo);
void realizarAccionCPU(uint32_t socket);

int guardarEnTabla(uint16_t cantidadPaginas);


/* Auxiliares */
void limpiarPosicion(void *buffer, stPosicion *pPos);
void limpiarEscrituraPagina(void *buffer, stEscrituraPagina *pPos);
void loguear_buffer(void *buffer, uint16_t size, int unSocket);
#endif /* ICPU_H_ */
