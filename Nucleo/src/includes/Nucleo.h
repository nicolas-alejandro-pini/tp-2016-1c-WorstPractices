#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/socketsIPCIRC.h>
#include <commons/ipctypes.h>
#include <commons/serializador.h>
#include <commons/parser/metadata_program.h>
#include </usr/include/semaphore.h>
#include <commons/pcb.h>
#include <commons/log.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef NUCLEO_H_
#define NUCLEO_H_

/*Archivos de Configuracion*/
#define CFGFILE		"nucleo.conf"

/*Definicion de MACROS*/
#define LONGITUD_MAX_DE_CONTENIDO 	1024
#define UNLARGO 					255
#define LARGOLOG					2500

/*Definicion de macros para Inotify*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

/*
 ============================================================================
 Estructuras del nucleo
 ============================================================================
 */
typedef struct {
	char* miIP; 			/* Mi direccion de IP. Ej: <"127.0.0.1"> */
	int miPuerto; 			/* Mi Puerto de escucha */
	char* ipUmc; 			/* Direccion IP de conexion a la UMC.*/
	int puertoUmc; 			/* Puerto de escucha */
	int sockEscuchador; 	/* Socket con el que escucho. */
	int sockUmc; 			/* Socket de comunicacion con la UMC. */
	int quantum; 			/* Quantum de tiempo para ejecucion de rafagas. */
	int quantumSleep; 		/* Retardo en milisegundos que el nucleo esperara luego de ejecutar cada sentencia. */
	int stackSize; 			/* Tamaño en páginas del Stack */
	t_list *dispositivos; 	/* Lista de stDispositivos*/
	t_list *consolas_activas;/* Lista de consolas conectadas */
	int fdMax; 				/* Numero que representa al mayor socket de fds_master. */
	int fdMax_ant; 			/*Numero que representa al mayor socket de fds_master anterior al proximo. */
	int tamanio_paginas; 	/*Tamaño de paginas configurado en la UMC*/
	int pidCounter;
	int salir; 				/*Indica si debo o no salir de la aplicacion. */
	char* path_conf;
} stEstado;

typedef struct {
	char* nombre; 			/*Nombre del dispositivo de I/O*/
	char* retardo; 			/*Retardo en milisegundos*/
	t_queue* rafagas; 		/*Cola de rafagas de ejecucion*/
	pthread_mutex_t mutex;	/*Sem Mutex*/
	pthread_mutex_t empty;	/*Sem Empty*/
	int numInq;
} stDispositivo;

typedef struct {
	uint32_t pid; 		/*PID del proceso que realiza el pedido de I/O*/
	int unidades; 		/*Unidades de ejecucion */
} stRafaga;

typedef struct {
	uint32_t pid;
	int socket;
} stConsola;

t_list *listaSem; 		/*Lista de todos los semaforos del sistema*/
t_list *listaSharedVars; /* Lista con las variables compartidas*/

void threadCPU(void *argumentos);
void cerrarSockets(stEstado *elEstadoActual);
void finalizarSistema(stMensajeIPC *unMensaje, int unSocket, stEstado *unEstado);
int calcular_cantidad_paginas(int size_programa, int tamanio_paginas);
void threadDispositivo(stDispositivo* unDispositivo);
stEstado obtenerEstadoActual();

#endif

