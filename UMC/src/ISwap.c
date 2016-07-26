/*
 * ISwap.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#include "ISwap.h"

pthread_mutex_t swap;

int inicializarSwap(stPageIni *st){
	stHeaderIPC* mensaje;
	int ret=EXIT_SUCCESS;

	pthread_mutex_lock(&swap);

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

	pthread_mutex_unlock(&swap);

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

	pthread_mutex_lock(&swap);

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

	pthread_mutex_unlock(&swap);

	return ret;
}


// TODO: Estos pedidos deberian estar sincronizados? Atiende por otra coneccion?
char* recibirPagina(uint16_t pid, uint16_t pagina){

	char* paginaSwap;
	stHeaderIPC* mensaje;
	int recibidos;

	pthread_mutex_lock(&swap);

	mensaje = nuevoHeaderIPC(LEER_PAGINA);
	mensaje->largo = 2*sizeof(uint16_t);

	if(enviarHeaderIPC(losParametros.sockSwap, mensaje)<=0){
		log_error("Error al enviar la solicitud de pagina al Swap");
		return NULL;
	}

	send(losParametros.sockSwap, &pid, sizeof(uint16_t), 0);
	send(losParametros.sockSwap, &pagina, sizeof(uint16_t), 0);

	if(recibirHeaderIPC(losParametros.sockSwap, mensaje)<=0){
		log_error("Error al recibir la pagina desde el Swap");
		return NULL;
	}

	if(mensaje->tipo != OK){
		log_error("Error al leer pagina %d del proceso %d desde el swap", pagina, pid);
		liberarHeaderIPC(mensaje);
		return NULL;
	}

	paginaSwap = malloc(mensaje->largo);
	recibidos = recv(losParametros.sockSwap, paginaSwap, mensaje->largo, 0);

	if(recibidos != mensaje->largo){
		log_error("Error al recibir bytes de la pagina %d del proceso %d desde el swap", pagina, pid);
		return NULL;
	}

	liberarHeaderIPC(mensaje);

	pthread_mutex_unlock(&swap);

	return paginaSwap;
}

int destruirPrograma(uint16_t pid){
	stHeaderIPC* mensaje;
	int ret=EXIT_SUCCESS;

	pthread_mutex_lock(&swap);

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

	pthread_mutex_unlock(&swap);

	return ret;
}
