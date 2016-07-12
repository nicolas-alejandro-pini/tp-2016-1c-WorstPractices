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
		// referencia a t_config* miConf !
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
		info->frameByProc = config_get_int_value(miConf,"MARCOS_X_PROC");
	} else {
		log_error("Parametro MARCOS_X_PROC no cargado en el archivo de configuracion - MARCOS_X_PROC");
		exit(-2);
	}
	frameByProc = info->frameByProc;

	if (config_has_property(miConf,"ALGORITMO")) {
		// referencia a t_config* miConf !
		info->algoritmo = config_get_string_value(miConf,"ALGORITMO");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - ALGORITMO");
		exit(-2);
	}

	if (config_has_property(miConf,"ENTRADAS_TLB")) {
		info->entradasTLB = config_get_int_value(miConf,"ENTRADAS_TLB");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - ENTRADAS_TLB");
		exit(-2);
	}

	if (config_has_property(miConf,"RETARDO")) {
		info->delay = config_get_int_value(miConf,"RETARDO");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion - RETARDO");
		exit(-2);
	}

	// No puedo liberar aca porque tengo dos referencias // config_destroy(miConf);
	// agrego la referencia a parametros para poder liberarla despues
	info->tConfig = miConf;
}

void loadInfo_destruir (stParametro* info){
	// Libera ipSwap y algoritmo + todo el tConfig
	free(info->tConfig);
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
	stHeaderIPC unHeader, *otroHeader;

	if(!recibirHeaderIPC(socket,&unHeader)){
		log_error("SOCKET_ERROR - No se recibe un mensaje correcto");
	}

	log_info("HandShake mensaje recibido %d",unHeader.tipo);


	if (unHeader.tipo == QUIENSOS)
	{
		otroHeader = nuevoHeaderIPC(tipoHeader);
		if(!enviarHeaderIPC(socket,otroHeader)){
			log_error("No se pudo enviar el MensajeIPC");
			return (-1);
		}
		if(!recibirHeaderIPC(socket,&unHeader)){
			log_error("SOCKET_ERROR - No se recibe un mensaje correcto");
			return (-1);
		}
		if (unHeader.tipo != OK){
			log_error("El handshake no se pudo completar");
			return (-1);
		}
	}else{
		log_error("El handshake no se pudo completar");
		return (-1);
	}

	log_info("HandShake: establecido");

	return 0;
}

void finalizarSistema(stMensajeIPC *unMensaje,int unSocket, stParametro *unEstado){

	unEstado->salir = 1;
	unMensaje->header.tipo = -1;
}

