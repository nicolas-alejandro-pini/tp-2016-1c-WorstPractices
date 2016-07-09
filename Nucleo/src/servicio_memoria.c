/*
 * servicio_memoria.c
 *
 *  Created on: 1/6/2016
 *      Author: utnso
 */
#include "includes/servicio_memoria.h"

int inicializar_programa(stPCB *unPCB, char* unPrograma, int socket_umc) {

	stPageIni *unInicioUMC;
	t_paquete paquete;
	stHeaderIPC *unHeaderIPC;

	// Le indico a la UMC que inicializo el programa
	unHeaderIPC = nuevoHeaderIPC(INICIALIZAR_PROGRAMA);
	if (!enviarHeaderIPC(socket_umc, unHeaderIPC)) {
	 	liberarHeaderIPC(unHeaderIPC);
	 	close(socket_umc);
		return EXIT_FAILURE;
	}
	liberarHeaderIPC(unHeaderIPC);

	unInicioUMC = malloc(sizeof(stPageIni));
	unInicioUMC->processId = unPCB->pid;
	unInicioUMC->cantidadPaginas = unPCB->cantidadPaginas;
	unInicioUMC->programa = unPrograma;

	crear_paquete(&paquete, INICIALIZAR_PROGRAMA);
	serializar_inicializar_programa(&paquete, unInicioUMC);

	if (enviar_paquete(socket_umc, &paquete)) {
		printf("No se pudo enviar paquete de inicio de programa para PID [%d]", unPCB->pid);
		close(socket_umc);
		return EXIT_FAILURE;
	}
	free_paquete(&paquete);
	free(unInicioUMC);

	unHeaderIPC = nuevoHeaderIPC(ERROR);// por default reservo memoria con tipo ERROR
	if (!recibirHeaderIPC(socket_umc, unHeaderIPC)) {
		log_error("UMC handshake error - No se pudo recibir mensaje de confirmacion");
		liberarHeaderIPC(unHeaderIPC);
		close(socket_umc);
		return EXIT_FAILURE;
	}
	liberarHeaderIPC(unHeaderIPC);

	if (unHeaderIPC->tipo == OK) {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
