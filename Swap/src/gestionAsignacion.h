/*
 * gestionAsignacion.h
 *
 *  Created on: May 20, 2016
 *      Author: nico
 */

#ifndef GESTIONASIGNACION_H_
#define GESTIONASIGNACION_H_

/**
 * Tabla de asignacion
 * ------------------------------
 * 	PID	|	Pagina	|	Sector	|
 * 	-----------------------------
 *
 */
typedef struct{
	unsigned long int pid;
	unsigned long int pagina;
	unsigned long int sector;
} t_asignacion;

typedef struct{
	unsigned long int offset;
	unsigned long int largo;
}t_bloque_libre;

/**
 * Inicializa la gestion de asignacion del espacio en la particion Swap
 */
int initGestionAsignacion(t_swap_config *);

/**
 * Termina la gestion de asignacion del espacio en la particion Swap liberando
 * todos los recursos originalmente asignados
 */
void destroyGestionAsignacion();


/**
 * Realiza la asignacion del espacio necesario para un programa
 * En caso de que haya espacio suficiente pero no contiguo compacta.
 * En caso que no haya espacio suficiente devuelve error
 */
int asignarEspacioAProceso(unsigned long int pID, unsigned long int cantidadPaginas, char *bufferPrograma);

/**
 * Libera el espacio previamente asignado al proceso
 */
int liberarEspacioDeProceso(unsigned long int pID);


/**
 * Realiza la lectura de la pagina de un proceso
 */
int leerPaginaProceso(unsigned long int pID, unsigned long int nroPagina, char *bufferPagina);

/**
 * Realiza la escritura de la pagina de un proceso
 */
int escribirPaginaProceso(unsigned long int pID, unsigned long int nroPagina, char *bufferPagina);

#endif /* GESTIONASIGNACION_H_ */
