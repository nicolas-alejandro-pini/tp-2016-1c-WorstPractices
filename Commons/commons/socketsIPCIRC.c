/*------------------------------------------------------------------------------*/
/*                --------------------------------------------			*/
/*                 BIBLIOTECA PARA MANEJO DE SOCKETS EN ANSI C			*/
/*                --------------------------------------------			*/
/*										*/
/*  Nombre: socketsIPCIRC.c							*/
/*  Versión: 1.5								*/
/*  Modificado: 02 de Julio de 2009						*/
/*  Description: Implementación de la biblioteca sockets			*/
/*               Modificacion para trabajar con otro tipo de mensajes		*/
/*------------------------------------------------------------------------------*/

#include "socketsIPCIRC.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include "log.h"
#include "sockets.h"

/*----------------------------------------------------------------------------*/
/*                         Funciones Privadas                                 */
/*----------------------------------------------------------------------------*/

/*
 * Devuelve una cadena que representa un numero aleatorio de 15 cifras
 *
 */
void nuevoID(char *id)
{
	char i;

	srand(time(NULL));
	for(i=0;i<15;i++){
		*(id + i) = (char)rand();
	}
	id[15] = '\0';
}

/**
 *  Devuelve un nuevo header con los campos cargados.
 *
 */
stHeaderIPC * nuevoHeaderIPC(unsigned long unTipo){

	stHeaderIPC * unHeader = malloc(sizeof(stHeaderIPC));
	nuevoID(unHeader->id);
	unHeader->tipo = unTipo;
	unHeader->largo = 0;
	return (unHeader);
}

void liberarHeaderIPC(stHeaderIPC *unHeader){
	free(unHeader);
}

/*----------------------------------------------------------------------------*/

int enviarHeaderIPC(int unSocket, stHeaderIPC *unHeader)
/*Envía unHeader por el socket que recibe como parametro y devuelve la cantidad enviada.*/
{
	int resultado;
	if((resultado = send(unSocket, unHeader, sizeof(stHeaderIPC),0)) == -1)
		log_error("No se pudo enviar el header!");
	/*printf("MANDE:%d\n",unHeader.tipo);*/
	return(resultado);
}

/*----------------------------------------------------------------------------*/

/*Devuelve el tamaño del header recibido atraves de unSocket y 0 si se cerró.*/
int recibirHeaderIPC(int unSocket, stHeaderIPC* nuevoPtrAHeader){
	return recv(unSocket, nuevoPtrAHeader,sizeof(stHeaderIPC),0);
}

/*----------------------------------------------------------------------------*/
/*                         Funciones Basicas                                  */
/*----------------------------------------------------------------------------*/

int enviarMensajeIPC(int unSocket,stHeaderIPC *unHeader, char* unContenido)
/*Envía primero unHeader y luego unContenido por el socket que recibe como parametro. Devuelve 0 si hubieron errores y el tamaño del contenido enviado -sin contar el header- si no.*/
{
	if (enviarHeaderIPC(unSocket, unHeader) <= 0)
		return(0);

	return(enviarContenido(unSocket,unContenido));
}

/*----------------------------------------------------------------------------*/

int recibirMensajeIPC(int unSocket, stMensajeIPC* unNuevoMensaje)
/*Devuelve la cantidad recibida -sin contar el header- si se pudo recibir un mensaje atraves de unSocket y 0 si este se cerro.*/
{
	if(!(recibirHeaderIPC(unSocket, &(unNuevoMensaje->header))))
		return(0);

	if (unNuevoMensaje->header.largo > 0 )
		return(recibirContenido(unSocket,unNuevoMensaje->contenido,unNuevoMensaje->header.largo));
	else {
	
		return 1;
	}
}
