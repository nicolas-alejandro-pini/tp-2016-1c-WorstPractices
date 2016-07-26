/*
 * Swap.h
 *
 *  Created on: 30/6/2016
 *      Author: ntoranzo
 */

#ifndef SWAP_H_
#define SWAP_H_

typedef struct{
    int puertoEscucha;
    char *nombreSwap;
    long cantidadPaginas;
    long tamanioPagina;
    long retardoCompactacion;
    long retardoAcceso;
}t_swap_config;


#endif /* SWAP_H_ */
