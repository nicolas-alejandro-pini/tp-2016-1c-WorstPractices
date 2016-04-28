/*
 ============================================================================
 Name        : UMC.c
 Author      : Diego Laib
 Version     : 0.3
 Copyright   : Mine
 Description : Programa pricipal de UMC - Servidor hacia CPU & Nucleo; Cliente con Swap
 ============================================================================
 */

#include "UMC.h"

/*
 ====o========================================================================
 Funciones
 ============================================================================
 */
int verificarNombreArchivo(char* file_name){
	return 1;
}

void loadInfo (stParametro* info, char* file_name){

	/* TODO realizar verificacines sobre lo cargadop desde el archivo */
	t_config* miConf = config_create (file_name); /*Estructura de configuracion*/

	if (config_has_property(miConf, "PUERTO")) {
		info->miPuerto = config_get_int_value(miConf, "PUERTO");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO");
		exit(-2);
	}

	if (config_has_property(miConf,"IP_SWAP")) {
		info->ipSwap = config_get_string_value(miConf,"IP_SWAP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","IP_SWAP");
		exit(-2);
	}

	if (config_has_property(miConf,"PUERTO_SWAP")) {
		info->puertoSwap = config_get_int_value(miConf,"PUERTO_SWAP");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_SWAP");
		exit(-2);
	}

	if (config_has_property(miConf,"MARCOS")) {
		info->frames = config_get_int_value(miConf,"MARCOS");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","MARCOS");
		exit(-2);
	}

	if (config_has_property(miConf,"MARCOS_SIZE")) {
		info->frameSize = config_get_int_value(miConf,"MARCOS_SIZE");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","MARCOS_SIZE");
		exit(-2);
	}

	if (config_has_property(miConf,"MARCOS_X_PROC")) {
		info->frameByProc = config_get_array_value(miConf,"MARCOS_X_PROC");
	} else {
		printf("Parametro MARCOS_X_PROC no cargado en el archivo de configuracion\n \"%s\"  \n","MARCOS_X_PROC");
		exit(-2);
	}

	if (config_has_property(miConf,"ENTRADAS_TLB")) {
		info->entradasTLB = config_get_array_value(miConf,"ENTRADAS_TLB");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","ENTRADAS_TLB");
		exit(-2);
	}

	if (config_has_property(miConf,"RETARDO")) {
		info->delay = config_get_array_value(miConf,"RETARDO");
	} else {
		printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","RETARDO");
		exit(-2);
	}

}

void cerrarSockets(stParametro *elEstadoActual){

	int unSocket;
	for(unSocket=3; unSocket <= elEstadoActual->fdMax; unSocket++)
		if(FD_ISSET(unSocket,&(fds_master)))
			close(unSocket);

	close(elEstadoActual->sockSwap);

	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));
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

