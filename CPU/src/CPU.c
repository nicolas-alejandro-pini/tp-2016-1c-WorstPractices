/*
 ============================================================================
 Name        : CPU.c
 Author      : Ezequiel Martinez
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
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
#include "commons/sockets.h"
#include <commons/socketsIPCIRC.h>
#include <commons/ipctypes.h>
#include <commons/pcb.h>
#include <commons/config.h>
#include "parser/parser.h"
#include "parser/metadata_program.h"


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
	int nroPagina;		// Indica el numero de pagina //
	int size;			// Tamaño//
	int offSet;			// offSet //
} t_posicion;


//Variables Globales//

fd_set fds_master;		/* Lista de todos mis sockets. */
fd_set read_fds;		/* Sublista de fds_master. */

int SocketAnterior = 0;

t_configCPU configuracionInicial; /* Estructura del CPU, contiene los sockets de conexion y parametros. */

stPCB* unPCB; /* Estructura del pcb para ejecutar las instrucciones */

t_posicion POSICION_DUMMY;


/*
 ============================================================================
 Name        : Funciones Primitivas para ANSISOP Program.
 Author      : Ezequiel Martinez
 Inputs      : N/A
 Outputs     : N/A
 Description : Se declaran todas las funciones primitivas.
 ============================================================================
 */

t_posicion definirVariable(t_nombre_variable identificador_variable){

	stMensajeIPC mensajePrimitiva;
	t_posicion posicionVariable;

	enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(NUEVAVARIABLE),identificador_variable);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajePrimitiva)){
		printf("Error: Fallo la definicion de variable %s.\n", identificador_variable);
		return posicionVariable;
	}

	if (mensajePrimitiva.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

	}

	//free(mensajePrimitiva);
	return posicionVariable;
}

t_posicion obtenerPosicionVariable(t_nombre_variable identificador_variable ){

	stMensajeIPC mensajePrimitiva;
	t_posicion posicionVariable;

	enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(POSICIONVARIABLE),identificador_variable);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajePrimitiva)){
		printf("Error: Falló la obtencion de posicion de la variable %s.\n", identificador_variable);
		return posicionVariable;
	}

	if (mensajePrimitiva.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

	}

	//free(mensajePrimitiva);
	return posicionVariable;

}


t_valor_variable dereferenciar(t_posicion direccion_variable){

	stMensajeIPC mensajePrimitiva;
	t_valor_variable valor;

	/*TODO Serializar el mensaje de estructura */
	char* estructuraSerializada;

	enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(VALORVARIABLE),estructuraSerializada);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajePrimitiva)){
		printf("Error: Fallo en deferenciar variable.\n");
		return NULL;
	}

	if (mensajePrimitiva.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

	}

	//free(mensajePrimitiva);
	return valor;


}

void asignar(t_posicion direccion_variable, t_valor_variable valor ){

	stMensajeIPC mensajePrimitiva;

	/*TODO Serializar el mensaje de estructura */
	char* estructuraSerializada;

	enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(ASIGNARVARIABLE),estructuraSerializada);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajePrimitiva)){
		printf("Error: Fallo en asignacion de la variable.\n");

	}

	if (mensajePrimitiva.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

	}

	//free(mensajePrimitiva); /*TODO Arreglar el free de las estructuras*/

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);

t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta) ;

t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_posicion donde_retornar, t_puntero_instruccion linea_en_ejecuccion);

t_puntero_instruccion retornar(t_valor_variable retorno);

int imprimir(t_valor_variable valor_mostrar);

int imprimirTexto(char* texto);

int entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);

int wait(t_nombre_semaforo identificador_semaforo);

int signal_cpu(t_nombre_semaforo identificador_semaforo);


AnSISOP_funciones AnSISOP_functions = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dereferenciar,
		.AnSISOP_asignar				= asignar,
		.AnSISOP_imprimir				= imprimir,
		.AnSISOP_imprimirTexto			= imprimirTexto,
		.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		.AnSISOP_irAlLabel				= irAlLabel,
		.AnSISOP_llamarConRetorno		= llamarFuncion,
		.AnSISOP_retornar				= retornar,
		.AnSISOP_entradaSalida			= entradaSalida,
};

