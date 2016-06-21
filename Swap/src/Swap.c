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

#include "particionSwap.h"
#include "gestionAsignacion.h"

int main(void) {
    char* temp_file = "swap.log";
    t_config *config = NULL;
    char *configPath = "swap.config";
    int srvSock;
    int cliSock;
    char terminar = 0;

    struct sockaddr sockAddress;

    stHeaderIPC *ipcHeader;

    //Configuracion cargada
    int puertoEscucha;
    char *nombreSwap;
    long cantidadPaginas;
    long tamanioPagina;
    long retardoCompactacion;

    //Primero instancio el log
    t_log* logger = log_create(temp_file, "SWAP",-1, LOG_LEVEL_INFO);

    log_info("Arancando el proceso SWAP...");

    config = config_create(configPath);
    if(config == NULL){
        log_error("Error arracando la configuracion...");
        log_destroy(logger);
    	return EXIT_FAILURE;
    }
    log_info("Configuracion cargada satisfactoriamente...");

    //Cargo la configuracion del proceso y la imprimo en el log
    puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    nombreSwap = config_get_string_value(config, "NOMBRE_SWAP");
    cantidadPaginas = config_get_long_value(config, "CANTIDAD_PAGINAS");
    tamanioPagina = config_get_long_value(config, "TAMANIO_PAGINA");
    retardoCompactacion = config_get_long_value(config, "RETARDO_COMPACTACION");

    log_info("Puerto de escucha: %d", puertoEscucha);
    log_info("Nombre del archivo Swap: %s", nombreSwap);
    log_info("Cantidad de paginas: %d", cantidadPaginas);
    log_info("Tamanio de pagina: %d", tamanioPagina);
    log_info("Retardo de compactacion: %d", retardoCompactacion);

    //Creo la particion SWAP
    if(crearParticionSwap(nombreSwap, cantidadPaginas, tamanioPagina) <  0){
    	//Error al crear
    	log_error("Error al crear la particion SWAP");
        log_destroy(logger);
        config_destroy(config);
    	return EXIT_FAILURE;
    }

    //Creo el socket de escucha
    srvSock = escuchar(puertoEscucha);
    if(srvSock == -1){
        log_error("Error creando el socket de escucha...");
        log_destroy(logger);
        config_destroy(config);
        destruirParticionSwap();
    	return EXIT_FAILURE;
    }

    //Arranco a escuchar mensajes
    log_info("Esperando conexiones...");
    while(!terminar){
    	cliSock = aceptar(srvSock, (struct sockaddr *)&sockAddress);
    	if(cliSock == -1){
            log_error("Error aceptando la conexion del cliente...");
            continue;
    	}

    	log_info("Nuevo cliente conectado");

    	ipcHeader = nuevoHeaderIPC(QUIENSOS);
    	enviarHeaderIPC(cliSock, ipcHeader);

    	if(recibirHeaderIPC(cliSock, ipcHeader) <= 0){
    		log_error("Cliente desconectado antes de saber quien era");
    		liberarHeaderIPC(ipcHeader);
    		continue;
    	}

    	if(ipcHeader->tipo == SOYUMC){
    		// Se me conecto un UMC por lo cual me quedo escuchando sus mensajes

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

            		break;

            	case DESTRUIR_PROGRAMA:

            		break;

            	case LEER_PAGINA:

            		break;

            	case ESCRIBIR_PAGINA:

            		break;
            	}

        	}
        	liberarHeaderIPC(ipcHeader);
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

	return EXIT_SUCCESS;
}

