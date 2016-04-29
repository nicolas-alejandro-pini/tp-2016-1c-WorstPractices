/*
 ============================================================================
 Name        : Nucleo.c
 Author      : Jose Maria Suarez
 Version     : 0.1
 Description : Elestac - Nucleo
 ============================================================================
 */
#include "../lib/librerias.h"
#include "../lib/sockets.c"
#include "../lib/socketsIPCIRC.c"
#include "../lib/fComunes.c"
#include "../lib/listas.c"

/*
 ============================================================================
 Estructuras y definiciones
 ============================================================================
 */

typedef struct{
	char* miIP;             /* Mi direccion de IP. Ej: <"127.0.0.1"> */
	int miPuerto;			/* Mi Puerto de escucha */
	int sockEscuchador;		/* Socket con el que escucho. */
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

/* Listas globales */
fd_set fds_master;			/* Lista de todos mis sockets.*/
fd_set read_fds;	  		/* Sublista de fds_master.*/

lista CPU_Conectados=NULL;   /*Lista de todos los CPU conectados al Nucleo*/

/*Listas de estados de planificacion*/
lista PCB_ready=NULL;   	 /*Lista de todos los CPU conectados al Nucleo*/
lista PCB_exit=NULL;		 /*Lista de todos los CPU listos para salir*/


/*
 ============================================================================
 Funciones
 ============================================================================
 */
void loadInfo (stEstado* info,char* file_name){

	t_config* miConf = config_create (CFGFILE); /*Estructura de configuracion*/

	if (config_has_property(miConf,"IP")) {
		info->miIP = config_get_string_value(miConf,"IP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IP");
		exit(-2);
	}

	if (config_has_property(miConf,"PUERTO")) {
		info->miPuerto = config_get_int_value(miConf,"PUERTO");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO");
		exit(-2);
	}

	if (config_has_property(miConf,"QUANTUM")) {
		info->quantum = config_get_int_value(miConf,"QUANTUM");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","QUANTUM");
		exit(-2);
	}

	if (config_has_property(miConf,"QUANTUM_SLEEP")) {
		info->quantumSleep = config_get_int_value(miConf,"QUANTUM_SLEEP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","QUANTUM_SLEEP");
		exit(-2);
	}

	if (config_has_property(miConf,"SEM_IDS")) {
		info->semIds = config_get_array_value(miConf,"SEM_IDS");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","SEM_IDS");
		exit(-2);
	}

	if (config_has_property(miConf,"SEM_INIT")) {
		info->semInit = config_get_array_value(miConf,"SEM_INIT");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","SEM_INIT");
		exit(-2);
	}

	if (config_has_property(miConf,"IO_IDS")) {
		info->ioIds = config_get_array_value(miConf,"IO_IDS");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IO_IDS");
		exit(-2);
	}

	if (config_has_property(miConf,"IO_SLEEP")) {
		info->semInit = config_get_array_value(miConf,"IO_SLEEP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IO_SLEEP");
		exit(-2);
	}

	if (config_has_property(miConf,"SHARED_VARS")) {
		info->sharedVars = config_get_array_value(miConf,"SHARED_VARS");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","SHARED_VARS");
		exit(-2);
	}
}

void cerrarSockets(stEstado *elEstadoActual){

	int unSocket;
	for(unSocket=3; unSocket <= elEstadoActual->fdMax; unSocket++)
		if(FD_ISSET(unSocket,&(fds_master)))
			close(unSocket);

	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));
}

void finalizarSistema(stMensajeIPC *unMensaje,int unSocket, stEstado *unEstado){

	unEstado->salir = 1;
	unMensaje->header.tipo = -1;
}

int enviarAEjecutar(lista listaCPU, char* unPrograma){

	stCPUConectado* unCPU;
	stMensajeIPC unMensaje;

	if (!isEmpty(listaCPU)) {
			unCPU=(stCPUConectado*)primerDato(listaCPU);
			quitarDeLista(&listaCPU,NULL,quitarDeAdelante);
				/*Para la primera entrega se manda unPrograma que representa un PCB*/
				if(!enviarMensajeIPC(unCPU->socket,nuevoHeaderIPC(EXECANSISOP),unPrograma)){
					printf("No se pudo enviar el MensajeIPC al CPU disponible\n");
					return 0;
				}
				memset(unMensaje.contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);
				if(!recibirMensajeIPC(unCPU->socket,&unMensaje)){
					printf("No se recibio el mensaje del CPU disponible\n");
					return 0;
				}
				if(!unMensaje.header.tipo==OK){
					printf("El CPU no responde\n");
				}else{
					agregarALista(&listaCPU,unCPU,agregarAtras);
					return 1;
		}

	}else{
		printf("No hay CPU disponible\n");
		return 0;
	}
}




/*
 ============================================================================
 Funcion principal
 ============================================================================
 */
