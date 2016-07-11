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
#include <unistd.h>
#include <commons/log.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/config.h>

//#define PATH_CONFIG "/usr/local/share/consola.config"
#define PATH_CONFIG "/home/utnso/workspace/tp-2016-1c-WorstPractices/Consola/consola.config"
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
 * @NAME: load_program
 * @PRE:  un puntero a una estructura tipo t_console, program ansisop por argumento
 * @POST: Lee archivo ansiop en modo interprete, o por parametro (path completo del archivo)
 */
int load_program(t_console* tConsole, int argc, char* argv[]);

/**
 * @NAME: handshake_console
 * @PRE:
 * @POST:
 */
int handshake_console(t_console* tConsole);

/**
 * @NAME: send_program
 * @PRE:
 * @POST:
 */
int send_program(t_console* tConsole);

int recv_print(t_console* tConsole);

/* AUXILIAR
 */
int server_test();

#endif /* CONSOLA_H_ */
