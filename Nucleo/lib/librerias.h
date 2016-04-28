#include <dirent.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <commons/config.h>

/*Archivos de Configuracion*/
#define CFGFILE		"nucleo.conf"

/*Definicion de Parametros de Conexiones comunes*/
#define OK					100
#define ERROR				101
#define QUIENSOS			102

/*Definicion de Parametros de Conexiones Consola*/
#define CONNECTCONSOLA		103
#define SENDANSISOP         106

/*Definicion de Parametros de Conexiones UMC*/
#define CONNECTNUCLEO		400
#define UMCOK				401
#define UMCQUIENSOS			402

/*Definicion de Parametros de Conexiones CPU*/
#define CONNECTCPU			104
#define EXECANSISOP			105

/*Definicion de Parametros para el Archivo Log*/
#define OK_LOG		51
#define ERROR_LOG	52
#define WARNING_LOG	53
#define INFO_LOG	54
#define DEBUG_LOG	55

/*Definicion de MACROS*/
#define LONGITUD_MAX_DE_CONTENIDO 	1024
#define UNLARGO 					255
#define LARGOLOG					2500

/*
 ============================================================================
 Estructuras del nucleo
 ============================================================================
 */
typedef struct {
	int socket;	/* Socket de CPU. */
}stCPUConectado;




