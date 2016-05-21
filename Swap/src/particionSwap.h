/*
 * disco.h
 *
 *  Created on: May 20, 2016
 *      Author: nico
 */

#ifndef PARTICIONSWAP_H_
#define PARTICIONSWAP_H_

/**
 * Crea la unidad de disco de acuerdo a los parametros
 */
int crearParticionSwap(char *nombreArchivo, unsigned long int cantidadPaginas, unsigned long int tamanioPagina);

/**
 * Cierra el archivo de disco
 */
int destruirParticionSwap();

/**
 * Escribe un sector
 */
int escribirSector(char *buffer, unsigned long int numeroSector);

/**
 * Lee un sector
 */
int leerSector(char *buffer, unsigned long int numeroSector);

#endif /* PARTICIONSWAP_H_ */
