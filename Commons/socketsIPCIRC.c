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

/*
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
*/

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
	char contenido[LONGITUD_MAXIMA_DE_CONTENIDO];/* } __attribute__((packed)) stMensajeIPC;*/
}stMensajeIPC;	

/*----------------------------------------------------------------------------*/
/*                         Funciones Privadas                                 */
/*----------------------------------------------------------------------------*/

stHeaderIPC nuevoHeaderIPC(const char unTipo)
/* Devuelve un nuevo header con los campos cargados. */
{
	int i;
	stHeaderIPC unHeader;
	char *unID = nuevoID();
	for(i = 0;i <16;i++) unHeader.id[i] = *(unID+i);
	unHeader.tipo = unTipo;
	unHeader.largo = 0;
	return (unHeader);
}

/*----------------------------------------------------------------------------*/

char *stringHeaderIPC(void* elHeader)
{
    stHeaderIPC *unPtrAHeader;
    int i = 1, j;
    char *unTipo, *unLargo, *unID, *unString = (char *) malloc (200);
/*    for(j=0; j<200; j++)
	unString[j]='\0'; */
	memset(unString,'\0',200);

    unPtrAHeader = (stHeaderIPC*) elHeader;
    unTipo = (char *) malloc(10);
    unLargo = (char *) malloc(10);

    unID = unPtrAHeader->id;
    sprintf(unTipo,"%03d",unPtrAHeader->tipo);
    sprintf(unLargo,"%03d",unPtrAHeader->largo);

    unString[0] = '[';
    while(*unID)
    {
        unString[i] = *unID++; i++;
    }
    unString[i++] = '|';
    j = 0;
    while(*unTipo)
    {
        unString[i] = *unTipo++; i++;
    }
    unString[i++] = '|';
    while(*unLargo)
    {
        unString[i] = *unLargo++; i++;
    }
    unString[i++] = ']';
    unString[i++] = '\n';    
    unString[i++] = '\0';
    return (unString);
}

/*----------------------------------------------------------------------------*/

stHeaderIPC HeaderStringIPC(const char *cadena)
{
	char temp[10];
	int i = 1,s= 0;
	stHeaderIPC unHeader;

	while (cadena[i] != '|')
	  {
	   unHeader.id[i-1] = cadena[i];
	   i++;
	  }

	i++;

	while (cadena[i] != '|')
	  {
	    temp[s] = (cadena[i]);
	    s++;
            i++;
	  }

	temp[s] = '\0';
	
	unHeader.tipo = atoi(temp);

	i++;

	s = 0;
	while (cadena[i] != ']')
	  {
	    temp[s] = (cadena[i]);
	    s++;
	    i++;
          }
	temp[s] = '\0';
	unHeader.largo = atoi(temp);
return unHeader;
}

/*----------------------------------------------------------------------------*/

int enviarHeaderIPC(int unSocket, const stHeaderIPC unHeader)
/*Envía unHeader por el socket que recibe como parametro y devuelve la cantidad enviada.*/
{
	int resultado;
	if((resultado = send(unSocket, &unHeader, sizeof(stHeaderIPC),0)) == -1)
		error("No se pudo enviar el header!");
	/*printf("MANDE:%d\n",unHeader.tipo);*/
	return(resultado);
}

/*----------------------------------------------------------------------------*/

int recibirHeaderIPC(int unSocket, stHeaderIPC* nuevoPtrAHeader)
/*Devuelve el tamaño del header recibido atraves de unSocket y 0 si se cerró.*/
{
	int resultado, tamanioHeader = sizeof(stHeaderIPC);
	resultado = recv(unSocket, nuevoPtrAHeader,tamanioHeader,0);

	if(resultado == -1)
		return -1;
	/*printf("recibi:%d\n",nuevoPtrAHeader->tipo);	*/
	return(resultado);
}

/*----------------------------------------------------------------------------*/
/*                         Funciones Basicas                                  */
/*----------------------------------------------------------------------------*/

int enviarMensajeIPCA(int unSocket,stHeaderIPC unHeader, char* unContenido)
/*Envía primero unHeader y luego unContenido por el socket que recibe como parametro. Devuelve 0 si hubieron errores y el tamaño del contenido enviado -sin contar el header- si no.*/
{
	if (!enviarHeaderIPC(unSocket, unHeader))
		return(0);

	return(enviarContenidoA(unSocket,unContenido,unHeader.largo));
}

int enviarMensajeIPC(int unSocket,stHeaderIPC unHeader, char* unContenido)
/*Envía primero unHeader y luego unContenido por el socket que recibe como parametro. Devuelve 0 si hubieron errores y el tamaño del contenido enviado -sin contar el header- si no.*/
{
	int unLargo = strlen(unContenido)+1;
	unHeader.largo = unLargo;
	if (!enviarHeaderIPC(unSocket, unHeader))
		return(0);
	/*printf("ENVIO HEADER:%d\n",unHeader.tipo);*/	
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