AnSISOP_kernel kernel_functions = {
		.AnSISOP_signal		= wait,
		.AnSISOP_signal		= signal_cpu
};



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
int cpuHandShake (int socket, int tipoHeader)
{
	stHeaderIPC *stHeaderIPC;

	if(!recibirHeaderIPC(socket,stHeaderIPC)){
		printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
		fflush(stdout);
	}

	printf("HandShake mensaje recibido %d",stHeaderIPC->tipo);

	if (stHeaderIPC->tipo == QUIENSOS)
	{
		stHeaderIPC = nuevoHeaderIPC(tipoHeader);
		if(!enviarHeaderIPC(socket,stHeaderIPC)){
			printf("No se pudo enviar el MensajeIPC\n");
			liberarHeaderIPC(stHeaderIPC);
			return (-1);
		}
	}

	if(!recibirHeaderIPC(socket,stHeaderIPC)){
			printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
			fflush(stdout);
			liberarHeaderIPC(stHeaderIPC);
			return (-1);
	}

	printf("HandShake: mensaje recibido %d",stHeaderIPC->tipo);
	fflush(stdout);

	if(stHeaderIPC->tipo == OK)
	{
		printf("Conexión establecida con id: %d...\n",tipoHeader);
		fflush(stdout);
		liberarHeaderIPC(stHeaderIPC);
		return socket;
	}

	liberarHeaderIPC(stHeaderIPC);
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

	printf("Conectando con %d...\n",puerto);
	fflush(stdout);
	socket = conectar(IP, puerto);

	// Inicio el handShake con el servidor //
	if (socket != -1)
	{
		if (cpuHandShake(socket, CONNECTCPU) != -1)
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
 Name        : cargarPCB()
 Author      : Ezequiel Martinez
 Inputs      : N/A.
 Outputs     : Retorna -1 en caso de no poder cargar el PCB.
 Description : Funcion para cargar el PCB del progracma ANSISOP.
 =========================================================================================
 */
int cargarPCB(void){

	t_paquete paquete;
	int type;

	recibir_paquete (configuracionInicial.sockNucleo, &paquete);

	type = obtener_paquete_type(&paquete);

	//if (cargarPCB(unMensaje.contenido) != -1)
	if (type == EXECANSISOP)
	{
		deserializar_pcb(&unPCB , &paquete);
		//log_info("PCB de ANSIPROG cargado. /n");
		free_paquete(&paquete);
		return 0;
	}else
		return (-1);

}

/*
 =========================================================================================
 Name        : getInstruccion()
 Author      : Ezequiel Martinez
 Inputs      : Recibe el comienzo de la instruccion y el size.
 Outputs     : Retorna string de la instruccion.
 Description : Funcion para obtener instruccion del progracma ANSISOP en memoria.
 =========================================================================================
 */
int getInstruccion (int start, int size, char** instruccion){

	stMensajeIPC mensajeUMC;

	char* estructuraSerializada;
	t_posicion posicionInstruccion;

	posicionInstruccion.size = size;
	posicionInstruccion.offSet = start;
	posicionInstruccion.nroPagina = unPCB->paginaInicial;

	/*TODO Serializar el mensaje de estructura */


	enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(GETINSTRUCCION),estructuraSerializada);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajeUMC)){
		printf("Error: Fallo en obtener instrucción.\n");
		free(estructuraSerializada);
		return (-1);

	}

	if (mensajeUMC.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

		instruccion = estructuraSerializada;

	}

	free(estructuraSerializada);
	return 0;
}

/*
 =========================================================================================
 Name        : ejecutarInstruccion()
 Author      : Ezequiel Martinez
 Inputs      : N/A
 Outputs     : Retorna -1 en caso de haber algun error.
 Description : Ejecuta una instrucción del PCB.
 =========================================================================================
 */
int ejecutarInstruccion(void){

	int programCounter = unPCB->pc;
	char* instruccion = NULL;

	instruccion = getInstruccion(unPCB->metadata_program->instrucciones_serializado[programCounter].start,
								 unPCB->metadata_program->instrucciones_serializado[programCounter].offset,
								 &instruccion);

	if (instruccion != NULL){
		analizadorLinea(strdup(instruccion), &AnSISOP_functions, &kernel_functions);
	}else{
		printf("Error: fallo la ejecución de instrucción.\n");
		return (-1);
	}

	free(instruccion);
	return 0;

}

/*
 =========================================================================================
 Name        : devolverPCBalNucleo()
 Author      : Ezequiel Martinez
 Inputs      : N/A
 Outputs     : Retorna -1 en caso de haber algun error.
 Description : Envia el PCB al Nucleo con la informacion actualizada.
 =========================================================================================
 */
