/*
 * gestionAsignacion.h
 *
 *  Created on: May 20, 2016
 *      Author: nico
 */

#ifndef GESTIONASIGNACION_H_
#define GESTIONASIGNACION_H_

/**
 * Inicializa la gestion de asignacion del espacio en la particion Swap
 */
int initGestionAsignacion(unsigned long int cantidadSectores);

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

#endif /* GESTIONASIGNACION_H_ */