void finalizarSistema(stMensajeIPC *unMensaje,int unSocket, stParametro *unEstado){

	unEstado->salir = 1;
	unMensaje->header.tipo = -1;
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

int main(int argc, char *argv[]) {

	stParametro elEstadoActual;
	stMensajeIPC unMensaje;
	int unCliente = 0, unSocket;
	struct sockaddr addressAceptado;
	int maximoAnterior;
	char enviolog[TAMDATOS];
	char elsocket[10];
	int agregarSock;
	int sockSwap;

	memset(&enviolog,'\0',TAMDATOS);
	/*elEstadoActual = (stParametro*)calloc(1, sizeof(stParametro)); */

	printf("-----------------------------------------------------------------------------\n");
	printf("------------------------------------UMC--------------------------------------\n");
	printf("------------------------------------v1.0-------------------------------------\n\n");

	/* ----------------------------------------Se carga el archivo de configuracion--------------------------- */

		/*loguear(INFO_LOG,"*****INICIO UMC*****\n","UMC"); */
		printf("Obteniendo configuracion...");
		if(verificarNombreArchivo(argv[1])==0){ /* TODO implementar funcion verificarNombreArchivo() */
			printf("\nSELECT ERROR - Error accediendo al archivo de configuracion\n");
			/*loguear(ERROR_LOG,"SELECT ERROR - accediendo al archivo de configuracion","UMC");*/
			return 1;
		}
		loadInfo(&elEstadoActual,argv[1]);
		printf("OK\n\n");
		/*loguear(INFO_LOG,"Configuración OK","SERVER");*/



	/* ----------------------------------------Se realiza la Inicializacion----------------------------------- */

		FD_ZERO(&(fds_master));
		FD_ZERO(&(read_fds));

		elEstadoActual.salir = 0;
		elEstadoActual.sockEscuchador = -1;

	/* ---------------------------------Me pongo a escuchar mi puerto escuchador------------------------------- */

		printf("Estableciendo conexion con socket escuchador...");
		elEstadoActual.sockEscuchador = escuchar(elEstadoActual.miPuerto);
		FD_SET(elEstadoActual.sockEscuchador,&(fds_master));
		elEstadoActual.fdMax =	elEstadoActual.sockEscuchador;
		printf("OK\n");
		/*loguear(INFO_LOG,"Esperando conexiones...","SERVER");*/

		/********************************* Lanzo conexión con el Swap ********************************************/


		elEstadoActual.sockSwap = conectar(elEstadoActual.ipSwap, elEstadoActual.puertoSwap);
		/* Inicio el handShake con el servidor */
		if (elEstadoActual.sockSwap != -1){
		/*	if (cpuHandShake(elEstadoActual.sockSwap, "SOYUMC", CONNECTSWAP) != -1)
			{
				printf("CONNECTION_ERROR - No se recibe un mensaje correcto en Handshake con Swap\n");
				fflush(stdout);
		*/		/*loguear(ERROR_LOG,"SOCKET_ERROR - No se recibe un mensaje correcto","SERVER");*/
		/*		close(elEstadoActual.sockSwap);
			}
			else
			{*/
				printf("OK - Swap conectado. \n");
				fflush(stdout);
				/*loguear(OK_LOG,"Swap conectado","Swap"); TODO Agregar funcion de logueo.*/
	/*		} */
		}	/*Fin de conexion al Swap*/


	/* ........................................Ciclo Principal SERVER........................................ */

		printf(".............................................................................\n"); fflush(stdout);
		printf("..............................Esperando Conexion.............................\n\n"); fflush(stdout);
		fflush(stdout);


		while(elEstadoActual.salir == 0)
		{
			read_fds = fds_master;

			if(seleccionar(elEstadoActual.fdMax,&read_fds,1) == -1)
				{
					printf("\nSELECT ERROR - Error Preparando el Select\n");
					/*loguear(INFO_LOG,"SELECT ERROR - Error Preparando el Select","SERVER");*/
					return 1;
				}


			for(unSocket=0;unSocket<=elEstadoActual.fdMax;unSocket++){

				if(FD_ISSET(unSocket,&read_fds)){

	/*----------------------------------------------------Conexion Nueva--------------------------------------*/
		        	if(unSocket == elEstadoActual.sockEscuchador){

		        		unCliente = aceptar(elEstadoActual.sockEscuchador,&addressAceptado);

		        		printf("Nuevo pedido de conexion...\n");

		        		if(!enviarMensajeIPC(unCliente,nuevoHeaderIPC(QUIENSOS),"MSGQUIENSOS")){
		        			printf("No se pudo enviar el MensajeIPC\n");
						}

		        		if(!recibirMensajeIPC(unCliente,&unMensaje)){
		        			printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
		        			fflush(stdout);
		        			/*loguear(ERROR_LOG,"SOCKET_ERROR - No se recibe un mensaje correcto","SERVER");*/
		        			close(unCliente);
		        		}

	/*------------------------------------Identifico quien se conecto y procedo--------------------------------*/

						if(unMensaje.header.tipo==SOYCPUHSK){
								if(!enviarMensajeIPC(unCliente,nuevoHeaderIPC(OKCPUHSK),"MSGOK")){
									printf("No se pudo enviar el MensajeIPC al cliente\n");
									return 0;
								}
								printf("Conexion con modulo cliente establecida\n");
								/*loguear(INFO_LOG,"Conexion con modulo cliente establecida\n","SERVER");*/
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
						}
						if(unMensaje.header.tipo==SOYNCLHSK){
							if(!enviarMensajeIPC(unCliente,nuevoHeaderIPC(OKNCLHSK),"MSGOK")){
								printf("No se pudo enviar el MensajeIPC al cliente\n");
								return 0;
							}
							printf("Conexion con modulo cliente establecida\n");
							/*loguear(INFO_LOG,"Conexion con modulo cliente establecida\n","SERVER");*/
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
						}
					}/*-Cierro if-Conexion Nueva-*/

	/*--------------------------------------Conexion de un cliente existente-------------------------------------*/
					else {
						memset(unMensaje.contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);

						if (!recibirMensajeIPC(unSocket,&unMensaje)){ /* Si se cerro una conexion, veo que el socket siga abierto*/

							if(unSocket==elEstadoActual.sockEscuchador){
								printf("Se perdio conexion con el cliente conectado...\n ");
								sprintf(enviolog,"SOCKET ERROR - Conexion perdida: %d",unSocket);
								/*loguear(INFO_LOG,"SOCKET ERROR - Conexion perdida:\n","CLIENTE");*/
							}

							/*Saco el socket de la lista Master*/
							FD_CLR(unSocket,&fds_master);
							close (unSocket);

							if (unSocket > elEstadoActual.fdMax){
								maximoAnterior = elEstadoActual.fdMax;
								elEstadoActual.fdMax = unSocket;
							}
							memset(enviolog,'\0',TAMDATOS);
							fflush(stdout);
						}
	        	        else{

	        	        	/*Se sigue comunicado con el cliente, podría recibir otros mensajes */
	        	        	/* TODO Realizar instrucciones de acuerdo a lo que nos diga el nucleo*/

	        	        	/* TODO Realizar instrucciones de acuerdo a lo que nos diga el CPU*/

	        	        	switch(unMensaje.header.tipo)
	        	        	{

	        	        		default:
	        	        			printf("\nRespondiendo solicitud Pedido CPU...\n");
	        	        			enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");
	        	        			enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");
	        	        			break;

	        	        	}
							/*Cierro switch(unMensaje.header.tipo)*/

			  			fflush(stdout);

	        	        }/*Ciero Else comunicacion con el servidor*/

					}/*Cierra Else de WebServer Conocido*/

				}/*Cierro if(FD_ISSET(unSocket,&read_fds))*/

		    }/*Cierro for(unSocket=0;unSocket<=elEstadoActual.fdMax;unSocket++)*/

		}/*Cierro while(elEstadoActual.salir == 0)*/

	/* ............................................Finalizacion............................................. */
		cerrarSockets(&elEstadoActual);
		printf("\nSERVER: Fin del programa\n");
		/*loguear(INFO_LOG,"Fin del programa","SERVER");*/
		return EXIT_SUCCESS;

}