int main(int argc, char *argv[]) {

	stEstado elEstadoActual;
	stMensajeIPC unMensaje;
	stCPUConectado* unNodoCPU;

	int unCliente = 0, unSocket;
	struct sockaddr addressAceptado;
	int maximoAnterior;
	int agregarSock;

	printf("----------------------------------Elestac------------------------------------\n");
	printf("-----------------------------------Nucleo------------------------------------\n");
	printf("------------------------------------v0.1-------------------------------------\n\n");
	fflush(stdout);

	/*Carga del archivo de configuracion*/
	printf("Obteniendo configuracion...");
	loadInfo(&elEstadoActual,CFGFILE);
	printf("OK\n");

	/*Inicializacion de listas de socket*/
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/*Inicializacion de socket de escucha*/
	elEstadoActual.salir = 0;
	elEstadoActual.sockEscuchador= -1;

	/*Iniciando escucha en el socket escuchador de Consola*/
	printf("Estableciendo conexion con socket escuchador...");
	elEstadoActual.sockEscuchador = escuchar(elEstadoActual.miPuerto);
	FD_SET(elEstadoActual.sockEscuchador,&(fds_master));
	printf("OK\n");

	/*Seteamos el maximo socket*/
	elEstadoActual.fdMax = elEstadoActual.sockEscuchador;


	/*Ciclo Principal del Nucleo*/
	printf(".............................................................................\n");
	fflush(stdout);
	printf("..............................Esperando Conexion.............................\n\n");
	fflush(stdout);

	while(elEstadoActual.salir == 0)
	{
		read_fds = fds_master;

		if(seleccionar(elEstadoActual.fdMax,&read_fds,1) == -1){
			printf("SELECT ERROR - Error Preparando el Select\n");
			return 1;
		}

		for(unSocket=0;unSocket<=elEstadoActual.fdMax;unSocket++){

			/*Nueva conexion*/
			if(unSocket == elEstadoActual.sockEscuchador){
				unCliente = aceptar(elEstadoActual.sockEscuchador,&addressAceptado);
				printf("Nuevo pedido de conexion...\n");

				if(!enviarMensajeIPC(unCliente,nuevoHeaderIPC(QUIENSOS),"MSGQUIENSOS")){
					printf("No se pudo enviar el MensajeIPC\n");
				}

				if(!recibirMensajeIPC(unCliente,&unMensaje)){
					printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
					fflush(stdout);
					close(unCliente);
				 }

				/*Identifico quien se conecto y procedo*/
				switch (unMensaje.header.tipo) {
					case CONNECTCONSOLA:

						if(!enviarMensajeIPC(unCliente,nuevoHeaderIPC(OK),"MSGOK")){
							printf("No se pudo enviar el MensajeIPC al cliente\n");
							return 0;
						}

						printf("Nueva consola conectada\n");
						agregarSock=1;

						/*Agrego el socket conectado A la lista Master*/
						if(agregarSock==1){
							FD_SET(unCliente,&(fds_master));
							if (unCliente > elEstadoActual.fdMax){
								maximoAnterior = elEstadoActual.fdMax;
								elEstadoActual.fdMax = unCliente;
							}
							agregarSock=0;
						}

						 if(!recibirMensajeIPC(unCliente,&unMensaje)){
							 printf("Error:No se recibio el mensaje de la consola\n");
							 break;
						 }else{
							 if (unMensaje.header.tipo == SENDANSISOP) {
								if (!enviarAEjecutar(CPU_Conectados,unMensaje.contenido)){
									printf("No se pudo enviar el programa\n");
								}
							}

						 }


						break;

					case CONNECTCPU:

						if(!enviarMensajeIPC(unCliente,nuevoHeaderIPC(OK),"MSGOK")){
							printf("No se pudo enviar el MensajeIPC al CPU\n");
							return 0;
						}

						printf("Nuevo CPU conectado\n");
						agregarSock=1;

						/*Agrego el socket conectado A la lista Master*/
						if(agregarSock==1){
							FD_SET(unCliente,&(fds_master));
							if (unCliente > elEstadoActual.fdMax){
								maximoAnterior = elEstadoActual.fdMax;
								elEstadoActual.fdMax = unCliente;
							}
							agregarSock=0;
						}

						unNodoCPU = (stCPUConectado*)malloc(sizeof(stCPUConectado));
						unNodoCPU -> socket = unCliente;
						agregarALista(&CPU_Conectados,(stCPUConectado*)unNodoCPU,agregarAdelante);

						break;
					default:
						break;
				}
			}else
			{
				/*Conexion existente*/
				memset(unMensaje.contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);
				if (!recibirMensajeIPC(unSocket,&unMensaje)){
					if(unSocket==elEstadoActual.sockEscuchador){
						printf("Se perdio conexion...\n ");
					}
					/*TODO: Sacar de la lista de cpu conectados si hay alguna desconexion.*/
					/*Saco el socket de la lista Master*/
					FD_CLR(unSocket,&fds_master);
					close (unSocket);

					if (unSocket > elEstadoActual.fdMax){
						maximoAnterior = elEstadoActual.fdMax;
						elEstadoActual.fdMax = unSocket;
					}

					fflush(stdout);
				}else{
					/*Recibo con mensaje de conexion existente*/


				}


			}

		}

	}
			cerrarSockets(&elEstadoActual);
			finalizarSistema(&unMensaje,unSocket,&elEstadoActual);
			printf("\nNUCLEO: Fin del programa\n");
			return 0;
}
