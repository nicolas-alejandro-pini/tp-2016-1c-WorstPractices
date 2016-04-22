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

	#include <ipctypes.h>

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
		unsigned int tipo;
		unsigned long largo;
	/*} __attribute__((packed)) stHeaderIPC; */
	} stHeaderIPC;

	typedef struct
	{
		stHeaderIPC header;
		char *contenido;	/* } __attribute__((packed)) stMensajeIPC;*/
	}stMensajeIPC;

	/*----------------------------------------------------------------------------*/
	/*                         Funciones Privadas                                 */
	/*----------------------------------------------------------------------------*/

	stHeaderIPC nuevoHeaderIPC(const char unTipo);

	/*----------------------------------------------------------------------------*/

	char *stringHeaderIPC(void* elHeader);

	/*----------------------------------------------------------------------------*/

	stHeaderIPC HeaderStringIPC(const char *cadena);

	/*----------------------------------------------------------------------------*/

	int enviarHeaderIPC(int unSocket, const stHeaderIPC unHeader);

	/*----------------------------------------------------------------------------*/

	int recibirHeaderIPC(int unSocket, stHeaderIPC* nuevoPtrAHeader);

	/*----------------------------------------------------------------------------*/
	/*                         Funciones Basicas                                  */
	/*----------------------------------------------------------------------------*/

	int enviarMensajeIPCA(int unSocket,stHeaderIPC unHeader, char* unContenido);

	int enviarMensajeIPC(int unSocket,stHeaderIPC unHeader, char* unContenido);

	/*----------------------------------------------------------------------------*/
	
	int recibirMensajeIPC(int unSocket, stMensajeIPC* unNuevoMensaje);

#endif /* SOCKETSIPCIRC_H_ */
