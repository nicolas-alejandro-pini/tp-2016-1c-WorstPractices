/*
 * Pagina.h
 *
 *  Created on: 4/5/2016
 *      Author: utnso
 */

#ifndef PAGINA_H_
#define PAGINA_H_

#include "../lib/librerias.h"
#include <pthread.h>

int inicializarPrograma(stPageIni*);
int leerBytes(stPageRead*);
int escribirBytes(stPageWrite*);
int finalizarPrograma(uint16_t unProcessId);
void realizarAccionUMC(unsigned int tipo, char* contenido);

#endif /* PAGINA_H_ */
