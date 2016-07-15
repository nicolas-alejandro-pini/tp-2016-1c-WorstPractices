/*
 * consumidor_cpu.h
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#ifndef CONSUMIDOR_CPU_H_
#define CONSUMIDOR_CPU_H_

#include "Nucleo.h"
#include "planificador.h"
#include "semaforos.h"
#include "shared_vars.h"
#include "nucleo_config.h"

int consola_activa(stPCB *unPCB);
int bloquear_pcb(stPCB *unPCB,char *dispositivo_name,int dispositivo_time);
void *consumidor_cpu(int * unCliente);




#endif /* CONSUMIDOR_CPU_H_ */
