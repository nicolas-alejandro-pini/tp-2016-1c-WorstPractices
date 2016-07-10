/*
 * cpuSignals.h
 *
 *  Created on: Jul 10, 2016
 *      Author: nico
 */

#ifndef CPUSIGNALS_H_
#define CPUSIGNALS_H_

/**
 * Funcion de interpretacion de seniales
 */
void signal_handler(int);

/**
 * Arranca la rutina de interpretacion de seniales de la CPU
 */
void init_signal_handler(t_configCPU *);

#endif /* CPUSIGNALS_H_ */
