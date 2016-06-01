/*
 * servicio_memoria.c
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */
#include "servicio_memoria.h"

int inicializar_programa(int pid, int cantidad_paginas, char* programa, int socket_umc, int pagina_inicial) {

	stPageIni *unInicioUMC;
	t_paquete paquete;
	stMensajeIPC unMensajeIPC;

	unInicioUMC = malloc(sizeof(stPageIni)) + strlen(programa + 1);
	unInicioUMC->processId = pid;
	unInicioUMC->programa = programa;
	unInicioUMC->cantidadPaginas = cantidad_paginas;

	crear_paquete(&paquete, INICIALIZAR_PROGRAMA);
	serializar_inicializar_programa(&paquete, unInicioUMC);

	if (enviar_paquete(socket_umc, &paquete)) {
		printf("No se pudo enviar paquete de inicio de programa para PID [%d]",pid);
		close(socket_umc);
		return -1;
	}
	free_paquete(&paquete);
	free(unInicioUMC);
	if(!recibirMensajeIPC(socket_umc,&unMensajeIPC)){
		printf("No se recibio confirmacion para el inicio de programa ");
		close(socket_umc);
		return -1;
	}

	if(unMensajeIPC.header.tipo == OK){
		pagina_inicial = atoi(unMensajeIPC.contenido);
		return 0;
	}
	return -1;
}
