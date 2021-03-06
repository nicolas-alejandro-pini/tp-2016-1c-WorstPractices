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
#define CFGFILE		"cpu.conf"

/*Definicion de Parametros de Conexiones*/
#define OK					100
#define ERROR				101
#define QUIENSOS			102
#define CONNECTCPU			104
#define CONNECTUMC			105
#define ANSIPROG			500
#define UMCINSTRUCCION		600

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
 Estructuras del CPU
 ============================================================================
 */




