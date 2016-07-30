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
#include <string.h>
#include <time.h>

#include "Swap.h"
#include "gestionAsignacion.h"
#include "particionSwap.h"
#include "commons/bitarray.h"
#include "commons/log.h"
#include "commons/collections/list.h"

//Bit Array
t_bitarray *bitArray;
char *bitArrayBuffer;

//Lista de asignaciones
t_list *assignmentList;

t_swap_config *loaded_config;

/**
 * Inicializa la gestion de asignacion del espacio en la particion Swap
 */
int initGestionAsignacion(t_swap_config * config){

	loaded_config = config;
//	int i = 0;

	//Reservo espacio necesario para el bitmap
	bitArrayBuffer = (char *)calloc(1, config->cantidadPaginas);
	if(bitArrayBuffer == NULL){
		log_error("Error al asignar espacio en memoria para el BitMap");
		return -1;
	}

	//Creo el bit array
	bitArray = bitarray_create(bitArrayBuffer, config->cantidadPaginas);

	//ELIMINAR ESTO!!!!
//	for(i = 0; i < config->cantidadPaginas; i+=2)
//		bitarray_set_bit(bitArray, i);
	///////////////////

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
	unsigned long int maximoSectoresLibres = 0;
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
 * Busca la información de asignación realizada a un sector dado
 *
 */
t_asignacion * buscarAsignacionSector(unsigned long int nroSector){
	int cantAsignaciones = list_size(assignmentList);
	unsigned long int i;
	t_asignacion *asignacion;

	for(i = 0; i < cantAsignaciones; i++){
		asignacion = list_get(assignmentList, i);
		if(asignacion->sector == nroSector)
			return asignacion;
	}

	return NULL;
}

/**
 * Compacta la particion SWAP para crear espacio libre contiguo para nuevos procesos
 */
int compactarParticionSwap(){

	off_t offset;
	off_t proximoSectorOcupado;

	t_asignacion * asignacion;

	char *buffer = malloc(loaded_config->tamanioPagina);

	for(offset = 0; offset < bitArray->size; offset++){
		if(bitarray_test_bit(bitArray, offset) == false){

			//Encontré un sector libre, busco el próximo ocupado

			proximoSectorOcupado = offset + 1;
			while(proximoSectorOcupado < bitArray->size){
				if(bitarray_test_bit(bitArray, proximoSectorOcupado) == true)
					break;
				proximoSectorOcupado++;
			}

			//Si llegué al final de mi particion termino el procedimiento
			if(proximoSectorOcupado == bitArray->size)
				break;

			//-------------------------------
			// 1. actualizo la partición SWAP
			//-------------------------------

			//Tengo que traer al offset actual el próximo ocupado
			if(leerSector(buffer, proximoSectorOcupado) < 0){
				//Error al leer la particion SWAP
				log_error("Error al leer la particion SWAP");
				free(buffer);
				return -1;
			}

			if(escribirSector(buffer, offset) < 0){
				//Error al escribir en el offset actual
				log_error("Error al escribir la particion SWAP");
				free(buffer);
				return -1;
			}


			//------------------------------------
			// 2. Actualizo la tabla de asignación
			//------------------------------------

			asignacion = buscarAsignacionSector(proximoSectorOcupado);
			if(asignacion == NULL){
				log_error("Error al acceder a la tabla de asignaciones");
				free(buffer);
				return -1;
			}

			//Realizo el cambio del sector
			asignacion->sector = offset;


			//--------------------------
			// 3. Actualizo el bit array
			//--------------------------

			//Marco el actual como ocupado
			bitarray_set_bit(bitArray, offset);

			//Marco el previamente ocupado como libre
			bitarray_clean_bit(bitArray, proximoSectorOcupado);

		}
	}


	//Libero lo alocado
	free(buffer);
	return 0;
}

/**
 * Verifica la existencia del proceso en la tabla de asignacion
 *
 */
unsigned long int buscarIndicePrimeraAsignacionProceso(unsigned long int pId){
	int cantAsignaciones = list_size(assignmentList);
	unsigned long int i;
	t_asignacion *asignacion;

	//Si no tengo asignaciones previas realizadas
	if(cantAsignaciones == 0)
		return -1;

	//Busco la primera asignación realizada al PID
	for(i = 0; i < cantAsignaciones; i++){
		asignacion = list_get(assignmentList, i);
		if(asignacion->pid == pId)
			break;
	}

	if(i == cantAsignaciones)
		return -1;

	return i;
}

/**
 * Reserva los sectores para el nuevo proceso, previamente se realizaron los controles necesarios
 *
 */
int reservarEspacioProceso(unsigned long int pid, unsigned long int sectorInicial, unsigned long int cantidadSectores){

	unsigned long int nroSector;
	unsigned long int nroPagina = 0;

	for(nroSector = sectorInicial; nroSector < sectorInicial + cantidadSectores; nroSector++){
		//Actualizo el bitmap
		bitarray_set_bit(bitArray, nroSector);

		//Agrego el elemento a la tabla de asignacion
		t_asignacion *nuevaAsignacion = (t_asignacion *)malloc(sizeof(t_asignacion));
		nuevaAsignacion->pid = pid;
		nuevaAsignacion->sector = nroSector;
		nuevaAsignacion->pagina = nroPagina++;

		list_add(assignmentList, nuevaAsignacion);
	}

	return 0;
}

/**
 * Libera el espacio previamente asignado al proceso
 */
int liberarEspacioDeProceso(unsigned long int pID){

	long int i;
	t_asignacion * asignacion;

	i = buscarIndicePrimeraAsignacionProceso(pID);
	while(i >= 0){

		//Libero la asignacion realizada
		asignacion = (t_asignacion *)list_remove(assignmentList, i);
		bitarray_clean_bit(bitArray, asignacion->sector);
		free(asignacion);

		//Busco la siguiente
		i = buscarIndicePrimeraAsignacionProceso(pID);
	}

	return 0;
}

/**
 * Realiza la asignacion del espacio necesario para un programa
 * En caso de que haya espacio suficiente pero no contiguo compacta.
 * En caso que no haya espacio suficiente devuelve error
 */
int asignarEspacioAProceso(unsigned long int pID, unsigned long int cantidadPaginas, char *bufferPrograma){

	t_bloque_libre info_bloque_libre;
	unsigned long int largoPrograma, offset;
	char * bufferSector;
	int r;

	r = buscarIndicePrimeraAsignacionProceso(pID);
	if(r >= 0){
		log_error("El proceso al que se desea asignar espacio ya se encuentra en la tabla de asignacion");
		return -1;
	}

	//Si la cantidad de paginas requeridas para el PID es mayor a las disponibles totales
	if(cantidadPaginas > bitArray->size){
		log_debug("La unidad SWAP no posee sectores suficientes para satisfacer la solicitud");
		return -2;
	}

	if(cantidadSectoresLibres() < cantidadPaginas){
		log_debug("La unidad SWAP no posee suficientes sectores libres para satisfacer la solicitud");
		return -3;
	}

	//Vamos a usar Worst Fit para la asignacion de sectores a los procesos
	if(cantidadSectoresLibresContiguosMaxima(&info_bloque_libre) < cantidadPaginas){
		//Debo compactar, luego tengo que poder realizar la asignacion
		log_info("Compactando la partición SWAP, la operación llevará %ld milisegundos", loaded_config->retardoCompactacion);
		compactarParticionSwap();
		usleep(loaded_config->retardoCompactacion * 1000);
		cantidadSectoresLibresContiguosMaxima(&info_bloque_libre);
		log_debug("Compactación terminada, ahora se dispone de un espacio contiguo de %ld sectores", info_bloque_libre.largo);
	}

	if(reservarEspacioProceso(pID, info_bloque_libre.offset, cantidadPaginas) < 0){
		log_error("Ocurrio un error al reservar el espacio en el SWAP para el proceso con PID: %d", pID);
		return -4;
	}

	log_debug("Asignacion de paginas satisfactoria al proceso(PID %u): %u->%u",
			pID, info_bloque_libre.offset, info_bloque_libre.offset + cantidadPaginas - 1);

	//Grabo la particion SWAP con el codigo del programa
	largoPrograma = strlen(bufferPrograma);
	offset = 0;
	bufferSector = (char *)calloc(1, loaded_config->tamanioPagina);
	while(offset < largoPrograma){

		//Cargo el buffer del sector con el código del programa
		if((largoPrograma - offset) < loaded_config->tamanioPagina){
			memset(bufferSector, '\0', loaded_config->tamanioPagina);
			memcpy(bufferSector, (bufferPrograma + offset), (largoPrograma - offset));
		} else {
			memcpy(bufferSector, (bufferPrograma + offset), loaded_config->tamanioPagina);
		}

		if(escribirSector(bufferSector, info_bloque_libre.offset + (offset / loaded_config->tamanioPagina)) < 0){
			//Error al escribir el sector en disco con el codigo del programa
			log_error("Error al escribir el codigo del programa en un sector del SWAP, se hace rollback de lo asignado");
			//Libero lo asignado al programa
			liberarEspacioDeProceso(pID);
			return -5;
		}

		//Me muevo el siguiente sector
		offset += loaded_config->tamanioPagina;
	}
	free(bufferSector);

	return 0;
}

/**
 * Busca la asignacion realizada para un proceso y una pagina
 */
t_asignacion *buscarAsignacionPaginaProceso(unsigned long int pID, unsigned long int nroPagina){
	int cantAsignaciones = list_size(assignmentList);
	unsigned long int i;
	t_asignacion *asignacion;

	for(i = 0; i < cantAsignaciones; i++){
		asignacion = list_get(assignmentList, i);
		if(asignacion->pid == pID && asignacion->pagina == nroPagina)
			return asignacion;
	}

	return NULL;
}

/**
 * Realiza la lectura de la pagina de un proceso
 *
 */
int leerPaginaProceso(unsigned long int pID, unsigned long int nroPagina, char *bufferPagina){
	t_asignacion *asignacion;

	log_debug("La operacion de lectura llevara %d milisegundos", loaded_config->retardoAcceso);
	usleep(loaded_config->retardoAcceso * 1000);

	if((asignacion = buscarAsignacionPaginaProceso(pID, nroPagina)) == NULL){
		//Asignacion no encontrada
		return -1;
	}

	if(leerSector(bufferPagina, asignacion->sector) < 0){
		//Error al leer el sector desde la particion SWAP
		return -2;
	}

	return 0;
}

/**
 * Realiza la escritura de la pagina de un proceso
 *
 */
int escribirPaginaProceso(unsigned long int pID, unsigned long int nroPagina, char *bufferPagina){
	t_asignacion *asignacion;

	log_debug("La operacion de escritura llevara %d milisegundos", loaded_config->retardoAcceso);
	usleep(loaded_config->retardoAcceso * 1000);

	if((asignacion = buscarAsignacionPaginaProceso(pID, nroPagina)) == NULL){
		//Asignacion no encontrada
		return -1;
	}

	if(escribirSector(bufferPagina, asignacion->sector) < 0){
		//Error al escribir el sector en la particion SWAP
		return -2;
	}

	return 0;
}
