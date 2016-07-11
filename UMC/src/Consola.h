/*
 * Consola.h
 *
 *  Created on: 8/7/2016
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "Parametros.h"
#include "string.h"
#include "TablaMarcos.h"
#include "TLB.h"
#include <stdlib.h>

#define MAX_BUFFER 200ul
#define MAX_COMANDO 20ul
#define MAX_PARAMETROS 20ul
#define CANTIDAD_PARAMETROS 2

void mostrarHelp();
void consolaUMC();

#endif /* CONSOLA_H_ */
