/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "consola.h"
#include <commons/config.h>
#include <commons/sockets.h>
#include <commons/socketsIPCIRC.h>
#include <commons/ipctypes.h>

int create_console(t_console* tConsole){

	int sockfd;

	if(tConsole)
	{
		tConsole->pSockfd = (int*) malloc(sizeof(int));
		tConsole->tDest_addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
		tConsole->tConfigFile = config_create(PATH_CONFIG);
	}
	else
		return EXIT_FAILURE;

	if(tConsole->tConfigFile == NULL)
	{
		perror("config_create");
		return EXIT_FAILURE;
	}
	// Valido parametros del archivo de configuracion.
	if(!config_has_property(tConsole->tConfigFile, PARAM_PORT)
	|| !config_has_property(tConsole->tConfigFile, PARAM_IP)) {
		return EXIT_FAILURE;
	}
	// Creo socket para conectar la consola al nucleo
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		return EXIT_FAILURE;
	}
	*(tConsole->pSockfd) = sockfd;

	// Destination address
	tConsole->tDest_addr->sin_family = AF_INET;
	tConsole->tDest_addr->sin_port = htons(config_get_int_value(tConsole->tConfigFile, PARAM_PORT));
	(tConsole->tDest_addr->sin_addr).s_addr = inet_addr(config_get_string_value(tConsole->tConfigFile, PARAM_IP));
	memset(tConsole->tDest_addr->sin_zero, '\0', 8);

	//printf("Consola creada.\n");
	return EXIT_SUCCESS;
}

int connect_console(t_console* tConsole){

	if(!(tConsole->pSockfd) || !(tConsole->tDest_addr)){
		return -1;
	}
	if((connect(*(tConsole->pSockfd), (struct sockaddr *) tConsole->tDest_addr, sizeof(struct sockaddr))) == -1)
	{
		perror("connect");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int handshake_console(t_console* tConsole){
	stHeaderIPC *hQuienSos = nuevoHeaderIPC(ERROR);
	stHeaderIPC *hConnectConsola = nuevoHeaderIPC(CONNECTCONSOLA);
	stHeaderIPC *hConfirm = nuevoHeaderIPC(ERROR);

    if(!recibirHeaderIPC(*(tConsole->pSockfd), hQuienSos))
    {
    	perror("Handshake: Error. se cerro la coneccion.");
    	return EXIT_FAILURE;
    }

	if(hQuienSos->tipo != QUIENSOS)
	{
		perror("Handshake: Error. Mensaje desconocido");
		return EXIT_FAILURE;
	}

	if(!enviarHeaderIPC(*(tConsole->pSockfd), hConnectConsola)){
		perror("Handshake: Error. no se pudo responder al Nucleo.");
		return EXIT_FAILURE;
	}

	if(!recibirHeaderIPC(*(tConsole->pSockfd), hConfirm))
	{
		perror("Handshake: Error, de comunicacion.");
		return EXIT_FAILURE;
	}

	if(hConfirm->tipo != OK)
	{
		perror("Handshake: Error, se esperaba confirmacion del handshake");
	}

	//printf("Conectado al Nucleo...\n");
    return EXIT_SUCCESS;
}

int send_program(t_console* tConsole){

	// Envio a bloques de LONGITUD_MAX_DE_CONTENIDO
	unsigned long program_length = strlen(tConsole->pProgram);

	// Defino header
	stHeaderIPC *header = nuevoHeaderIPC(SENDANSISOP);
	header->largo = program_length + 1;

	if(!enviarMensajeIPC(*(tConsole->pSockfd),header,tConsole->pProgram))
	{
		perror("send_program: Error. no se pudo enviar el programa al Nucleo.");
		return EXIT_FAILURE;
	}

	//printf("Programa enviado...\n");
	return EXIT_SUCCESS;
}

void destroy_console(t_console* tConsole){

	close(*(tConsole->pSockfd));

	if(tConsole->tConfigFile)
		config_destroy(tConsole->tConfigFile);
	if(tConsole->pSockfd)
		free(tConsole->pSockfd);
	if(tConsole->tDest_addr)
		free(tConsole->tDest_addr);
	if(tConsole->pProgram)
		free(tConsole->pProgram);

}

int load_program(t_console* tConsole, int argc, char* argv[])
{
	FILE* body;
	char *program = (char*) malloc(10);
	char buffer[BUFFERSIZE];

	if(argc < 2)
		body = fopen(argv[0], "r");    // Hashbang
	else
		body = fopen(argv[1], "r");    // Consola <path>

	while(fgets(buffer, BUFFERSIZE, body)){
		program = (char*) realloc( program, strlen(program)+1+strlen(buffer));
		if(program)
			strcat(program, buffer);  /* concatena un salto de linea cada vez */
	}
	fclose(body);

	// Agrego el programa a t_console
	if(!tConsole)
	{
		free(program);
		return EXIT_FAILURE;
	}
	tConsole->pProgram = program;

	return EXIT_SUCCESS;
}

int recv_print(t_console* tConsole){
	stMensaje unMensaje;
	unMensaje.header.tipo = ERROR;

	while(unMensaje.header.tipo != KILLPID){

		recibirMensaje(*(tConsole->pSockfd), &unMensaje);

		switch(unMensaje.header.tipo)
		{
			case IMPRIMIR:
				break;
			case IMPRIMIRTEXTO:
				break;
			case SENDPID:
				break;
			case KILLPID:
					printf("Finalizando consola...\n");
				break;
			default:
				break;
		}
	}

	return EXIT_SUCCESS;
}


