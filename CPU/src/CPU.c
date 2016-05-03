/*
 ============================================================================
 Name        : CPU.c
 Author      : Ezequiel Martinez
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "../lib/librerias.h"
#include "commons/sockets.h"
#include "commons/socketsIPCIRC.h"

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

//Variables Globales//

fd_set fds_master;		/* Lista de todos mis sockets. */
fd_set read_fds;		/* Sublista de fds_master. */


int nucleo = 0;
int umc = 0;
int pistaActual = 0;
int sectorActual = 1;
int SocketAnterior = 0;

/*
 ============================================================================
 Name        : cargarConf
 Author      : Ezequiel Martinez
 Inputs      : Recibe archivo de configuracion y nombre del archivo.
 Outputs     : N/A
 Description : Funcion para cargar los parametros del archivo de configuración
 ============================================================================
 */
void cargarConf(t_configCPU* config,char* file_name){

	t_config* miConf = config_create ("cpu.conf"); /*Estructura de configuracion*/

		if (config_has_property(miConf,"NUCLEO_IP")) {
			config->ipNucleo = config_get_string_value(miConf,"NUCLEO_IP");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","NUCLEO_IP");
			exit(-2);
		}

		if (config_has_property(miConf,"PUERTO_NUCLEO")) {
			config->puertoNucleo = config_get_int_value(miConf,"PUERTO_NUCLEO");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_NUCLEO");
			exit(-2);
		}

		if (config_has_property(miConf,"UMC_IP")) {
			config->ipUmc= config_get_string_value(miConf,"UMC_IP");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","UMC_IP");
			exit(-2);
		}

		if (config_has_property(miConf,"PUERTO_UMC")) {
			config->puertoUmc = config_get_int_value(miConf,"PUERTO_UMC");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_UMC");
			exit(-2);
		}

		if (config_has_property(miConf,"QUANTUM")) {
			config->quantum = config_get_int_value(miConf,"QUANTUM");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","QUANTUM");
			exit(-2);
		}

}

/*
 =========================================================================================
 Name        : cpuHandShake
 Author      : Ezequiel Martinez
 Inputs      : Recibe el socket, un mensaje para identificarse y un tipo para el header.
 Outputs     : Retorna -1 en caso de error y si no hay error devuelve el socket.
 Description : Funcion para cargar los parametros del archivo de configuración
 =========================================================================================
 */
int cpuHandShake (int socket, char* mensaje, int tipoHeader)
{
	stMensajeIPC unMensaje;

	if(!recibirMensajeIPC(socket,&unMensaje)){
		printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
		fflush(stdout);
	}

	printf("HandShake mensaje recibido %d",unMensaje.header.tipo);

	if (unMensaje.header.tipo == QUIENSOS)
	{
		if(!enviarMensajeIPC(socket,nuevoHeaderIPC(tipoHeader),mensaje)){
			printf("No se pudo enviar el MensajeIPC\n");
			return (-1);
		}
	}

	if(!recibirMensajeIPC(socket,&unMensaje)){
			printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
			fflush(stdout);
			return (-1);
	}

	printf("HandShake: mensaje recibido %d",unMensaje.header.tipo);
	fflush(stdout);

	if(unMensaje.header.tipo == OK)
	{
		printf("Conexión establecida con id: %d...\n",tipoHeader);
		fflush(stdout);
		return socket;
	}
	else
		return (-1);
}

/*
 =========================================================================================
 Name        : cpuConectarse()
 Author      : Ezequiel Martinez
 Inputs      : Recibe IP y Puerto.
 Outputs     : Retorna -1 en caso de error y si no hay error devuelve el socket.
 Description : Realiza la conexión con un servidor.
 =========================================================================================
 */
int cpuConectarse(char* IP, int puerto, char* aQuien){

	int socket = 0;

	printf("Conectando con %s...\n",aQuien);
	fflush(stdout);
	socket = conectar(IP, puerto);

	// Inicio el handShake con el servidor //
	if (socket != -1)
	{
		if (cpuHandShake(socket, "SOYCPU", CONNECTCPU) != -1)
			return socket;
	}

	return (-1); // Retorna -1 si no se pudo crear el socket o fallo el handshake

}

/*
 =========================================================================================
 Name        : cerrarSockets()
 Author      : Ezequiel Martinez
 Inputs      : Recibe puntero a estructura.
 Outputs     : Retorna -1 en caso de error y si no hay error devuelve el socket.
 Description : Cierra todos los sockets existentes en la lista.
 =========================================================================================
 */
void cerrarSockets(t_configCPU *configuracionInicial){

	int unSocket;
	for(unSocket=3; unSocket <= configuracionInicial->socketMax; unSocket++)
		if(FD_ISSET(unSocket,&(fds_master)))
			close(unSocket);

	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));
}

