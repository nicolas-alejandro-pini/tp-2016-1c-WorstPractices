/*
 ============================================================================
 Name        : Swap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/sockets.h>
#include <commons/ipctypes.h>
#include <commons/socketsIPCIRC.h>

#include "Swap.h"
#include "particionSwap.h"
#include "gestionAsignacion.h"

int main(void) {
    char* temp_file = "swap.log";
    t_config *config = NULL;
    char *configPath = "swap.config";
    int srvSock;
    int cliSock;
    char terminar = 0;

    uint16_t pID, cantPaginas;
    char * bufferPagina;
    uint16_t nroPagina;
    char * bufferPrograma;
    unsigned long int tamanioPrograma;

    struct sockaddr sockAddress;

    stHeaderIPC *ipcHeader;

    //Configuracion cargada
    t_swap_config loaded_config;

    printf("Arancando el proceso SWAP...\n");

    config = config_create(configPath);
    if(config == NULL){
        printf("Error arracando la configuracion...\n");
    	return EXIT_FAILURE;
    }

    //Cargo la configuracion del proceso y la imprimo en el log
    loaded_config.puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    loaded_config.nombreSwap = config_get_string_value(config, "NOMBRE_SWAP");
    loaded_config.cantidadPaginas = config_get_long_value(config, "CANTIDAD_PAGINAS");
    loaded_config.tamanioPagina = config_get_long_value(config, "TAMANIO_PAGINA");
    loaded_config.retardoCompactacion = config_get_long_value(config, "RETARDO_COMPACTACION");
    loaded_config.retardoAcceso = config_get_long_value(config, "RETARDO_ACCESO");
    loaded_config.logLevel = config_get_int_value(config, "LOG_LEVEL");

    //Primero instancio el log
    t_log* logger = log_create(temp_file, "SWAP",-1, loaded_config.logLevel);

    log_debug("Configuracion cargada satisfactoriamente...");
    log_info("Puerto de escucha: %d", loaded_config.puertoEscucha);
    log_info("Nombre del archivo Swap: %s", loaded_config.nombreSwap);
    log_info("Cantidad de paginas: %d", loaded_config.cantidadPaginas);
    log_info("Tamanio de pagina: %d", loaded_config.tamanioPagina);
    log_info("Retardo de compactacion: %d", loaded_config.retardoCompactacion);
    log_info("Retardo de acceso: %d", loaded_config.retardoAcceso);

    //Creo la particion SWAP
    if(crearParticionSwap(loaded_config.nombreSwap, loaded_config.cantidadPaginas, loaded_config.tamanioPagina) <  0){
    	//Error al crear
    	log_error("Error al crear la particion SWAP");
        log_destroy(logger);
        config_destroy(config);
    	return EXIT_FAILURE;
    }

    log_debug("Partición SWAP creada satisfactoriamente");

    //Creo la gestión de asignación
    if(initGestionAsignacion(&loaded_config) < 0){
    	//Error al arrancar la gestión de asignación de la partición SWAP
    	log_error("Error al arrancar la gestion de asignacion de la partición SWAP");
        log_destroy(logger);
        config_destroy(config);
    	return EXIT_FAILURE;
    }

    log_debug("Gestión de asignación del SWAP inicializada satisfactoriamente");

    //Creo el socket de escucha
    srvSock = escuchar(loaded_config.puertoEscucha);
    if(srvSock == -1){
        log_error("Error creando el socket de escucha...");
        log_destroy(logger);
        config_destroy(config);
        destruirParticionSwap();
    	return EXIT_FAILURE;
    }

    //Inicializo el buffer de la pagina, lo voy a utilizar bastante
    bufferPagina = (char *)malloc(loaded_config.tamanioPagina);

    //----------------------
    // Correr las pruebas aca
    // ELIMINAR ESTO!!!
    //----------------------
//
//    pID = 1;
//    cantPaginas = 3;
//    bufferPrograma = (char *)malloc(1024);
//    strcpy(bufferPrograma, "PROCESO 1");
//	if(asignarEspacioAProceso((unsigned long int) pID, (unsigned long int) cantPaginas, bufferPrograma) < 0){
//		log_error("Error asignando espacio a proceso");
//	}
//    pID = 2;
//    cantPaginas = 3;
//    strcpy(bufferPrograma, "PROCESO 2");
//	if(asignarEspacioAProceso((unsigned long int) pID, (unsigned long int) cantPaginas, bufferPrograma) < 0){
//		log_error("Error asignando espacio a proceso");
//	}
//
//	memset(bufferPrograma, '\0', 1024);
//
//	if(leerPaginaProceso(1, 0, bufferPrograma)){
//		log_error("Error al leer la pagina del proceso");
//	}
//
//	memset(bufferPrograma, '\0', 1024);
//
//	if(leerPaginaProceso(2, 0, bufferPrograma)){
//		log_error("Error al leer la pagina del proceso");
//	}
//
//	log_debug("Buffer PID %d pag %d: %s", 2, 0, bufferPrograma);

//	//---------------------
//	//---------------------
//	//---------------------


    //Arranco a escuchar mensajes
    log_debug("Esperando conexiones...");
    while(!terminar){
    	cliSock = aceptar(srvSock, (struct sockaddr *)&sockAddress);
    	if(cliSock == -1){
            log_error("Error aceptando la conexion del cliente...");
            continue;
    	}

    	log_debug("Nuevo cliente conectado");

    	ipcHeader = nuevoHeaderIPC(QUIENSOS);
    	enviarHeaderIPC(cliSock, ipcHeader);

    	if(recibirHeaderIPC(cliSock, ipcHeader) <= 0){
    		log_error("Cliente desconectado antes de saber quien era");
    		liberarHeaderIPC(ipcHeader);
    		continue;
    	}

    	if(ipcHeader->tipo == SOYUMC){
    		// Se me conecto un UMC por lo cual me quedo escuchando sus mensajes

    		log_info("Se detecta conexion desde UMC");

    		//Le envio un OK a modo respuesta de que el handshake se realizó con éxito
    		ipcHeader->tipo = OK;
    		enviarHeaderIPC(cliSock, ipcHeader);

        	while(1){
            	if(recibirHeaderIPC(cliSock, ipcHeader) <= 0){
            		log_error("La UMC se desconecto u ocurrio un error de comunicacion");
            		liberarHeaderIPC(ipcHeader);
            		terminar = -1;
            		break;
            	}

            	//Tratamiento de los diferentes mensajes desde la UMC
            	switch(ipcHeader->tipo){

					case INICIAR_PROGRAMA:
						//Espero PID(uint16_t), Cantidad paginas (uint16_t), programa (buffer)

						log_debug("Nuevo comando recibido: INICIALIZAR PROGRAMA...");

						//Recibo el PID
						if(sizeof(uint16_t) != recv(cliSock, &pID, sizeof(uint16_t), 0)){
							log_error("Error al recibir el PID del proceso");
						}

						//Recibo la cantidad de paginas
						if(sizeof(uint16_t) != recv(cliSock, &cantPaginas, sizeof(uint16_t), 0)){
							log_error("Error al recibir la cantidad de paginas");
						}

						//Recibo el programa
						tamanioPrograma = ipcHeader->largo - 2 * sizeof(uint16_t);
						bufferPrograma = (char *)malloc(tamanioPrograma);
						if(recv(cliSock, bufferPrograma, tamanioPrograma, 0) != tamanioPrograma){
							log_error("Error recibiendo el programa");
						}

						log_info("Inicia PID: %d con %d paginas", pID, cantPaginas);
						log_debug("Programa: %s", bufferPrograma);

						//Llamo a la funcion de asignacion y armo la respuesta
						ipcHeader->tipo = OK;
						if(asignarEspacioAProceso((unsigned long int) pID, (unsigned long int) cantPaginas, bufferPrograma) < 0){
							log_error("Error asignando espacio a proceso");
							ipcHeader->tipo = ERROR;
						}
						free(bufferPrograma);

						if(enviarHeaderIPC(cliSock, ipcHeader) != sizeof(stHeaderIPC)){
							log_error("Error al enviar la respuesta a la UMC");
						}

						break;

					case DESTRUIR_PROGRAMA:

						log_debug("Nuevo comando recibido: DESTRUIR PROGRAMA...");

						//Recibo el PID
						if(sizeof(uint16_t) != recv(cliSock, &pID, sizeof(uint16_t), 0)){
							log_error("Error al recibir el PID del proceso");
						}

						log_info("Finaliza PID: %d", pID);

						//LLamo a la funcion de asignacion y armo la respuesta
						ipcHeader->tipo = OK;
						if(liberarEspacioDeProceso((unsigned long int)pID)<0){
							log_error("Error al liberar el espacio del proceso");
							ipcHeader->tipo = ERROR;
						}

						if(enviarHeaderIPC(cliSock, ipcHeader) != sizeof(stHeaderIPC)){
							log_error("Error al enviar la respuesta a la UMC");
						}

						break;

					case LEER_PAGINA:

						log_debug("Nuevo comando recibido: LEER PAGINA...");

						ipcHeader->tipo = OK;
						//Recibo el PID
						if(sizeof(uint16_t) != recv(cliSock, &pID, sizeof(uint16_t), 0)){
							log_error("Error al recibir el PID del proceso");
							ipcHeader->tipo = ERROR;
						}

						//Recibo el numero de pagina
						if(sizeof(uint16_t) != recv(cliSock, &nroPagina, sizeof(uint16_t), 0)){
							log_error("Error al recibir el numero de paginas");
							ipcHeader->tipo = ERROR;
						}

						//Leo la pagina del proceso
						ipcHeader->tipo = OK;
						if(leerPaginaProceso((unsigned long int)pID, (unsigned long int)nroPagina, bufferPagina) < 0){
							log_error("Error al leer la pagina desde el SWAP. pID %d #pag %d", pID, nroPagina);
							ipcHeader->tipo = ERROR;
						}

						log_info("Lectura PID: %d, pagina: %d", pID, nroPagina);
						log_debug("Pagina leida: %s", bufferPagina);

						//Envio la respuesta a la UMC
						ipcHeader->largo = loaded_config.tamanioPagina;
						if(enviarHeaderIPC(cliSock, ipcHeader) != sizeof(stHeaderIPC)){
							log_error("Error al enviar la respuesta a la UMC");
						}else{
							if(ipcHeader->tipo == OK)
								if(send(cliSock, bufferPagina, loaded_config.tamanioPagina, 0) != loaded_config.tamanioPagina){
									log_error("Error escribiendo la respuesta a la UMC");
								}
						}

						break;

					case ESCRIBIR_PAGINA:

						log_debug("Nuevo comando recibido: ESCRIBIR PAGINA...");

						//Recibo el PID
						if(sizeof(uint16_t) != recv(cliSock, &pID, sizeof(uint16_t), 0)){
							log_error("Error al recibir el PID del proceso");
							ipcHeader->tipo = ERROR;
						}

						//Recibo el numero de pagina
						if(sizeof(uint16_t) != recv(cliSock, &nroPagina, sizeof(uint16_t), 0)){
							log_error("Error al recibir el numero de paginas");
							ipcHeader->tipo = ERROR;
						}

						if(recv(cliSock, bufferPagina, loaded_config.tamanioPagina, 0) != loaded_config.tamanioPagina){
							log_error("Error al recibir la pagina");
							ipcHeader->tipo = ERROR;
						}

						log_info("Escritura PID: %d, pagina: %d", pID, nroPagina);
						log_debug("Pagina a escribir: %s", bufferPagina);

						//Escribo la pagina del proceso
						ipcHeader->tipo = OK;
						if(escribirPaginaProceso((unsigned long int)pID, (unsigned long int)nroPagina, bufferPagina) < 0){
							log_error("Error al escribir la pagina en el SWAP. pID %d #pag %d", pID, nroPagina);
							ipcHeader->tipo = ERROR;
						}

						//Envio la respuesta a la UMC
						ipcHeader->largo = loaded_config.tamanioPagina;
						if(enviarHeaderIPC(cliSock, ipcHeader) != sizeof(stHeaderIPC)){
							log_error("Error al enviar la respuesta a la UMC");
						}

						break;
					}

        	}
        	//liberarHeaderIPC(ipcHeader);
    	} else {
    		// Se me conecto cualquiera, lo tengo que rechazar
    		liberarHeaderIPC(ipcHeader);
    		close(cliSock);
    	}
    }

    //Libero lo reservado
    log_destroy(logger);
    config_destroy(config);
    destruirParticionSwap();

    log_info("Proceso Swap finalizado");

	return EXIT_SUCCESS;
}

