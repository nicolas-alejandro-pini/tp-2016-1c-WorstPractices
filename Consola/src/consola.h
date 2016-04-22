/*
 * consola.h
 *
 *  Created on: 18/4/2016
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/config.h>

#define PATH_CONFIG "/usr/local/share/consola.config"
#define PARAM_PORT "DEST_PORT"
#define PARAM_IP "DEST_IP"
#define BUFFERSIZE 128

typedef struct t_console {
	char *pProgram;
	t_config *tConfigFile;
	int *pSockfd;
	struct sockaddr_in *tDest_addr;
} t_console;


/**
 * @NAME: create_console
 * @PRE:  un puntero a una estructura tipo t_console
 * @POST: Inicializa coneccion segun archivo de config.
 */
int create_console(t_console* tConsole);

/**
 * @NAME: connect_console
 * @PRE: un puntero a una estructura tipo t_console
 * @POST: conecta al nucleo. Devuelve -1 si hubo error
 */
int connect_console(t_console* tConsole);

/**
 * @NAME: destroy_console
 * @PRE:  un puntero a una estructura tipo t_console
 * @POST: libera memoria de la estructura t_console
 */
void destroy_console(t_console* tConsole);

/**
 * @NAME: setConsole_mode
 * @PRE:  un puntero a una estructura tipo t_console, cantidad de argumentos, array de argumentos
 * @POST: setea estado INPUT o FILE de la consola.
 */
int load_program(t_console* tConsole, int argc, char* argv[]);


/* AUXILIAR
 */
int server_test();

#endif /* CONSOLA_H_ */