/*
 =========================================================================================
 Name        : main()
 Author      : Ezequiel Martinez
 Inputs      : N/A.
 Outputs     : N/A.
 Description : Proceso principal del programa.
 =========================================================================================
 */
int main(void) {

	t_configCPU configuracionInicial;
	stMensajeIPC unMensaje;
	int unSocket;

	printf("CPU Application"); /* prints CPU Application */
	fflush(stdout);

	// Limpio las liastas //
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/***** Cargo la configuracion desde el archivo cpu.conf ********/
	cargarConf(&configuracionInicial, CFGFILE);

	/***** Lanzo conexión con el Nucleo ********/

	configuracionInicial.sockNucleo = cpuConectarse(configuracionInicial.ipNucleo, configuracionInicial.puertoNucleo, "Nucleo");

	if (configuracionInicial.sockNucleo != -1){
		FD_SET(configuracionInicial.sockNucleo,&(fds_master));
		configuracionInicial.socketMax = configuracionInicial.sockNucleo;
		SocketAnterior = configuracionInicial.socketMax;
		printf("OK - Nucleo conectado. \n");
		fflush(stdout);
		//loguear(OK_LOG,"Nucleo conectado","Nucleo"); TODO Agregar funcion de logueo.

	}	//Fin de conexion al Nucleo//


	/***** Lanzo conexión con el UMC ********/

	configuracionInicial.sockUmc = cpuConectarse(configuracionInicial.ipUmc, configuracionInicial.puertoUmc, "UMC");

	if (configuracionInicial.sockUmc > 0){

		FD_SET(configuracionInicial.sockUmc,&(fds_master));
		configuracionInicial.socketMax = configuracionInicial.sockUmc;
		SocketAnterior = configuracionInicial.socketMax;
		printf("OK - UMC conectada. \n");
		fflush(stdout);
		//loguear(OK_LOG,"Nucleo conectado","Nucleo"); TODO Agregar funcion de logueo.

	}
		//Fin de conexion al UMC//

	while(configuracionInicial.salir == 0)
	{
		read_fds = fds_master;
		if(seleccionar(configuracionInicial.socketMax,&read_fds,1) == -1)
		{
			printf("Error Preparando el Select\n");
			//loguear(ERROR_LOG,"Error preparando el select","CPU"); //TODO Funciones de logueo
			configuracionInicial.salir = 1;
		}

		for(unSocket=0;unSocket<=configuracionInicial.socketMax;unSocket++)
		{
			if(FD_ISSET(unSocket,&read_fds))
			{
				if (!recibirMensajeIPC(unSocket,&unMensaje))/* Si se cerro un Cliente. */
				{
					if (configuracionInicial.sockNucleo == unSocket)
					{
						printf("Se desconecto el Servidor\n"); fflush(stdout);
						//loguear(INFO_LOG,"Se perdio la conexion con el Nucleo","Nucleo");//TODO Funciones de logueo
						configuracionInicial.sockNucleo = -1;
						configuracionInicial.salir=1;

						FD_CLR(unSocket,&fds_master);
						close (unSocket);

						if (unSocket > configuracionInicial.socketMax){
							SocketAnterior = configuracionInicial.socketMax;
							configuracionInicial.socketMax = unSocket;
						}

						//loguear(INFO_LOG,"Se perdio conexion con Nucleo","Nucleo");//TODO Funciones de logueo

					}else if (configuracionInicial.sockUmc == unSocket)
					{
						printf("Se desconecto el UMC\n"); fflush(stdout);
						//loguear(INFO_LOG,"Se perdio la conexion con el UMC","UMC");//TODO Funciones de logueo
						configuracionInicial.sockUmc = -1;
						configuracionInicial.salir = 1;

						FD_CLR(unSocket,&fds_master);
						close (unSocket);

						if (unSocket > configuracionInicial.socketMax){
							SocketAnterior = configuracionInicial.socketMax;
							configuracionInicial.socketMax = unSocket;
						}

						//loguear(INFO_LOG,"Se perdio la conexion con el UMC","UMC");//TODO Funciones de logueo
					}
				}
				else
				{
					switch(unMensaje.header.tipo)
					{
						case ANSIPROG:

							printf("Respondiendo solicitud ANSIPROG...");

							enviarMensajeIPC(configuracionInicial.sockNucleo,nuevoHeaderIPC(OK),"CPU: Programa recibido.");

						break;


						case UMCINSTRUCCION:
						{
							printf("Respondiendo solicitud UMCINSTRUCCION...");

							enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(OK),"CPU: Recibi del UMC.");

						}
						break;
					}
				}

			}

		}
	}

	cerrarSockets(&configuracionInicial);
	printf("\nCPU: Fin del programa\n");

	return EXIT_SUCCESS;
}




