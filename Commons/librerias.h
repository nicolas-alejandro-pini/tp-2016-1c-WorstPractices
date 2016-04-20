/*------------------------------------------------------------------------------*/
/*                --------------------------------------------			*/
/*           		        Libreria.h					*/
/*                --------------------------------------------			*/
/*------------------------------------------------------------------------------*/

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

/*Archivos de Configuracion*/
#define CFGFILE		"configuracion.conf"
/*Definicion de Parametros de Conexiones*/

#define ERROR				101
#define OK				103
#define QUIENSOS			105
#define CONNECTCLIENTE			116

	  

	

/*Definicion de Parametros de Respuesta*/

#define CONNECT		0x01	    /* Usa este tipo para el connect*/
#define SEARCH		0x04

#define NOTFOUND	60
#define ADD		0x07
#define EXIT		0x14

/*Definicion de Parametros para el Archivo Log*/

#define OK_LOG		51
#define ERROR_LOG	52
#define WARNING_LOG	53
#define INFO_LOG	54
#define DEBUG_LOG	55

/*Definicion de MACROS*/

#define TAMANIOMAXPALABRAS 2048
#define TAMDATOS 100
#define TAMBUFFER 255
#define LONGITUD_MAX_DE_CONTENIDO 1024
#define UNLARGO 2048
#define TAMSECTOR 512
#define TAMID 10

/*--------------------------------------------Estructuras----------------------------------------*/
/*												 */   

typedef struct {
	int socket;	/* Socket con el que me comunico con él. */
	char dispositivo[TAMDATOS];
	int estado;
}stConectados;

typedef struct {
	int unServidor;
	int tipoDeAccion;
	char elResultado[LONGITUD_MAX_DE_CONTENIDO];
}stResultados;

typedef struct {		/*Estructura para el manejo de los sectores del Disco*/

	int  numSector;
	char infoSector[TAMSECTOR];
} stSector;



typedef struct{

	int idSector;
}stSectorOFT;



