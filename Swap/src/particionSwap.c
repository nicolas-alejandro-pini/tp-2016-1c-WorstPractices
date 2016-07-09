/*
 * disco.c
 *
 *  Created on: May 20, 2016
 *      Author: nico
 */

#include <stddef.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

FILE *diskFile;

unsigned long int cantidadSectores;
unsigned long int tamanioSector;

/**
 * Crea la unidad de disco de acuerdo a los parametros
 */
int crearParticionSwap(char *nombreArchivo, unsigned long int cantidadPaginas, unsigned long int tamanioPagina){

	char clearByte = '\0';
	unsigned long int totalAEscribir;

	//Inicializo la configuracion
	cantidadSectores = cantidadPaginas;
	tamanioSector = tamanioPagina;

	unsigned long int aux;

	//Abro el archivo y lo borro si existe
	diskFile = fopen(nombreArchivo,"w+");
	if(diskFile == NULL){
		log_error("Error creando el archivo de disco...");
		return -1;
	}

	//Debo inicializar el archivo: lo cargo con ceros y dimensiono de acuerdo a la configuracion
	totalAEscribir = tamanioPagina*cantidadPaginas;
	while(totalAEscribir-- > 0){
		aux = fwrite(&clearByte, 1, 1, diskFile);
		if(aux != 1){
			log_error("Error inicializando el archivo de disco...");
			return -2;
		}
	}

	return 0;
}

/**
 * Posiciona la ventana sobre el sector a leer
 */
int seekSector(unsigned long int numeroSector){
	if(fseek(diskFile, (long int)(numeroSector*tamanioSector), SEEK_SET)!=0){
		return -1;
	}
	return 0;
}

/**
 * Escribe un sector
 */
int escribirSector(char *buffer, unsigned long int numeroSector){

	unsigned long int offset;
	//Me posiciono en el sector a escribir
	if(seekSector(numeroSector) < 0){
		log_error("Error en seek sobre la particion de Swap");
		return -1;
	}

	//Escribo el sector en la particion Swap
	for(offset = 0; offset < tamanioSector; offset++)
		if(fwrite((buffer + offset), 1, 1, diskFile) != 1){
			log_error("Error de escritura en la particion de Swap");
			return -2;
		}

	return 0;
}

/**
 * Lee un sector
 */
int leerSector(char *buffer, unsigned long int numeroSector){

	//Me posiciono en el sector a escribir
	if(seekSector(numeroSector) < 0){
		log_error("Error en seek sobre la particion de Swap");
		return -1;
	}

	if(fwrite(buffer, tamanioSector, 1, diskFile) != tamanioSector){
		log_error("Error de lectura desde la particion de Swap");
		return -2;
	}

	return 0;
}

/**
 * Cierra el archivo de disco
 */
int destruirParticionSwap(){
	//Escribo lo que haya quedado en el stream, es buena costumbre
	if(fflush(diskFile) == EOF) return -1;
	//Cierro el archivo
	if(fclose(diskFile) == EOF) return -2;
	return 0;
}
