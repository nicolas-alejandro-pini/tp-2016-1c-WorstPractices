/*
 * ISwap.h
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */

#ifndef ISWAP_H_
#define ISWAP_H_

#include <commons/serializador.h>
#include <commons/sockets.h>
#include "Parametros.h"


int inicializarSwap(stPageIni *st);
int enviarPagina(uint16_t pagina, char* buffer);
char* recibirPagina(uint16_t pagina);
int destruirPrograma(uint16_t pid);


#endif /* ISWAP_H_ */
