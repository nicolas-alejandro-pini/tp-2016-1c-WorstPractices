/*
 * gestionAsignacion.c
 *
 *  Created on: May 20, 2016
 *      Author: nico
 */

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>

#include "commons/bitarray.h"
#include "commons/log.h"


//Bit Array
t_bitarray *bitArray;
char *bitArrayBuffer;

/**
 * Inicializa la gestion de asignacion del espacio en la particion Swap
 */
int initGestionAsignacion(unsigned long int cantidadSectores){

	//Reservo espacio necesario para el bitmap
	bitArrayBuffer = malloc(cantidadSectores);
	if(bitArrayBuffer == NULL){
		log_error("Error al asignar espacio en memoria para el BitMap");
		return -1;
	}

	//Creo el bit array
	bitArray = bitarray_create(bitArrayBuffer, cantidadSectores);
	if(bitArray == NULL){
		log_error("Error al crear el BitMap");
		return -2;
	}

	return 0;
}

/**
 * Termina la gestion de asignacion del espacio en la particion Swap liberando
 * todos los recursos originalmente asignados
 */
void destroyGestionAsignacion(){
	free(bitArrayBuffer);
	bitarray_destroy(bitArray);
}

/**
 * Realiza la asignacion del espacio necesario para un programa
 * En caso de que haya espacio suficiente pero no contiguo compacta.
 * En caso que no haya espacio suficiente devuelve error
 */
int asignarEspacioAProceso(unsigned long int pID, unsigned long int cantidadPaginas){

	return 0;
}

/**
 * Libera el espacio previamente asignado al proceso
 */
int liberarEspacioDeProceso(unsigned long int pID){

	return 0;
}

