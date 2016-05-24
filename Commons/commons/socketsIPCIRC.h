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
#ifndef SOCKETSIPCIRC_H_
#define SOCKETSIPCIRC_H_

	#include "../commons/ipctypes.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>
	#include <netdb.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

	/*----------------------------------------------------------------------------*/
	/*                     Definiciones y Declaraciones                           */
	/*----------------------------------------------------------------------------*/

	/*
	 * Estructura Header de la comunicación IPC
	 *
	 * @id: id del mensaje que se está transmitiendo, es un código que debe ser único
	 * @respuesta_a_id: id del mensaje original que genera el actual como respuesta
	 * @tipo: tipo de mensaje, que representa?
	 * @largo: es el largo del contenido
	 *
	 */
	typedef struct
	{
		char id[16];
		char respuesta_a_id[16]; 	//No se si va a ser necesario, pero a veces esto
									//ayuda en la comunicacion asincronica
		unsigned long int tipo;
		unsigned long largo;
	/*} __attribute__((packed)) stHeaderIPC; */
	} stHeaderIPC;

	typedef struct
	{
		stHeaderIPC header;
		char *contenido;	/* } __attribute__((packed)) stMensajeIPC;*/
	}stMensajeIPC;

	#define CONNECTION_CLOSED 0
	#define CONFIG_UMC 1

	typedef uint16_t t_htons;
	typedef uint32_t t_htonl;
	typedef uint16_t t_buffer;

	typedef struct{
		t_htons type;
		t_htonl length;
	} __attribute__ ((__packed__)) t_header;

	typedef struct{
		t_header header;
		t_buffer *data;
	}  t_paquete;

	typedef struct{
		int paginasXProceso;
		int tamanioPagina;
	//} __attribute__((packed)) t_UMCConfig;
	} t_UMCConfig;

	/*----------------------------------------------------------------------------*/
	/*                         Funciones Privadas                                 */
	/*----------------------------------------------------------------------------*/

	stHeaderIPC * nuevoHeaderIPC(unsigned long unTipo);

	/*----------------------------------------------------------------------------*/

	int enviarHeaderIPC(int unSocket, stHeaderIPC *unHeader);

	/*----------------------------------------------------------------------------*/

	int recibirHeaderIPC(int unSocket, stHeaderIPC* nuevoPtrAHeader);

	/*----------------------------------------------------------------------------*/
	/*                         Funciones Basicas                                  */
	/*----------------------------------------------------------------------------*/

	int enviarMensajeIPC(int unSocket,stHeaderIPC * unHeader, char* unContenido);

	/*----------------------------------------------------------------------------*/
	
	int recibirMensajeIPC(int unSocket, stMensajeIPC* unNuevoMensaje);

	/*----------------------------------------------------------------------------*/
	/*                         Manejo de paquetes                                 */
	/*----------------------------------------------------------------------------*/


	t_buffer* serializar_header(t_paquete *paquete, int32_t *offset);
	void deserializar_header(t_header *buf_header, int32_t *offset, t_header *header);
	int32_t* serializar_campo(t_buffer *buffer, int32_t *offset, void *campo, int32_t size);
	int enviar_paquete(int sockfd, t_paquete *paquete);
	int recibir_paquete(int sockfd, t_paquete *paquete);
	int recibir_header(int sockfd, t_header *header);
	void free_paquete(t_paquete *paquete);

	/** Estructuras especificas **/
	int serializarConfigUMC(t_paquete *paquete, t_UMCConfig *self);
	int deserializarConfigUMC(t_UMCConfig *self, t_paquete *paquete);


#endif /* SOCKETSIPCIRC_H_ */
