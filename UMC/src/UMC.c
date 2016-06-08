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

void loadInfo (stParametro* info, char* file_name){

	/* TODO realizar verificacines sobre lo cargadop desde el archivo */
	t_config* miConf = config_create (file_name);

	if(miConf == NULL){
		log_error("CONFIG ERROR - Error accediendo al archivo de configuracion");
		exit(-1);
	}

	if (config_has_property(miConf, "PUERTO")) {
		info->miPuerto = config_get_int_value(miConf, "PUERTO");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - PUERTO");
		exit(-2);
	}

	if (config_has_property(miConf,"IP_SWAP")) {
		info->ipSwap = config_get_string_value(miConf,"IP_SWAP");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - IP_SWAP");
		exit(-2);
	}

	if (config_has_property(miConf,"PUERTO_SWAP")) {
		info->puertoSwap = config_get_int_value(miConf,"PUERTO_SWAP");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - PUERTO_SWAP");
		exit(-2);
	}

	if (config_has_property(miConf,"MARCOS")) {
		info->frames = config_get_int_value(miConf,"MARCOS");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - MARCOS");
		exit(-2);
	}
	frames = info->frames;

	if (config_has_property(miConf,"MARCOS_SIZE")) {
		info->frameSize = config_get_int_value(miConf,"MARCOS_SIZE");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - MARCOS_SIZE");
		exit(-2);
	}
	frameSize = info->frameSize;

	if (config_has_property(miConf,"MARCOS_X_PROC")) {
		info->frameByProc = config_get_array_value(miConf,"MARCOS_X_PROC");
	} else {
		log_error("Parametro MARCOS_X_PROC no cargado en el archivo de configuracion - MARCOS_X_PROC");
		exit(-2);
	}
	frameByProc = info->frameByProc;

	if (config_has_property(miConf,"ALGORITMO")) {
		info->algoritmo = config_get_string_value(miConf,"ALGORITMO");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - ALGORITMO");
		exit(-2);
	}

	if (config_has_property(miConf,"ENTRADAS_TLB")) {
		info->entradasTLB = config_get_array_value(miConf,"ENTRADAS_TLB");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - ENTRADAS_TLB");
		exit(-2);
	}

	if (config_has_property(miConf,"RETARDO")) {
		info->delay = config_get_array_value(miConf,"RETARDO");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - RETARDO");
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
 Name        : swapHandShake
 Author      : Diego Laib
 Inputs      : Recibe el socket, un mensaje para identificarse y un tipo para el header.
 Outputs     : Retorna -1 en caso de error y si no hay error devuelve el socket.
 Description : Funcion handshake con Swap
 =========================================================================================
 */
int swapHandShake (int socket, char* mensaje, int tipoHeader)
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

	log_info("HandShake: establecido");
	fflush(stdout);

	return 0;
}

void finalizarSistema(stMensajeIPC *unMensaje,int unSocket, stParametro *unEstado){

	unEstado->salir = 1;
	unMensaje->header.tipo = -1;
}

int main(int argc, char *argv[]) {

	stParametro elEstadoActual;
	stMensajeIPC *unMensaje;
	stHeaderIPC *unaCabecera;
	int unCliente = 0, unSocket;
	struct sockaddr addressAceptado;
	int maximoAnterior;
	char enviolog[TAMDATOS];
	char elsocket[10];
	int agregarSock;
	pthread_attr_t attr;
	char* temp_file = "umc.log";

	memset(&enviolog,'\0',TAMDATOS);
	/*elEstadoActual = (stParametro*)calloc(1, sizeof(stParametro)); */

	//Primero instancio el log
		t_log* logger = log_create(temp_file, "UMC",-1, LOG_LEVEL_INFO);

	log_info("-----------------------------------------------------------------------------\n");
	log_info("------------------------------------UMC--------------------------------------\n");
	log_info("------------------------------------v1.0-------------------------------------\n\n");



	/* ----------------------------------------Se carga el archivo de configuracion--------------------------- */



		log_info("*****INICIO UMC*****");
		log_info("Obteniendo configuracion...");
		loadInfo(&elEstadoActual,argv[1]);
		log_info("Configuración OK");



		/* --------------------------------Se realiza la Inicializacion de estructuras---------------------------- */

		memoriaPrincipal = inicializarMemoriaDisponible(elEstadoActual.frameSize, elEstadoActual.frames);


		/* --------------------------Se realiza la Inicializacion para la conexion-------------------------------- */

		FD_ZERO(&(fds_master));
		FD_ZERO(&(read_fds));

		elEstadoActual.salir = 0;
		elEstadoActual.sockEscuchador = -1;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);

		/* ---------------------------------Me pongo a escuchar mi puerto escuchador------------------------------- */

		log_info("Creando el socket de escucha...");
		elEstadoActual.sockEscuchador = escuchar(elEstadoActual.miPuerto);
		if(elEstadoActual.sockEscuchador < 0){
			log_info("No puede crear el socket de escucha...");
			exit(-1);
		}
		FD_SET(elEstadoActual.sockEscuchador,&(fds_master));
		elEstadoActual.fdMax =	elEstadoActual.sockEscuchador;
		log_info("Socket de escucha OK\n");
		/*loguear(INFO_LOG,"Esperando conexiones...","SERVER");*/

		/********************************* Lanzo conexión con el Swap ********************************************/
		log_info("Conectando con el Swap");
		sockSwap = conectar(elEstadoActual.ipSwap, elEstadoActual.puertoSwap);
		/* Inicio el handShake con el servidor */
		if (sockSwap != -1){
			if (swapHandShake(elEstadoActual.sockSwap, "SOYUMC", SOYUMC) != -1)
			{
				log_info("CONNECTION_ERROR - No se recibe un mensaje correcto en Handshake con Swap");
				fflush(stdout);
				log_info("SOCKET_ERROR - No se recibe un mensaje correcto");
				close(elEstadoActual.sockSwap);
			}
			else
			{
				log_info("OK - Swap conectado.");
				fflush(stdout);
				/*loguear(OK_LOG,"Swap conectado","Swap"); TODO Agregar funcion de logueo.*/
			}
		}	/*Fin de conexion al Swap*/
		else{
			log_info("No se pudo conectar con el Swap");
			log_destroy(logger);
		}

	/* ........................................Ciclo Principal SERVER........................................ */

		printf(".............................................................................\n"); fflush(stdout);
		printf("..............................Esperando Conexion.............................\n\n"); fflush(stdout);
		fflush(stdout);


		while(elEstadoActual.salir == 0)
		{
			read_fds = fds_master;

			if(seleccionar(elEstadoActual.fdMax,&read_fds,0) == -1)
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

		        		unaCabecera = nuevoHeaderIPC(QUIENSOS);
		        		if(!enviarHeaderIPC(unCliente,unaCabecera)){
		        			printf("HandShake Error - No se pudo enviar mensaje QUIENSOS\n");
		        			liberarHeaderIPC(unaCabecera);
		        			close(unCliente);
		        			continue;
		        		}

		        		if(!recibirHeaderIPC(unCliente,unaCabecera)){
		        			printf("HandShake Error - No se pudo recibir mensaje de respuesta\n");
		        			liberarHeaderIPC(unaCabecera);
		        			close(unCliente);
		        			continue;
		        		}

	/*------------------------------------Identifico quien se conecto y procedo--------------------------------*/

						switch(unaCabecera->tipo){
							case CONNECTCPU:

								unaCabecera = nuevoHeaderIPC(OK);
								if(!enviarHeaderIPC(unCliente, unaCabecera)){
									printf("No se pudo enviar un mensaje de confirmacion a la consola conectada\n");
									liberarHeaderIPC(unaCabecera);
									close(unCliente);
									continue;
								}
								printf("Conexion con CPU establecida\n");
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
								break;
							case CONNECTNUCLEO:
								unaCabecera = nuevoHeaderIPC(OK);
								if(!enviarHeaderIPC(unCliente, unaCabecera)){
									printf("No se pudo enviar un mensaje de confirmacion a la consola conectada\n");
									liberarHeaderIPC(unaCabecera);
									close(unCliente);
									continue;
								}

								enviarConfigUMC(unCliente, elEstadoActual.frameSize, elEstadoActual.frameByProc);

//								log_info("Conexion con modulo cliente establecida Nucleo");
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
						unMensaje->contenido = calloc(1,LONGITUD_MAX_DE_CONTENIDO);
						memset(unMensaje->contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);

						if (!recibirMensajeIPC(unSocket,unMensaje)){ /* Si se cerro una conexion, veo que el socket siga abierto*/

							if(unSocket==elEstadoActual.sockEscuchador){
								printf("Se perdio conexion con el cliente conectado...\n ");
								sprintf(enviolog,"SOCKET ERROR - Conexion perdida: %d",unSocket);
								log_info(enviolog);
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
	        	        	/* TODO Realizar instrucciones de acuerdo a lo que nos diga la consola*/

	        	        	/* Aplico demora definida en archivo de configuracion */
	        	        	sleep(elEstadoActual.delay);

	        	        	realizarAccionUMC(unMensaje->header.tipo, unMensaje->contenido, unSocket, attr);

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
void* inicializarMemoriaDisponible(long tamanio, long cantidad){
	void* r;
	if((r=calloc(cantidad, tamanio))==NULL){
		printf("No hay memoria disponible...");
		exit(-1);
	}
	return r;
}