int main(int argc, char *argv[]) {

	stMensajeIPC unMensaje;
	stHeaderIPC *unaCabecera = NULL;
	stHeaderIPC *unaCabecera2 = NULL;
	int i=0, unCliente = 0, unSocket = 0;
	struct sockaddr addressAceptado;
	int maximoAnterior = 0;
	char enviolog[TAMDATOS];
	//char elsocket[10];
	int agregarSock;
	pthread_attr_t attr;
	pthread_t tid;
	char* temp_file = "umc.log";
	stIni *ini = NULL;
	stPageIni *unPageIni;
	stEnd *end = NULL;
	t_paquete paquete_stPageIni;

	memset(&enviolog,'\0',TAMDATOS);
	/*elEstadoActual = (stParametro*)calloc(1, sizeof(stParametro)); */

	//Primero instancio el log
		logger = log_create(temp_file, "UMC",-1, LOG_LEVEL_INFO);

	// Ejecuto las pruebas
	if(argv[2])
		if(strcmp(argv[2], "--cunit")==0)
			test_unit_umc();

	printf("-----------------------------------------------------------------------------\n");
	printf("------------------------------------UMC--------------------------------------\n");
	printf("------------------------------------v1.0-------------------------------------\n\n");



	/* ----------------------------------------Se carga el archivo de configuracion--------------------------- */



		log_info("*****INICIO UMC*****");
		log_info("Obteniendo configuracion...");
		if(!argv[1])
		{
			log_error("./UMC <umc.conf>\n");
			exit(-1);
		}
		loadInfo(&losParametros,argv[1]);
		log_info("Configuración OK");



		/* --------------------------------Se realiza la Inicializacion de estructuras---------------------------- */
		t_list_mutex *TablaMarcos;
		TablaMarcos = NULL;
		creatListaDeTablas(TablaMarcos); // TablaMarcos global
		crearTLB(losParametros.entradasTLB);
		memoriaPrincipal = inicializarMemoriaPrincipal(losParametros.frameSize, losParametros.frames);

		/* --------------------------Se realiza la Inicializacion para la conexion-------------------------------- */

		FD_ZERO(&(fds_master));
		FD_ZERO(&(read_fds));

		losParametros.salir = 0;
		losParametros.sockEscuchador = -1;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);

		/* ---------------------------------Me pongo a escuchar mi puerto escuchador------------------------------- */

		log_info("Creando el socket de escucha...");
		losParametros.sockEscuchador = escuchar(losParametros.miPuerto);
		if(losParametros.sockEscuchador < 0){
			log_info("No puede crear el socket de escucha...");
			exit(-1);
		}
		FD_SET(losParametros.sockEscuchador,&(fds_master));
		losParametros.fdMax =	losParametros.sockEscuchador;
		log_info("Socket de escucha OK\n");
		/*loguear(INFO_LOG,"Esperando conexiones...","SERVER");*/

		/********************************* Lanzo conexión con el Swap ********************************************/
		log_info("Conectando con el Swap");
		losParametros.sockSwap = conectar(losParametros.ipSwap, losParametros.puertoSwap);
		/* Inicio el handShake con el servidor */
		if (losParametros.sockSwap != -1){
			if (swapHandShake(losParametros.sockSwap, "SOYUMC", SOYUMC) == -1)
			{
				log_info("CONNECTION_ERROR - No se recibe un mensaje correcto en Handshake con Swap");
				fflush(stdout);
				log_info("SOCKET_ERROR - No se recibe un mensaje correcto");
				close(losParametros.sockSwap);
			}
			else
			{
				FD_SET(losParametros.sockSwap, &(fds_master));
				log_info("OK - Swap conectado.");
				fflush(stdout);

			}
		}	/*Fin de conexion al Swap*/
		else{
			log_info("No se pudo conectar con el Swap");
			log_destroy(logger);
			exit(EXIT_FAILURE);
		}

	/* Agrego el stdin a la lista de sockets............................................................................. */
			FD_SET(fileno(stdin),&(fds_master));
			mostrarHelp();
			log_info("fdMax: %d",losParametros.fdMax);

	/* ........................................Ciclo Principal SERVER........................................ */

		printf(".............................................................................\n");
		printf("..............................Esperando Conexion.............................\n");
		fflush(stdout);


		while(losParametros.salir == 0)
		{
			read_fds = fds_master;

			if(seleccionar(losParametros.fdMax,&read_fds,0) == -1)
				{
					log_info("SELECT ERROR - Error Preparando el Select");
					return 1;
				}


			for(unSocket=0;unSocket<=losParametros.fdMax;unSocket++){

				if(FD_ISSET(unSocket,&read_fds)){

	/*----------------------------------------------------Conexion Nueva--------------------------------------*/
					if(unSocket == fileno(stdin)){
						consolaUMC();
					}
					else if(unSocket == losParametros.sockEscuchador){

		        		unCliente = aceptar(losParametros.sockEscuchador,&addressAceptado);

		        		printf("Nuevo pedido de conexion...\n");

		        		unaCabecera = nuevoHeaderIPC(QUIENSOS);
		        		if(!enviarHeaderIPC(unCliente,unaCabecera)){
		        			printf("HandShake Error - No se pudo enviar mensaje QUIENSOS\n");
		        			liberarHeaderIPC(unaCabecera);
		        			close(unCliente);
		        			continue;
		        		}

		        		if(!recibirHeaderIPC(unCliente,unaCabecera)){
		        			log_error("HandShake Error - No se pudo recibir mensaje de respuesta\n");
		        			liberarHeaderIPC(unaCabecera);
		        			close(unCliente);
		        			continue;
		        		}

	/*------------------------------------Identifico quien se conecto y procedo--------------------------------*/

						switch(unaCabecera->tipo){
							case CONNECTCPU:

								unaCabecera2 = nuevoHeaderIPC(OK);
								if(!enviarHeaderIPC(unCliente, unaCabecera2)){
									log_error("No se pudo enviar un mensaje de confirmacion a la consola conectada");
									liberarHeaderIPC(unaCabecera2);
									close(unCliente);
									continue;
								}
								liberarHeaderIPC(unaCabecera2);
								log_info("Conexion con CPU establecida\n");
								agregarSock=1;

								// enviar tamaño de pagina a CPU
								enviarConfigUMC(unCliente, losParametros.frameSize, losParametros.frameByProc);

								pthread_create(&tid, &attr, (void*)realizarAccionCPU, unCliente);

								FD_CLR(unCliente,&fds_master);
								if(unCliente>losParametros.fdMax){
									unCliente=losParametros.fdMax;
								}
								break;
							case CONNECTNUCLEO:
								unaCabecera2 = nuevoHeaderIPC(OK);
								if(!enviarHeaderIPC(unCliente, unaCabecera2)){
									log_error("No se pudo enviar un mensaje de confirmacion a la consola conectada");
									liberarHeaderIPC(unaCabecera2);
									close(unCliente);
									continue;
								}
								liberarHeaderIPC(unaCabecera2);

								// enviar tamaño de pagina a Nucleo
								enviarConfigUMC(unCliente, losParametros.frameSize, losParametros.frameByProc);

								log_info("Conexion con Nucleo establecida");
								agregarSock=1;

								/*Agrego el socket conectado A la lista Master*/
								if(agregarSock==1){
									FD_SET(unCliente,&(fds_master));
									if (unCliente > losParametros.fdMax){
										maximoAnterior = losParametros.fdMax;
										losParametros.fdMax = unCliente;
									}
									agregarSock=0;
								}
						}
						liberarHeaderIPC(unaCabecera);
					}/*-Cierro if-Conexion Nueva-*/

	/*--------------------------------------Conexion de un cliente existente-------------------------------------*/
					else {

						/* Valido si esta definido unMensaje */
						unMensaje.contenido = malloc(LONGITUD_MAX_DE_CONTENIDO);
						memset(unMensaje.contenido,'\0',LONGITUD_MAX_DE_CONTENIDO);

						if (!recibirMensajeIPC(unSocket,&unMensaje)){ /* Si se cerro una conexion, veo que el socket siga abierto*/

							if(unSocket==losParametros.sockSwap){
								log_error("Se perdio conexion con el swap.. Finalizando UMC.");
								cerrarSockets(&losParametros);
								exit(EXIT_FAILURE);
							}

							/*Saco el socket de la lista Master*/
							FD_CLR(unSocket,&fds_master);
							close (unSocket);

							if (unSocket > losParametros.fdMax){
								maximoAnterior = losParametros.fdMax;
								losParametros.fdMax = unSocket;
							}
							memset(enviolog,'\0',TAMDATOS);
							fflush(stdout);
						}
	        	        else{

	        	        	/* Se sigue comunicado con el Nucleo, podría recibir otros mensajes */

	        	        	switch(unMensaje.header.tipo){

	        	        		case INICIALIZAR_PROGRAMA:


	        	        			if (recibir_paquete(unCliente, &paquete_stPageIni)) {
										printf("No se pudo recibir paquete de inicio de programa");
										close(unCliente);
										return -1;
									}
									unPageIni = (stPageIni*)malloc(sizeof(stPageIni));
									deserializar_inicializar_programa(unPageIni,&paquete_stPageIni);

	        	        			ini = (stIni*)calloc(1,sizeof(stIni));
									ini->marcos_x_proceso = frameByProc;
									ini->socketResp = unCliente;
	        	        			ini->sPI= unPageIni;

	        	        			inicializarPrograma(ini);

	        	        			break;

	        	        		case FINPROGRAMA:

	        	        			end = calloc(1,sizeof(stEnd));
	        	        			end->socketResp = unCliente;
	        	        			end->pid = atoi(unMensaje.contenido);

	        	        			finalizarProgramaNucleo(end);
	        	        			break;


	        	        		default:
	        	        			printf("Se recibio una peticion con un codigo desconocido...%d", unMensaje.header.tipo);
	        	        			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
	        	        			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
	        	        			break;

	        	        	}

	        	        	fflush(stdout);

	        	        }/*Ciero Else comunicacion con el servidor*/

					}/*Cierra Else de WebServer Conocido*/

				}/*Cierro if(FD_ISSET(unSocket,&read_fds))*/

		    }/*Cierro for(unSocket=0;unSocket<=elEstadoActual.fdMax;unSocket++)*/

		}/*Cierro while(elEstadoActual.salir == 0)*/

	/* ............................................Finalizacion............................................. */
		cerrarSockets(&losParametros);
		loadInfo_destruir(&losParametros); // libera losParametros
		destruirMemoriaPrincipal();
		printf("\nSERVER: Fin del programa\n");
		/*loguear(INFO_LOG,"Fin del programa","SERVER");*/
		return EXIT_SUCCESS;

}