int devolverPCBalNucleo(void){

	stHeaderIPC *unHeaderIPC;
	t_paquete paquete;
	int resultado=  0;

	if (unPCB->metadata_program->instrucciones_size < unPCB->pc) //Si la cantidad total de instrucciones menor al pc significa que termino el programa.
		unHeaderIPC = nuevoHeaderIPC(FINANSISOP);
	else
		unHeaderIPC = nuevoHeaderIPC(QUANTUMFIN);

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

	crear_paquete(&paquete, EXECANSISOP);
	serializar_pcb(&paquete, unPCB);

	if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
		log_error("No se pudo enviar el PCB al Nucleo[%d]", unPCB->pid);
		resultado = -1;
	}

	free_paquete(&paquete);

	liberarHeaderIPC(unHeaderIPC);

	return resultado;
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

	stMensajeIPC unMensaje;
	stHeaderIPC *unHeaderIPC;
	int unSocket;
	int quantum=0;
	int quantumSleep=0;
	char* temp_file = "cpu.log";

	 //Primero instancio el log
	 t_log* logger = log_create(temp_file, "CPU",-1, LOG_LEVEL_INFO);

	log_info("Iniciando el proceo CPU..."); /* prints CPU Application */


	// Limpio las liastas //
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/***** Cargo la configuracion desde el archivo cpu.conf ********/
	log_info("Cargando configuracion de CPU.");

	cargarConf(&configuracionInicial, CFGFILE);

	log_info("Configuración OK.");

	/***** Lanzo conexión con el Nucleo ********/

	log_info("Conectando al Nucleo...");

	configuracionInicial.sockNucleo = cpuConectarse(configuracionInicial.ipNucleo, configuracionInicial.puertoNucleo, "Nucleo");

	if (configuracionInicial.sockNucleo != -1){
		FD_SET(configuracionInicial.sockNucleo,&(fds_master));
		configuracionInicial.socketMax = configuracionInicial.sockNucleo;
		SocketAnterior = configuracionInicial.socketMax;
		//log_info("OK - Nucleo conectado.");
		fflush(stdout);
		//loguear(OK_LOG,"Nucleo conectado","Nucleo"); TODO Agregar funcion de logueo.

	}	//Fin de conexion al Nucleo//


	/***** Lanzo conexión con el UMC ********/

	//log_info("Conectando al UMC...");

	configuracionInicial.sockUmc = cpuConectarse(configuracionInicial.ipUmc, configuracionInicial.puertoUmc, "UMC");

	if (configuracionInicial.sockUmc > 0){

		FD_SET(configuracionInicial.sockUmc,&(fds_master));
		configuracionInicial.socketMax = configuracionInicial.sockUmc;
		SocketAnterior = configuracionInicial.socketMax;
		//log_info("OK - UMC conectada.");
		fflush(stdout);

	}
		//Fin de conexion al UMC//

	while(configuracionInicial.salir == 0)
	{
		read_fds = fds_master;
		if(seleccionar(configuracionInicial.socketMax,&read_fds,1) == -1)
		{
			log_info("Error Preparando el Select con CPU.\n");
			configuracionInicial.salir = 1;
		}

		for(unSocket=0;unSocket<=configuracionInicial.socketMax;unSocket++)
		{
			if(FD_ISSET(unSocket,&read_fds))
			{
				if (!recibirHeaderIPC(unSocket,&unHeaderIPC))/* Si se cerro un Cliente. */
				{
					if (configuracionInicial.sockNucleo == unSocket)
					{
						log_info("Se desconecto el Servidor Nucleo.\n");
						fflush(stdout);
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
						configuracionInicial.sockUmc = -1;
						configuracionInicial.salir = 1;

						FD_CLR(unSocket,&fds_master);
						close (unSocket);

						if (unSocket > configuracionInicial.socketMax){
							SocketAnterior = configuracionInicial.socketMax;
							configuracionInicial.socketMax = unSocket;
						}

						log_info("Se desconecto el UMC.\n");
						fflush(stdout);
					}
				}
				else
				{
					switch(unHeaderIPC->tipo)
					{
						case EXECANSISOP:

//							log_info("Respondiendo solicitud ANSIPROG...");

							unHeaderIPC = nuevoHeaderIPC(OK);

							enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

							if (cargarPCB() != -1)
							{
								/* Obtengo el quantum del programa */
								quantum = unPCB->quantum;

								if (quantum <= 0){
									printf("Error en Quantum definido. /n");
									break;
								}

								quantumSleep = unPCB->quantumSleep;

								//Ejecuto las instrucciones defidas por quamtum

								while (quantum > 0 && unPCB->pc <= unPCB->metadata_program->instrucciones_size){
									if(ejecutarInstruccion() == OK)
									{
										sleep(quantumSleep);
										quantum --; 	/* descuento un quantum para proxima ejecución */
										unPCB->pc ++; 	/* actualizo el program counter a la siguiente posición */

									}

								}

								if (devolverPCBalNucleo() == -1){

									log_info("Error al devolver PCB de ANSIPROG...");
									configuracionInicial.salir = 1;
									break;

								}

							}else
								log_info("Error en lectura ANSIPROG...");

							printf("AnSISOP fin de Ejecucion por Quantum");
							break;


						case SIGUSR1:

							log_info("Respondiendo solicitud SIGUSR1...");

							unHeaderIPC = nuevoHeaderIPC(SIGUSR1CPU);
							/* Notifico al nucleo mi desconexion*/
							enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

							configuracionInicial.salir = 1;
							break;

					}
				}

			}

		}
	}

	liberarHeaderIPC(unHeaderIPC);
	cerrarSockets(&configuracionInicial);
	log_info("CPU: Fin del programa");
	log_destroy(logger);
	return EXIT_SUCCESS;
}




