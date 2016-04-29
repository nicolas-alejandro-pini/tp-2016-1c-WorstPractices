/*----------------------------------------------------------------------------*/
/*                --------------------------------------------                */
/*                 BIBLIOTECA PARA MANEJO DE SOCKETS EN ANSI C                */
/*                --------------------------------------------                */
/*----------------------------------------------------------------------------*/
#ifndef SOCKETS_H_
#define SOCKETS_H_

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>

	/*----------------------------------------------------------------------------*/
	/*                     Definiciones y Declaraciones                           */
	/*----------------------------------------------------------------------------*/

	#define LONGITUD_MAXIMA_DE_CONTENIDO 8000
	#define CONNECT_TIMEOUT 1

	typedef struct
	{
		char id[16];
		unsigned char tipo;
		char ttl;
		char hops;
		int largo;
	} __attribute__((packed)) stHeader;

	typedef struct
	{
		stHeader header;
		char contenido[LONGITUD_MAXIMA_DE_CONTENIDO];
	} __attribute__((packed)) stMensaje;

	static int sTimeout = 0;

	/*----------------------------------------------------------------------------*/
	/*                         Funciones Basicas                                  */
	/*----------------------------------------------------------------------------*/

	int conectar(char* unStringDeIP, unsigned unPort);

	/*----------------------------------------------------------------------------*/

	int escuchar(unsigned unPort);
	
	/*----------------------------------------------------------------------------*/
	
	int aceptar(int unEscuchador, struct sockaddr *unPtrASuSA);

	/*----------------------------------------------------------------------------*/

	int seleccionar(int fdMax, fd_set *ptrARead_fds, int segundos);

	/*----------------------------------------------------------------------------*/

	int enviarMensaje(int unSocket,stHeader unHeader, char* unContenido);

	int enviarMensajeA(int unSocket,stHeader unHeader, char* unContenido);

	/*----------------------------------------------------------------------------*/

	int recibirMensaje(int unSocket, stMensaje* unNuevoMensaje);

	/*----------------------------------------------------------------------------*/
	void cerrar(int unSocket,fd_set *fds_master);

	/*----------------------------------------------------------------------------*/

	int enviarTodo (int unSocket, void* unContenido, int totalAEnviar);


	/*----------------------------------------------------------------------------*/

	int enviarTodoMensaje(int unSocket, stHeader *unPtrAHeader, char *unContenido);

#endif /* SOCKETS_H_ */
