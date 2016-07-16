/*
 * shared_vars.h
 *
 *  Created on: 27/6/2016
 *      Author: utnso
 */

#ifndef INCLUDES_SHARED_VARS_H_
#define INCLUDES_SHARED_VARS_H_

#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/serializador.h>

void inicializar_lista_shared_var();
void crear_shared_var(char *nombre);
stSharedVar *quitar_shared_var(char *nombre);
void grabar_shared_var(stSharedVar *unaSharedVar);
stSharedVar obtener_shared_var(char *nombre);

#endif /* INCLUDES_SHARED_VARS_H_ */
