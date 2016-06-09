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

/* Listas globales */
fd_set fds_master; /* Lista de todos mis sockets.*/
fd_set read_fds; /* Sublista de fds_master.*/

/*
 ============================================================================
 Estructuras del nucleo
 ============================================================================
 */
typedef struct {
	char* miIP;             /* Mi direccion de IP. Ej: <"127.0.0.1"> */
	int miPuerto;			/* Mi Puerto de escucha */
	char* ipUmc;            /* direccion IP de conexion a la UMC.*/
	int puertoUmc;			/* Puerto de escucha */
	int sockEscuchador;		/* Socket con el que escucho. */
	int sockUmc;			/* Socket de comunicacion con la UMC. */
	int quantum;			/* Quantum de tiempo para ejecucion de rafagas. */
	int quantumSleep;		/* Retardo en milisegundos que el nucleo esperara luego de ejecutar cada sentencia. */
	int stackSize;			/* Tamaño en páginas del Stack */
	t_list *dispositivos; 	/* Lista de stDispositivos*/
	t_list *semaforos;		/* Lista de los semaforos con sus valores*/
	t_list *sharedVars;		/* Lista con las variables compartidas*/
	int fdMax;              /* Numero que representa al mayor socket de fds_master. */
	int salir;              /* Indica si debo o no salir de la aplicacion. */
} stEstado;

struct thread_cpu_arg_struct {
	stEstado *estado;
    int socketCpu;
};

typedef struct{
	char* nombre;           /*Nombre del dispositivo de I/O*/
	char* retardo;			/*Retardo en milisegundos*/
	t_queue* rafagas;		/*Cola de rafagas de ejecucion*/
} stDispositivo;

typedef struct{
	uint32_t pid;           /*PID del proceso que realiza el pedido de I/O*/
	int unidades;			/*Unidades de ejecucion */
} stRafaga;

typedef struct{
	char* nombre;           /*Nombre del semaforo*/
	char* valor; 			/*Valor del semaforo*/
} stSemaforo;

typedef struct{
	char* nombre;           /*Nombre del semaforo*/
	char* valor; 			/*Valor del semaforo*/
} stSharedVar;

void threadCPU(void *argumentos);
void cerrarSockets(stEstado *elEstadoActual);
void finalizarSistema(stMensajeIPC *unMensaje, int unSocket, stEstado *unEstado);


#endif

