
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
#include <commons/log.h>
#include <commons/sockets.h>
#include <commons/socketsIPCIRC.h>
#include <commons/ipctypes.h>
#include <commons/pcb.h>
#include <commons/config.h>
#include <commons/serializador.h>
#include <commons/parser/parser.h>
#include <commons/parser/metadata_program.h>

/*Archivos de Configuracion*/
#define CFGFILE		"cpu.conf"

//Estructuras del CPU//

typedef struct{
	char* ipNucleo;		// Ip del Nucleo para conectarme //
	int puertoNucleo;	// Puerto del Nucleo //
	char* ipUmc;		// Ip del UMC //
	int puertoUmc;		// Puerto del UMC //
	int quantum;		// Quamtum del CPU //
	int sockNucleo;		// Socket para conexion con el nucleo //
	int sockUmc;		// Socket para conexion con el umc //
	int socketMax;		// Contiene el ultimo socket conectado//
	int salir;			// Flaf para indicar el fin del programa //
} t_configCPU;


typedef struct{
	char* nombre;           /*Nombre del semaforo*/
	char* valor; 			/*Valor del semaforo*/
} stSemaforo;

typedef struct{
	t_nombre_compartida nombre;		/*Nombre del semaforo*/
	int valor;						/*Valor del semaforo*/
} stSharedVar;

typedef struct{
	char* nombre;	/*Nombre del dispositivo*/
	int tiempo;		/*Tiempo de espera*/
} stIO;

#define TAMANIOVARIABLES 4;
