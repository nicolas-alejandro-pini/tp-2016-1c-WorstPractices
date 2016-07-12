/*
 * ISwap.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#include "ISwap.h"

int inicializarSwap(stPageIni *st){
	stHeaderIPC* mensaje;
	int ret=EXIT_SUCCESS;

	mensaje = nuevoHeaderIPC(INICIAR_PROGRAMA);
	mensaje->largo = 2*sizeof(uint16_t) + strlen(st->programa) + 1;

	enviarHeaderIPC(losParametros.sockSwap, mensaje);

	send(losParametros.sockSwap, &st->processId, sizeof(uint16_t), 0);
	send(losParametros.sockSwap, &st->cantidadPaginas, sizeof(uint16_t), 0);
	send(losParametros.sockSwap, st->programa, strlen(st->programa) + 1, 0);

	recibirHeaderIPC(losParametros.sockSwap, mensaje);

	if(mensaje->tipo != OK){
		log_error("Error al inicializar el proceso %d en el swap", st->processId);
		ret = EXIT_FAILURE;
	}

	liberarHeaderIPC(mensaje);
	return ret;
}

//define INICIAR_PROGRAMA	141ul /* pid (ui), cantidad paginas (ui), (ul), codigo prg (char*) */
//define DESTRUIR_PROGRAMA	142ul /* pid */
//define LEER_PAGINA			143ul /* pid, numero de pagina */
//define ESCRIBIR_PAGINA		144ul /* pid, numero de pagina, contenido de pagina */

/*
 * enviarPagina()<- NULL
 * recibir pagina()<-contenido de la pagina
 *
 * destruirPrograma()<-NULL
 */
int enviarPagina(uint16_t pid, uint16_t pagina, char* buffer){
	stHeaderIPC* mensaje;
	int ret=EXIT_SUCCESS;

	mensaje = nuevoHeaderIPC(ESCRIBIR_PAGINA);
	mensaje->largo = 2*sizeof(uint16_t) + losParametros.frameSize;

	enviarHeaderIPC(losParametros.sockSwap, mensaje);

	send(losParametros.sockSwap, &pid, sizeof(uint16_t), 0);
	send(losParametros.sockSwap, &pagina, sizeof(uint16_t), 0);
	send(losParametros.sockSwap, buffer, losParametros.frameSize, 0);

	recibirHeaderIPC(losParametros.sockSwap, mensaje);

	if(mensaje->tipo != OK){
		log_error("Error al escribir pagina %d del proceso %d en el swap", pagina, pid);
		ret = EXIT_FAILURE;
	}

	liberarHeaderIPC(mensaje);

	return ret;
}


// TODO: Estos pedidos deberian estar sincronizados? Atiende por otra coneccion?
char* recibirPagina(uint16_t pid, uint16_t pagina){

	unsigned char* paginaSwap;
	stHeaderIPC* mensaje;
	int recibidos;

	mensaje = nuevoHeaderIPC(LEER_PAGINA);
	mensaje->largo = 2*sizeof(uint16_t);

	enviarHeaderIPC(losParametros.sockSwap, mensaje);

	send(losParametros.sockSwap, &pid, sizeof(uint16_t), 0);
	send(losParametros.sockSwap, &pagina, sizeof(uint16_t), 0);

	recibirHeaderIPC(losParametros.sockSwap, mensaje);

	if(mensaje->tipo != OK){
		log_error("Error al leer pagina %d del proceso %d desde el swap", pagina, pid);
		liberarHeaderIPC(mensaje);
		return NULL;
	}
	liberarHeaderIPC(mensaje);

	paginaSwap = malloc(losParametros.frameSize);
	recibidos = recv(losParametros.sockSwap, paginaSwap, losParametros.frameSize, 0);

	if(recibidos!= losParametros.frameSize){
		log_error("Error al recibir bytes de la pagina %d del proceso %d desde el swap", pagina, pid);
		return NULL;
	}

	return paginaSwap;
}

int destruirPrograma(uint16_t pid){
	stHeaderIPC* mensaje;
	int ret=EXIT_SUCCESS;

	mensaje = nuevoHeaderIPC(DESTRUIR_PROGRAMA);
	mensaje->largo = sizeof(uint16_t);

	enviarHeaderIPC(losParametros.sockSwap, mensaje);

	send(losParametros.sockSwap, &pid, sizeof(uint16_t), 0);

	recibirHeaderIPC(losParametros.sockSwap, mensaje);

	if(mensaje->tipo != OK){
		log_error("Error al desruir programa %d en swap", pid);
		ret=EXIT_FAILURE;
	}

	liberarHeaderIPC(mensaje);

	return ret;
}
