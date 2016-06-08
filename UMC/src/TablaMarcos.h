/*
 * TablaMarcos.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef TABLAMARCOS_H_
#define TABLAMARCOS_H_

#include <stdint.h>
#include "Tablas.h"

/* puntero a la tabla de Marcos */
char *TablaMarcos;

int buscarEnTabla(uint16_t paginaBuscada);
int reemplazarValorTabla(uint16_t Pagina, stRegistroTablas registro);

#endif /* TABLAMARCOS_H_ */
