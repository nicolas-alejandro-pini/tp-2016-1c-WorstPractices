/*
 * gestionAsignacion.c
 *
 *  Created on: May 20, 2016
 *      Author: nico
 */

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

#include "gestionAsignacion.h"
#include "commons/bitarray.h"
#include "commons/log.h"
#include "commons/collections/list.h"

//Bit Array
t_bitarray *bitArray;
char *bitArrayBuffer;

//Lista de asignaciones
t_list *assignmentList;

unsigned long int retardoCompactacion;

/**
 * Inicializa la gestion de asignacion del espacio en la particion Swap
 */
int initGestionAsignacion(unsigned long int cantidadSectores, unsigned long int retardo){

	retardoCompactacion = retardo;

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

	//Inicializo la lista de asignaciones
	assignmentList = list_create();

	return 0;
}

/**
 * Termina la gestion de asignacion del espacio en la particion Swap liberando
 * todos los recursos originalmente asignados
 */
void destroyGestionAsignacion(){
	free(bitArrayBuffer);
	bitarray_destroy(bitArray);
	list_destroy(assignmentList);
}

/**
 * Devuelve la cantidad total de sectores libres en la unidad SWAP
 */
unsigned long int cantidadSectoresLibres(){
	unsigned long int sectoresLibres = 0;
	off_t offset;
	//Chequeo la cantidad requerida contra el total libre
	for(offset = 0; offset < bitArray->size; offset++){
		if(bitarray_test_bit(bitArray, offset) == false)
			sectoresLibres++;
	}
	return sectoresLibres;
}

/**
 * Devuelve el tamaño del conjunto mas grande de sectores libres contiguos
 */
unsigned long int cantidadSectoresLibresContiguosMaxima(t_bloque_libre *info_bloque){

	unsigned long int sectoresLibres = 0;
	unsigned long int maximoSectoresLibres = -1;
	off_t offset;

	//Chequeo la cantidad requerida contra el total libre
	for(offset = 0; offset < bitArray->size; offset++){
		if(bitarray_test_bit(bitArray, offset) == false){
			if(info_bloque != NULL && sectoresLibres == 0){
				info_bloque->offset = offset;
			}
			sectoresLibres++;
		} else {
			if(maximoSectoresLibres < sectoresLibres){
				//Se encontro un nuevo maximo
				maximoSectoresLibres = sectoresLibres;
				if(info_bloque != NULL){
					info_bloque->largo = sectoresLibres;
				}
			}
			sectoresLibres = 0;
		}
	}

	if(maximoSectoresLibres < sectoresLibres){
		//Se encontro un nuevo maximo
		maximoSectoresLibres = sectoresLibres;
		if(info_bloque != NULL){
			info_bloque->largo = sectoresLibres;
		}
	}
	return maximoSectoresLibres;
}

/**
 * Compacta la particion SWAP para crear espacio libre contiguo para nuevos procesos
 */
int compactarParticionSwap(){
	return 0;
}

/**
 * Reserva los sectores para el nuevo proceso
 *
 */
int reservarEspacioProceso(unsigned long int pid, unsigned long int sectorInicial, unsigned long int cantidadSectores){
	return 0;
}

/**
 * Libera el espacio previamente asignado al proceso
 */
int liberarEspacioDeProceso(unsigned long int pID){

	return 0;
}

/**
 * Realiza la asignacion del espacio necesario para un programa
 * En caso de que haya espacio suficiente pero no contiguo compacta.
 * En caso que no haya espacio suficiente devuelve error
 */
int asignarEspacioAProceso(unsigned long int pID, unsigned long int cantidadPaginas){

	t_bloque_libre info_bloque_libre;

	//Si la cantidad de paginas requeridas para el PID es mayor a las disponibles totales
	if(cantidadPaginas > bitArray->size){
		log_info("La unidad SWAP no posee sectores suficientes para satisfacer la solicitud");
		return -1;
	}

	if(cantidadSectoresLibres() < cantidadPaginas){
		log_info("La unidad SWAP no posee suficientes sectores libres para satisfacer la solicitud");
		return -2;
	}

	//Vamos a usar Worst Fit para la asignacion de sectores a los procesos
	if(cantidadSectoresLibresContiguosMaxima(&info_bloque_libre) < cantidadPaginas){
		//Debo compactar, luego tengo que poder realizar la asignacion
		compactarParticionSwap();
		usleep(retardoCompactacion);
		cantidadSectoresLibresContiguosMaxima(&info_bloque_libre);
	}

	if(reservarEspacioProceso(pID, info_bloque_libre.offset, cantidadPaginas) < 0){
		log_error("Ocurrio un error al reservar el espacio en el SWAP para el proceso con PID: %d", pID);
		return -3;
	}

	log_info("Asignacion de paginas satisfactoria al proceso(PID %ul): %ul->%ul",
			pID, info_bloque_libre.offset, info_bloque_libre.offset + cantidadPaginas - 1);

	return 0;
}

