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
#include <commons/parser/parser.h>

void init_console(t_console* tConsole){
	tConsole->pProgram = NULL;
	tConsole->pSockfd = NULL;
	tConsole->tConfigFile = NULL;
	tConsole->tDest_addr = NULL;
}

int create_console(t_console* tConsole, char *path_config){

	int sockfd;

	if(!path_config)
		return EXIT_FAILURE;

	if(tConsole)
	{
		tConsole->pSockfd = (int*) malloc(sizeof(int));
		tConsole->tDest_addr = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
		tConsole->tConfigFile = config_create(path_config);
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

	liberarHeaderIPC(hQuienSos);
	liberarHeaderIPC(hConnectConsola);
	liberarHeaderIPC(hConfirm);
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

	liberarHeaderIPC(header);
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

int load_program(t_console* tConsole, char* argv)
{
	FILE* body = NULL;
	char buffer[BUFFERSIZE];
	char *program = NULL;

	// Abro el archivo arg de entrada
	if(!argv)
		return EXIT_FAILURE;

	// Si no esta definida la consola
	if(!tConsole)
		return EXIT_FAILURE;

	body = fopen(argv, "r");

	while(fgets(buffer, BUFFERSIZE, body)){

		if(!program)
		{
			program = malloc(strlen(buffer)+1);
			strcpy(program, buffer);
		}
		else
		{
			program = realloc( program, strlen(program)+strlen(buffer)+1);
			strcat(program, buffer);
		}
	}
	fclose(body);

	// Copio programa a la estructura de la consola
	tConsole->pProgram = program;

	return EXIT_SUCCESS;
}

int recv_print(t_console* tConsole){
	stHeaderIPC unMensaje;
	unMensaje.tipo = ERROR;
	t_valor_variable valor;
	char * texto;

	while(unMensaje.tipo != FINPROGRAMA){

		/* Valido cierre de conexion con el nucleo */
		if(recibirHeaderIPC(*(tConsole->pSockfd), &unMensaje)<=0)
			return EXIT_FAILURE;

		switch(unMensaje.tipo)
		{
			case IMPRIMIR:
				//Leo del socket el valor de la variable
				log_info("Recibo un mensaje de IMPRIMIR");
				recv(*(tConsole->pSockfd), &valor, sizeof(t_valor_variable), 0);
				log_info("IMPRIMIR VARIABLE [%d]", valor);
				break;
			case IMPRIMIRTEXTO:
				log_info("Recibo un mensaje de IMPRIMIR TEXTO");
				recv(*(tConsole->pSockfd), &texto, unMensaje.largo, 0);
				log_info("IMPRIMIR [%s]", texto);
				break;
			case FINPROGRAMA:
				log_info("Finalizando consola...\n");
				break;
			default:
				log_info("Me mandaron FRUTA");
				break;
		}
	}

	return EXIT_SUCCESS;
}


