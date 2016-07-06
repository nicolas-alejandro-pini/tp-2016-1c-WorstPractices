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

stSharedVar *crear_sharedVar(char *nombre);
void grabar_shared_var(t_list *lista_shared_vars,char *nombre,int *valor);
stSharedVar *obtener_shared_var(t_list *lista_shared_vars,char *nombre);

#endif /* INCLUDES_SHARED_VARS_H_ */
