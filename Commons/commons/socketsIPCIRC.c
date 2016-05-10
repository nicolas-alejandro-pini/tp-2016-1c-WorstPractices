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
#include <time.h>

/*----------------------------------------------------------------------------*/
/*                         Funciones Privadas                                 */
/*----------------------------------------------------------------------------*/

/*
 * Devuelve una cadena que representa un numero aleatorio de 15 cifras
 *
 * @return Devuelve una nueva cadena con el ID. Debe liberarse con free!
 */
char *nuevoID()
{
	int i;
	char strAux[11];
	char *unVector = (char*) malloc(16);
	srand(time(NULL));
	for(i=0;i<15;i++)
	{

		sprintf(strAux,"%d",rand());
		unVector[i] = *strAux;
	}
	unVector[i] = '\0';
	return(unVector);
}

stHeaderIPC nuevoHeaderIPC(const unsigned long int unTipo)
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
    int i = 1;
    char *unTipo, *unLargo, *unID, *unString = (char *) malloc (200);
/*    for(j=0; j<200; j++)
	unString[j]='\0'; */
	memset(unString,'\0',200);

    unPtrAHeader = (stHeaderIPC*) elHeader;
    unTipo = (char *) malloc(10);
    unLargo = (char *) malloc(10);

    unID = unPtrAHeader->id;
    sprintf(unTipo,"%03d",unPtrAHeader->tipo);
    sprintf(unLargo,"%03u",unPtrAHeader->largo);

    unString[0] = '[';
    while(*unID)
    {
        unString[i] = *unID++; i++;
    }
    unString[i++] = '|';
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
