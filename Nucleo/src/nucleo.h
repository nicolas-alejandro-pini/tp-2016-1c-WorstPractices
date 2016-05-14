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
	char* miIP;             /* Mi direccion de IP. Ej: <"127.0.0.1"> */
	int miPuerto;			/* Mi Puerto de escucha */
	char* ipUmc;            /* direccion IP de conexion a la UMC.*/
	int puertoUmc;			/* Puerto de escucha */
	int sockEscuchador;		/* Socket con el que escucho. */
	int sockUmc;			/* Socket de comunicacion con la UMC. */
	int quantum;			/* Quantum de tiempo para ejecucion de rafagas. */
	int quantumSleep;		/* Retardo en milisegundos que el nucleo esperara luego de ejecutar cada sentencia. */
	char** ioIds;			/* Array con los dispositivos conectados*/
	char** ioSleep;			/* Array con los retardos por cada dispositivo conectados (en milisegundos)*/
	char** semIds;			/* Array con identificadores por cada semaforo*/
	char** semInit;			/* Array con los valores iniciales de los semaforos conectados*/
	char** sharedVars;		/* Array con las variables compartidas*/
	int fdMax;              /* Numero que representa al mayor socket de fds_master. */
	int salir;              /* Indica si debo o no salir de la aplicacion. */
} stEstado;

typedef struct stCPUConectado{
	int socket;	/* Socket de CPU. */
}stCPUConectado;

#endif

