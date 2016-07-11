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
#include "serializador.h"

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
	int i;
	stHeaderIPC * unHeader = malloc(sizeof(stHeaderIPC));
	nuevoID( (char*) unHeader->id);
	unHeader->tipo = unTipo;
	unHeader->largo = 0;
	for(i=0; i<16; i++){
		unHeader->respuesta_a_id[i] = 0;
	}
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

	return(enviarContenido(unSocket,unContenido,unHeader->largo));
}

/*----------------------------------------------------------------------------*/

int recibirMensajeIPC(int unSocket, stMensajeIPC* unNuevoMensaje)
/*Devuelve la cantidad recibida -sin contar el header- si se pudo recibir un mensaje atraves de unSocket y 0 si este se cerro.*/
{
	if(!(recibirHeaderIPC(unSocket, &(unNuevoMensaje->header))))
		return(0);

	unNuevoMensaje->contenido = (char *) malloc(unNuevoMensaje->header.largo);

	if (unNuevoMensaje->header.largo > 0 )
		return(recibirContenido(unSocket,unNuevoMensaje->contenido,unNuevoMensaje->header.largo));
	else {
	
		return 1;
	}
}


/*----------------------------------------------------------------------------*/
/*                         Manejo de paquetes                                 */
/*----------------------------------------------------------------------------*/

void crear_paquete(t_paquete *paquete, int type){
	t_header header;
	header.type = type;
	header.length = 0;

	// Copio Header
	memcpy(&(paquete->header), &header, sizeof(t_header));
	// Reservo espacio para header
	paquete->data= malloc(sizeof(t_header));
}

void free_paquete(t_paquete *paquete){
	free(paquete->data);
}

int obtener_paquete_type(t_paquete *paquete){
	return paquete->header.type;
}

int recibir_paquete(int sockfd, t_paquete *paquete){
	uint32_t size = 0;

	// recibo header
	if(recibir_header(sockfd, &(paquete->header)))
		return EXIT_FAILURE;

	// obtengo payload
	paquete->data = malloc(paquete->header.length);
	size = recv(sockfd, paquete->data, paquete->header.length, MSG_WAITALL);

	if(size == 0)  // CONNECTION CLOSED
	{
		paquete->header.type = CONNECTION_CLOSED;
		paquete->header.length = 0;
	}
	if(size == paquete->header.length)
		return EXIT_SUCCESS;

	paquete->header.type = BROKENPIPE;
	paquete->header.length = 0;
	perror("recv_payload en recibirPaquete");
	free(paquete->data);
	return EXIT_FAILURE;
}

int recibir_header(int sockfd, t_header *header){
	int32_t size_header = sizeof(t_header);
	int32_t tmp_size = 0, offset =0;
	t_header *buf_header = malloc(sizeof(t_header));

	tmp_size = recv(sockfd, buf_header, size_header, MSG_WAITALL);
	if(tmp_size == 0)  // CONNECTION CLOSED
	{
		header->type = CONNECTION_CLOSED;
		header->length = 0;
	}
	if(tmp_size == size_header)
	{
		deserializar_header(buf_header, &offset, header);  // guardo header
		free(buf_header);
		return EXIT_SUCCESS;
	}
	perror("recv_header");
	free(buf_header);
	return EXIT_FAILURE;
}

int enviar_paquete(int sockfd, t_paquete *paquete){
	int32_t enviados = 0, ret = 0, size_paquete = 0;

	size_paquete = sizeof(t_header) + paquete->header.length;
	while(enviados < size_paquete)
	{
		ret = send(sockfd, paquete->data, size_paquete , 0);
		if(ret < 0)
			return EXIT_FAILURE;
		enviados += ret;
	}
	return EXIT_SUCCESS;
}
