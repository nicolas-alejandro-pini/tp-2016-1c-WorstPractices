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

/*
 ============================================================================
 Estructuras y definiciones
 ============================================================================
 */

typedef struct{
	char* miIP;             /* Mi direccion de IP. Ej: <"127.0.0.1"> */
	int puertoProg;			/* Puerto de escucha Consola */
	int sockProg;			/* Socket de escucha Consola */
	int puertoCpu;			/* Puerto de escucha CPU */
	int sockCpu;			/* Socket de escucha CPU */
	int fdMax;              /* Numero que representa al mayor socket de fds_master. */
	int fdMax2;             /* Numero que representa al mayor socket de fds_master. */
	int salir;              /* Indica si debo o no salir de la aplicacion. */
} stEstado;

fd_set fds_master;			/* Lista de todos mis sockets.*/
fd_set read_fds;	  		/* Sublista de fds_master.*/


/*
 ============================================================================
 Funciones
 ============================================================================
 */
void loadInfo (stEstado* info,char* file_name){

	t_config* miConf = config_create ("nucleo.conf"); /*Estructura de configuracion*/

	if (config_has_property(miConf,"NUCLEO_IP")) {
		info->miIP = config_get_string_value(miConf,"NUCLEO_IP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","NUCLEO_IP");
		exit(-2);
	}

	if (config_has_property(miConf,"PUERTO_PROG")) {
		info->puertoProg = config_get_int_value(miConf,"PUERTO_PROG");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_PROG");
		exit(-2);
	}

	if (config_has_property(miConf,"PUERTO_CPU")) {
		info->puertoCpu= config_get_int_value(miConf,"PUERTO_CPU");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_CPU");
		exit(-2);
	}
}

void cerrarSockets(stEstado *elEstadoActual){

	int unSocket;
	for(unSocket=3; unSocket <= elEstadoActual->fdMax; unSocket++)
		if(FD_ISSET(unSocket,&(fds_master)))
			close(unSocket);

	for(unSocket=3; unSocket <= elEstadoActual->fdMax2; unSocket++)
		if(FD_ISSET(unSocket,&(fds_master)))
			close(unSocket);

	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));
}

void finalizarSistema(stMensajeIPC *unMensaje,int unSocket, stEstado *unEstado){

	unEstado->salir = 1;
	unMensaje->header.tipo = -1;
}

/*
 ============================================================================
 Funcion principal
 ============================================================================
 */
int main(int argc, char *argv[]) {
	stEstado elEstadoActual;
	stMensajeIPC unMensaje;

	int unClienteConsola = 0, unSocket;
	int unClienteCpu = 0;
	struct sockaddr addressAceptado;
	int maximoAnterior;
	int agregarSockConsola, agregarSockCpu;

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

	/*Inicializacion de sockets de escucha*/
	elEstadoActual.salir = 0;
	elEstadoActual.sockCpu= -1;
	elEstadoActual.sockProg = -1;

	/*Iniciando escucha en el socket escuchador de Consola*/
	printf("Estableciendo conexion con socket escuchador de Consola...");
	elEstadoActual.sockProg = escuchar(elEstadoActual.puertoProg);
	FD_SET(elEstadoActual.sockProg,&(fds_master));
	elEstadoActual.fdMax =	elEstadoActual.sockProg;
	printf("OK\n");

	/*Iniciando escucha en el socket escuchador de Consola*/
	printf("Estableciendo conexion con socket escuchador de CPU...");
	elEstadoActual.sockCpu = escuchar(elEstadoActual.puertoCpu);
	FD_SET(elEstadoActual.sockCpu,&(fds_master));
	elEstadoActual.fdMax2 =	elEstadoActual.sockCpu;
	printf("OK\n");

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

		if(seleccionar(elEstadoActual.fdMax2,&read_fds,1) == -1){
			printf("SELECT ERROR - Error Preparando el Select\n");
			return 1;
		}

		for(unSocket=0;unSocket<=elEstadoActual.fdMax2;unSocket++){
			unClienteCpu = aceptar(elEstadoActual.sockCpu,&addressAceptado);
			printf("Nuevo pedido de conexion...\n");

			if(!enviarMensajeIPC(unClienteConsola,nuevoHeaderIPC(QUIENSOS),"MSGQUIENSOS")){
				printf("No se pudo enviar el MensajeIPC\n");
			}

			if(!recibirMensajeIPC(unClienteConsola,&unMensaje)){
				printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
				fflush(stdout);
				close(unClienteConsola);
			 }

			/*Identifico quien se conecto y procedo*/
			if(unMensaje.header.tipo=="CONNECTCPU"){
				if(!enviarMensajeIPC(unClienteCpu,nuevoHeaderIPC(OK),"MSGOK")){
					printf("No se pudo enviar el MensajeIPC al cliente\n");
					return 0;
				}

				printf("Conexion con modulo cliente establecida\n");
				agregarSockCpu=1;

				/*Agrego el socket conectado A la lista Master*/
				if(agregarSockCpu==1){
					FD_SET(unClienteCpu,&(fds_master));
					if (unClienteCpu > elEstadoActual.fdMax){
						maximoAnterior = elEstadoActual.fdMax;
						elEstadoActual.fdMax = unClienteCpu;
					}
					agregarSockCpu=0;
				}
			}else{
				memset(unMensaje.contenido,'\0',"LONGITUD_MAX_DE_CONTENIDO");

				if (!recibirMensajeIPC(unSocket,&unMensaje)){
					if(unSocket==elEstadoActual.sockProg){
						printf("Se perdio conexion con el cpu conectado...\n ");
					}

					/*Saco el socket de la lista Master*/
					FD_CLR(unSocket,&fds_master);
					close (unSocket);

					if (unSocket > elEstadoActual.fdMax){
						maximoAnterior = elEstadoActual.fdMax;
						elEstadoActual.fdMax = unSocket;
					}

					fflush(stdout);

				}else{
					/*Agregar en una lista de CPU  */
				}

			}
		}

	}
			cerrarSockets(&elEstadoActual);
			finalizarSistema(&unMensaje,unSocket,&elEstadoActual);
			printf("\nNUCLEO: Fin del programa\n");
			return 0;
}
