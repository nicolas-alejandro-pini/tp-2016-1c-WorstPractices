/*
 * servicio_memoria.c
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */
#include "includes/servicio_memoria.h"

int inicializar_programa(int pid, int cantidad_paginas, char* programa, int socket_umc) {

	stPageIni *unInicioUMC;
	t_paquete paquete;
	stMensajeIPC unMensajeIPC;
	stHeaderIPC *stHeaderIPC;

	stHeaderIPC = nuevoHeaderIPC(INICIALIZAR_PROGRAMA);
	if (!enviarHeaderIPC(socket_umc, stHeaderIPC)) {
		liberarHeaderIPC(stHeaderIPC);
		close(socket_umc);
		return EXIT_FAILURE;
	}

	stHeaderIPC = nuevoHeaderIPC(OK);
	if (!recibirHeaderIPC(socket_umc, stHeaderIPC)) {
		log_error("UMC handshake error - No se pudo recibir mensaje de confirmacion");
		liberarHeaderIPC(stHeaderIPC);
		close(socket_umc);
		return EXIT_FAILURE;
	}

	unInicioUMC = malloc(sizeof(stPageIni)) + strlen(programa + 1);
	unInicioUMC->processId = pid;
	unInicioUMC->programa = programa;
	unInicioUMC->cantidadPaginas = cantidad_paginas;

	crear_paquete(&paquete, INICIALIZAR_PROGRAMA);
	serializar_inicializar_programa(&paquete, unInicioUMC);

	if (enviar_paquete(socket_umc, &paquete)) {
		printf("No se pudo enviar paquete de inicio de programa para PID [%d]", pid);
		close(socket_umc);
		return EXIT_FAILURE;
	}
	free_paquete(&paquete);
	free(unInicioUMC);

	if (!recibirHeaderIPC(socket_umc, stHeaderIPC)) {
		log_error("UMC handshake error - No se pudo recibir mensaje de confirmacion");
		liberarHeaderIPC(stHeaderIPC);
		close(socket_umc);
		return EXIT_FAILURE;
	}

	if (stHeaderIPC->tipo == OK) {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
