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
#include "../lib/librerias.h"
#include "../lib/sockets.c"
#include "../lib/socketsIPCIRC.c"
#include "../lib/fComunes.c"

int create_console(t_console* tConsole){

	int sockfd;

	if(tConsole)
	{
		tConsole->pSockfd = (int*) malloc(sizeof(int));
		tConsole->tDest_addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
		tConsole->tConfigFile = config_create(PATH_CONFIG);
	}
	else
		return -1;

	if(tConsole->tConfigFile == NULL)
	{
		perror("config_create");
		return -1;
	}
	// Valido parametros del archivo de configuracion.
	if(!config_has_property(tConsole->tConfigFile, PARAM_PORT)
	|| !config_has_property(tConsole->tConfigFile, PARAM_IP)) {
		return -1;
	}
	// Creo socket para conectar la consola al nucleo
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		return -1;
	}
	*(tConsole->pSockfd) = sockfd;

	// Destination address
	tConsole->tDest_addr->sin_family = AF_INET;
	tConsole->tDest_addr->sin_port = htons(config_get_int_value(tConsole->tConfigFile, PARAM_PORT));
	(tConsole->tDest_addr->sin_addr).s_addr = inet_addr(config_get_string_value(tConsole->tConfigFile, PARAM_IP));
	memset(tConsole->tDest_addr->sin_zero, '\0', 8);

	//printf("Consola creada.\n");
	return 0;
}

int connect_console(t_console* tConsole){

	if(!(tConsole->pSockfd) || !(tConsole->tDest_addr)){
		return -1;
	}
	if((connect(*(tConsole->pSockfd), (struct sockaddr *) tConsole->tDest_addr, sizeof(struct sockaddr))) == -1)
	{
		perror("connect");
		return -1;
	}

	return 0;
}

int handshake_console(t_console* tConsole){
	stMensajeIPC handshake;

    if(!recibirMensajeIPC(*(tConsole->pSockfd), &handshake))
    {
    	perror("Handshake: Error. se cerro la coneccion.");
    	return -1;
    }

	if(handshake.header.tipo != QUIENSOS)
	{
		perror("Handshake: Error. Mensaje desconocido");
		return -1;
	}

	if(!enviarMensajeIPC(*(tConsole->pSockfd),nuevoHeaderIPC(CONNECTCONSOLA),"MSGOK")){
		perror("Handshake: Error. no se pudo responder al Nucleo.");
		return -1;
	}

	printf("Conectado al Nucleo...\n");
    return 0;
}

int send_program(t_console* tConsole){

	// Envio a bloques de LONGITUD_MAX_DE_CONTENIDO
	int program_length = strlen(tConsole->pProgram);
	int program_sent = 0;
	char str[LONGITUD_MAX_DE_CONTENIDO+1];

	// Defino header
	stHeaderIPC header = nuevoHeaderIPC(CONNECTCONSOLA);
	header.largo = LONGITUD_MAX_DE_CONTENIDO;

	while(program_sent < program_length)
	{
		strncpy(str, tConsole->pProgram + program_sent, LONGITUD_MAX_DE_CONTENIDO);
		str[LONGITUD_MAX_DE_CONTENIDO] = '\0';

		if(!enviarMensajeIPC(*(tConsole->pSockfd),nuevoHeaderIPC(SENDANSISOP),str))
		{
			perror("send_program: Error. no se pudo enviar el programa al Nucleo.");
			return -1;
		}
		else
			program_sent+=LONGITUD_MAX_DE_CONTENIDO;
	}

	printf("Programa enviado...\n");
	return 0;
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
		return -1;
	}
	tConsole->pProgram = program;

	// Imprimo programa por stdout
	printf("\nPrograma: \n[%s]\n", program);
	return 0;
}




