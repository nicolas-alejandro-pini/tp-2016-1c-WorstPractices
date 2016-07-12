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
		uint8_t id[16];
		uint8_t respuesta_a_id[16]; 	//No se si va a ser necesario, pero a veces esto
									//ayuda en la comunicacion asincronica
		uint32_t tipo;
		uint32_t largo;
	/*} __attribute__((packed)) stHeaderIPC; */
	} stHeaderIPC;

	typedef struct
	{
		stHeaderIPC header;
		uint8_t *contenido;	/* } __attribute__((packed)) stMensajeIPC;*/
	}stMensajeIPC;

	/*
	 * Estructura de paquete
	 *
	 * @t_buffer: estructura minima del paquete
	 * @respuesta_a_id: id del mensaje original que genera el actual como respuesta
	 * @tipo: tipo de mensaje, que representa?
	 * @largo: es el largo del contenido
	 *
	 */
	typedef uint8_t t_buffer;

	typedef struct{
		uint8_t type;
		uint16_t length;
	} __attribute__ ((__packed__)) t_header;

	typedef struct{
		t_header header;
		t_buffer *data;
	}  t_paquete;

	/** Primitivas del cliente **/
	void crear_paquete(t_paquete *paquete, int type);
	int enviar_paquete(int sockfd, t_paquete *paquete);

	/** Primitivas del servidor **/
	int recibir_paquete(int sockfd, t_paquete *paquete);
	int recibir_header(int sockfd, t_header *header);
	int obtener_paquete_type(t_paquete *paquete);

	/** Primitivas comunes **/
	void free_paquete(t_paquete *paquete);

	/*----------------------------------------------------------------------------*/
	/*                         Funciones Privadas                                 */
	/*----------------------------------------------------------------------------*/

	stHeaderIPC * nuevoHeaderIPC(unsigned long unTipo);

	void liberarHeaderIPC(stHeaderIPC *unHeader);

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

	/* Por cada recibirMensajeIPC debe haber un liberarMensajeIPC, libera el contenido */
	void liberarMensajeIPC(stMensajeIPC* unNuevoMensaje);

#endif /* SOCKETSIPCIRC_H_ */
