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

#include "commons/log.h"
#include "commons/config.h"
#include "commons/sockets.h"

int main(void) {
    char* temp_file = "swap.log";
    t_config *config = NULL;
    char *configPath = "swap.config";
    int sockId;
    unsigned char terminar = 0;
    struct sockaddr_in sockAddress;
    char * clientIPAddress;
    int r;

    //Configuracion cargada
    int puertoEscucha;
    char *nombreSwap;
    int cantidadPaginas;
    int tamanioPagina;
    int retardoCompactacion;

    //Primero instancio el log
    t_log* logger = log_create(temp_file, "SWAP",true, LOG_LEVEL_INFO);

    log_info(logger, "Arancando el proceso SWAP...");

    config = config_create(configPath);
    if(config == NULL){
        log_error(logger, "Error arracando la configuracion...");
        log_destroy(logger);
    	return EXIT_FAILURE;
    }
    log_info(logger, "Configuracion cargada satisfactoriamente...");

    //Cargo la configuracion del proceso y la imprimo en el log
    puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    nombreSwap = config_get_string_value(config, "NOMBRE_SWAP");
    cantidadPaginas = config_get_int_value(config, "CANTIDAD_PAGINAS");
    tamanioPagina = config_get_int_value(config, "TAMANIO_PAGINA");
    retardoCompactacion = config_get_int_value(config, "RETARDO_COMPACTACION");

    log_info(logger, "Puerto de escucha: %d", puertoEscucha);
    log_info(logger, "Nombre del archivo Swap: %s", nombreSwap);
    log_info(logger, "Cantidad de paginas: %d", cantidadPaginas);
    log_info(logger, "Tamanio de pagina: %d", tamanioPagina);
    log_info(logger, "Retardo de compactacion: %d", retardoCompactacion);

    //Creo el socket de escucha
    sockId = escuchar(puertoEscucha);
    if(sockId == -1){
        log_error(logger, "Error creando el socket de escucha...");
        log_destroy(logger);
        config_destroy(config);
    	return EXIT_FAILURE;
    }
    //Arranco a escuchar mensajes
    log_info(logger, "Esperando conexiones..");
    while(!terminar){
    	r = aceptar(sockId, (struct sockAddress *)&sockAddress);
    	if(r == -1){
            log_error(logger, "Error aceptando la conexion del cliente...");
            continue;
    	}
    	//clientIPAddress = inet_ntoa(sockAddress.sin_addr);
        //log_info(logger, "Nuevo cliente conectado desde la IP: %s", clientIPAddress);
    	log_info(logger, "Nuevo cliente conectado");



    }

    //Libero lo reservado
    log_destroy(logger);
    config_destroy(config);
	return EXIT_SUCCESS;
}

