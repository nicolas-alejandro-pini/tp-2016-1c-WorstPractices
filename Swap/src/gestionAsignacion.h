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
int initGestionAsignacion(unsigned long int cantidadSectores, unsigned long int retardo);

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
int asignarEspacioAProceso(unsigned long int pID, unsigned long int cantidadPaginas);

/**
 * Libera el espacio previamente asignado al proceso
 */
int liberarEspacioDeProceso(unsigned long int pID);

#endif /* GESTIONASIGNACION_H_ */
