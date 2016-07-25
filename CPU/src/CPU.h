#ifndef CPU_H_
#define CPU_H_

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
	char* nombre;	/*Nombre del dispositivo*/
	int tiempo;		/*Tiempo de espera*/
} stIO;

t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable );
stPosicion *obtenerPosicion(t_puntero direccion_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);
void asignar(t_puntero direccion_variable, t_valor_variable valor );
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
int asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta etiqueta);
void llamarFuncionConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor_mostrar);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void wait(t_nombre_semaforo identificador_semaforo);
void signal_cpu(t_nombre_semaforo identificador_semaforo);
void finalizar();
void cargarConf(t_configCPU* config,char* file_name);
int cpuHandShake (int socket, int tipoHeader);
int cpuConectarse(char* IP, int puerto, char* aQuien);
void cerrarSockets(t_configCPU *configuracionInicial);
int cargarPCB(void);
int calcularPaginaInstruccion (int paginaLogica);
char* getInstruccion (int startRequest, int sizeRequest);
int ejecutarInstruccion(void);
int devolverPCBalNucleo(void);
int cambiarContextoUMC(uint32_t pid);
void reemplazarBarraN(char* buffer);



#define TAMANIOVARIABLES 4

#endif
