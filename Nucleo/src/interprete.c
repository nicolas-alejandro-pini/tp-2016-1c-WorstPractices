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


	while(proximaSentencia(&tInterprete)){

	}


	return 0;
}

void iniciarInterprete(t_Interprete *tInterprete, char *programa){
	strcpy(tInterprete->programa, programa);
	tInterprete->longPrograma = strlen(programa);
	tInterprete->posActual = tInterprete->programa;
	tInterprete->cantSentencias = 0;
}

int proximaSentencia(t_Interprete *tInterprete){
	char *proxSentencia = strchr(tInterprete->posActual, SALTO_DE_LINEA);

	if(proxSentencia == NULL)
		return 0;

	tInterprete->cantSentencias += 1;
	tInterprete->posActual = proxSentencia;
	return 1;
}


