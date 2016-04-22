/*
 ============================================================================
 Name        : CPU.c
 Author      : Ezequiel Martinez
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <commons/config.h>

//Definiciones CPU//

#define CFGFILE		"cpu.conf"

//Estructuras del CPU//

typedef struct{
	char* miIP;			//Mi direccion de IP. Ej: <"127.0.0.1"> */
	int puertoCpu;		// Puerto de CPU //
	char* ipNucleo;		// Ip del Nucleo para conectarme //
	int puertoNucleo;	// Puerto del Nucleo //
	char* ipUmc;		// Ip del UMC //
	int puertoUmc;		// Puerto del UMC //
	int quantum;		// Quamtum del CPU //
} t_configCPU;

//Variables Globales//

int nucleo = 0;
int umc = 0;

//Funcion para cargar los parametros del archivo de configuración//

void cargarConf(t_configCPU* config,char* file_name){

	t_config* miConf = config_create ("cpu.conf"); /*Estructura de configuracion*/

		if (config_has_property(miConf,"CPU_IP")) {
			config->miIP = config_get_string_value(miConf,"CPU_IP");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","NUCLEO_IP");
			exit(-2);
		}

		if (config_has_property(miConf,"PUERTO_CPU")) {
			config->puertoCpu = config_get_int_value(miConf,"PUERTO_CPU");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_PROG");
			exit(-2);
		}

		if (config_has_property(miConf,"NUCLEO_IP")) {
			config->ipNucleo= config_get_int_value(miConf,"NUCLEO_IP");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_CPU");
			exit(-2);
		}

		if (config_has_property(miConf,"PUERTO_NUCLEO")) {
			config->puertoNucleo = config_get_int_value(miConf,"PUERTO_NUCLEO");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_PROG");
			exit(-2);
		}

		if (config_has_property(miConf,"UMC_IP")) {
			config->ipUmc= config_get_int_value(miConf,"UMC_IP");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_CPU");
			exit(-2);
		}

		if (config_has_property(miConf,"PUERTO_NUCLEO")) {
			config->puertoUmc = config_get_int_value(miConf,"PUERTO_NUCLEO");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_PROG");
			exit(-2);
		}

		if (config_has_property(miConf,"QUANTUM")) {
			config->quantum = config_get_int_value(miConf,"QUANTUM");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_PROG");
			exit(-2);
		}

}

//Funcion para obtener un socket//
int cpuDameSocket (char* IP, int puerto){
	//EML: Las IP y Puerto recibirlo por parametro.
	struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = inet_addr(IP);
		direccionServidor.sin_port = htons(puerto); //Le indico el puerto en el que escucha.

		int cliente = socket(AF_INET, SOCK_STREAM, 0);

		if(connect(cliente, (void*)&direccionServidor, sizeof(direccionServidor))!=0)
			return 0;//El connect si devuelve 0 es por un error.
		else
			return cliente;
}

int cpuConectarseAlNucleo(char* IP, int puerto){

	int idNucleo = 0;
	puts("Conectando al nucleo...\n");
	idNucleo = cpuDameSocket(IP, puerto);
	return idNucleo;
}

int cpuConectarseAlUmc(char* IP, int puerto){

	int idUMC = 0;
	puts("Conectando al UMC...\n");
	idUMC = cpuDameSocket(IP, puerto);
	return idUMC;
}

void mensajesDesdeNucleo(void){
	int bytesRecibidos=0;
	char* buffer = malloc(1000);

	while(1){
		bytesRecibidos = recv(nucleo, buffer,1000,0);

		if (bytesRecibidos <=0){
			perror("No hay nadie conectado o se desconeto.");
		}

		buffer[bytesRecibidos]='\0';
		printf("Me llegaron %d bytes con %s \n", bytesRecibidos, buffer);
	}

}

void mensajesDesdeUmc(void){
	int bytesRecibidos=0;
	char* buffer = malloc(1000);

	while(1){
		bytesRecibidos = recv(nucleo, buffer,1000,0);

		if (bytesRecibidos <=0){
			perror("No hay nadie conectado o se desconeto.");
		}

		buffer[bytesRecibidos]='\0';
		printf("Me llegaron %d bytes con %s \n", bytesRecibidos, buffer);
	}

}

int main(void) {

	pthread_t hiloNucleo;
	pthread_t hiloUmc;
	t_configCPU configuracionInicial;

	puts("CPU Application"); /* prints CPU Application */

	cargarConf(&configuracionInicial, CFGFILE); // Cargo la configuracion desde el archivo cpu.conf

	// Lanzo conexion con el nucleo //

	nucleo = cpuConectarseAlNucleo(configuracionInicial.ipNucleo, configuracionInicial.puertoNucleo); //EML: Pendiente- Leer un .conf y pasar ip y puerto.

	if (nucleo != 0){
		puts("Conectado con el nucleo.");

		//Lanzo el hilo del Nucleo una vez establecida la conexion

		pthread_create(&hiloNucleo,NULL,(void*)mensajesDesdeNucleo, NULL);

		while(1){
			char mensaje[1000];
			scanf("%s", mensaje);
			send (nucleo, mensaje, strlen(mensaje), 0); //Pruebo comunicacion con un server.
		}
	}else
		perror("Error al conectarme con el nucleo.");
	//Fin de conexion al Nucleo//

	//Lanzo conexión con el umc//

	umc = cpuConectarseAlUmc(configuracionInicial.ipUmc, configuracionInicial.ipUmc);

	if (umc != 0){
			puts("Conectado con el umc.");

			//Lanzo el hilo del Nucleo una vez establecida la conexion

			pthread_create(&hiloUmc,NULL,(void*)mensajesDesdeUmc, NULL);

			while(1){
				char mensaje[1000];
				scanf("%s", mensaje);
				send (umc, mensaje, strlen(mensaje), 0); //Pruebo comunicacion con un server.
			}
		}else
			perror("Error al conectarme con el umc.");
		//Fin de conexion al Nucleo//

	return EXIT_SUCCESS;
}




