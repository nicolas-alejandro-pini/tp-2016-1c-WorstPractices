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
	stPosicion *sPos;
}stRead;
typedef struct{
	uint16_t socketResp;
	stEscrituraPagina *sEP;
}stWrite;
typedef struct{
	uint16_t socketResp;
	uint16_t pid;
}stEnd;

int frames;
int frameSize;
int frameByProc;

void *inicializarPrograma(stIni*);
void *leerBytes(stRead*);
void *escribirBytes(stWrite*);
void *finalizarPrograma(stEnd*);
int cambiarContexto(uint16_t pagina);
int elegirReemplazo(int cantidad);
int hayMarcoslibres(int cantidad);
int estaPaginaDisponible(uint16_t pagina);
void realizarAccionCPU(unsigned int tipo, char* contenido, uint16_t socket, pthread_attr_t attr);

int guardarEnTabla(uint16_t cantidadPaginas);

#endif /* ICPU_H_ */
