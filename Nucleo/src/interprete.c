/*
 * interprete.c
 *
 *  Created on: 12/5/2016
 *      Author: utnso
 */

#include "interprete.h"

int interprete(stPCB *unPCB, const stEstado elEstadoActual, char *programa){
	t_Interprete tInterprete;

	iniciarInterprete(&tInterprete, programa);

	/*
	if(!(crearInterprete(&tInterprete, programa))){
		printf("Interprete: Error al crear el interprete");
		return -1;
	}
	*/

	while(existeProxSentencia(&tInterprete)){

	}


	return 0;
}

void iniciarInterprete(t_Interprete *tInterprete, char *programa){

	tInterprete->programa = programa;
	tInterprete->posMaxima = strlen(programa);
	tInterprete->posActual = 0;
	tInterprete->cantSentencias = 0;
}

int existeProxSentencia(t_Interprete *tInterprete){
	//SALTO_DE_LINEA
	//memchr()
	if(tInterprete->posMaxima < tInterprete->posMaxima)
	return 0;
}

int proximaSentencia(t_Interprete *tInterprete){
	return 0;
}
