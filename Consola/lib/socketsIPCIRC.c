/*----------------------------------------------------------------------------*/
/*                --------------------------------------------                */
/*                 BIBLIOTECA PARA MANEJO DE SOCKETS EN ANSI C                */
/*                --------------------------------------------                */
/*                                                                            */
/*  Nombre: sockets.c                                                         */
/*  Versión: 1.0                                                              */
/*  Fecha: 18 de Abril de 2016					              				  */
/*  Description: Implementación de la biblioteca para mensajes IPCIRC         */
/*                                                                            */
/*----------------------------------------------------------------------------*/


	#include <dirent.h>
	#include <signal.h>
	#include <sys/time.h>
	#include <sys/stat.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include "librerias.h"
#define LONGITUD_MAX_DE_CONTENIDO 	1024

/*----------------------------------------------------------------------------*/
/*                     Definiciones y Declaraciones                           */
/*----------------------------------------------------------------------------*/

typedef struct
{
	char id[16];
	char tipo;
	int largo;
/*} __attribute__((packed)) stHeaderIPC; */
} stHeaderIPC; 

typedef struct
{
	stHeaderIPC header;
	char contenido[LONGITUD_MAX_DE_CONTENIDO];/* } __attribute__((packed)) stMensajeIPC;*/
}stMensajeIPC;	

/*----------------------------------------------------------------------------*/
/*                         Funciones Privadas                                 */
/*----------------------------------------------------------------------------*/
/* Devuelve un nuevo header con los campos cargados. */
stHeaderIPC nuevoHeaderIPC(const char unTipo){
	int i;
	stHeaderIPC unHeader;
	char *unID = nuevoID();
	for(i = 0;i <16;i++) unHeader.id[i] = *(unID+i);
	unHeader.tipo = unTipo;
	unHeader.largo = 0;
	return (unHeader);
}

/*----------------------------------------------------------------------------*/
/*Envía unHeader por el socket que recibe como parametro y devuelve la cantidad enviada.*/
int enviarHeaderIPC(int unSocket, const stHeaderIPC unHeader){
	int resultado;

	if((resultado = send(unSocket, &unHeader, sizeof(stHeaderIPC),0)) == -1)
		error("No se pudo enviar el header!");
	return(resultado);
}

/*----------------------------------------------------------------------------*/
/*Devuelve el tamaño del header recibido atraves de unSocket y 0 si se cerró.*/
int recibirHeaderIPC(int unSocket, stHeaderIPC* nuevoPtrAHeader){
	int resultado, tamanioHeader = sizeof(stHeaderIPC);
	resultado = recv(unSocket, nuevoPtrAHeader,tamanioHeader,0);

	if(resultado == -1)
		return -1;
	return(resultado);
}

/*----------------------------------------------------------------------------*/
/*                         Funciones Basicas                                  */
/*----------------------------------------------------------------------------*/
/*Envía primero unHeader y luego unContenido por el socket que recibe como parametro. Devuelve 0 si hubieron errores y el tamaño del contenido enviado -sin contar el header- si no.*/
int enviarMensajeIPCA(int unSocket,stHeaderIPC unHeader, char* unContenido){
	if (!enviarHeaderIPC(unSocket, unHeader))
		return(0);

	return(enviarContenidoA(unSocket,unContenido,unHeader.largo));
}

/*Envía primero unHeader y luego unContenido por el socket que recibe como parametro. Devuelve 0 si hubieron errores y el tamaño del contenido enviado -sin contar el header- si no.*/
int enviarMensajeIPC(int unSocket,stHeaderIPC unHeader, char* unContenido){
	int unLargo = strlen(unContenido)+1;
	unHeader.largo = unLargo;
	if (!enviarHeaderIPC(unSocket, unHeader))
		return(0);

	return(enviarContenido(unSocket,unContenido));
}

/*----------------------------------------------------------------------------*/

/*Devuelve la cantidad recibida -sin contar el header- si se pudo recibir un mensaje atraves de unSocket y 0 si este se cerro.*/
int recibirMensajeIPC(int unSocket, stMensajeIPC* unNuevoMensaje){
	if(!(recibirHeaderIPC(unSocket, &(unNuevoMensaje->header))))
		return(0);

	if (unNuevoMensaje->header.largo > 0 )
		return(recibirContenido(unSocket,unNuevoMensaje->contenido,unNuevoMensaje->header.largo));
	else {
	
		return 1;
	}
}

