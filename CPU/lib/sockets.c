/*----------------------------------------------------------------------------*/
/*                --------------------------------------------                */
/*                 BIBLIOTECA PARA MANEJO DE SOCKETS EN ANSI C                */
/*                --------------------------------------------                */
/*                                                                            */
/*  Nombre: sockets.c                                                         */
/*  Versión: 1.0                                                              */
/*  Fecha: 18 de Abril de 2016					              				  */
/*  Description: Implementación de la biblioteca sockets                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

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
/* Imprime el error en pantalla y termina la ejecución del programa. */
void error(char* unDescriptorDeError){
	printf("<<<ERROR:[%s]>>>\n",unDescriptorDeError);
	/*exit(EXIT_FAILURE);*/
}

/*----------------------------------------------------------------------------*/
/* Devuelve un nuevo socket del tipo STREAM. */
int nuevoSocket(){

	int unSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(unSocket == -1)
		error("No se pudo crear el socket!");
	return(unSocket);
}

/*----------------------------------------------------------------------------*/
/* Devuelve un puntero a un nuevo address del tipo sockaddr seteado. Si unStringDeIP es la cadena vacía, entonces el address tendrá la dirección de IP local. */
struct sockaddr *nuevoAddress(char* unStringDeIP, unsigned unPort){
	/*Creo una variable del tipo sockaddr_in y le reservo la memoria necesaria. */
	struct sockaddr_in *unPtrAAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
	/*Valido que unStringDeIP no esté vacío. Si lo está, cargo el IP propio, si no cargo unStringDeIP. */
	if(strcmp(unStringDeIP,"") == 0)
	{
		(unPtrAAddr->sin_addr).s_addr = INADDR_ANY;
	} else
		if( inet_aton(unStringDeIP, &(unPtrAAddr->sin_addr)) == 0)
			error("Address invalido!");
	/*Valido que el port no esté fuera de rango y lo cargo.*/
	if(unPort <= 1024 || unPort > 65535)
			error("Port fuera de rango!");
	unPtrAAddr->sin_port = htons(unPort);
	/*Cargo el resto de los datos.*/
	unPtrAAddr->sin_family = AF_INET;
	memset(&(unPtrAAddr->sin_zero), '\0',8);
	/*Casteo el sockaddr_in* a sockaddr* y lo devuelvo.*/
	return((struct sockaddr *) unPtrAAddr);
}

/*----------------------------------------------------------------------------*/
/*Devuelve una cadena que representa un numero aleatorio de 15 cifras.*/
char *nuevoID(){
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

/*----------------------------------------------------------------------------*/
/*Envía un string por el socket que recibe como parametro y devuelve la cantidad enviada.*/
int enviarContenido(int unSocket, char* unContenido){
	int resultado;
	if((resultado = send(unSocket, unContenido, strlen(unContenido)+1,0)) == -1)
		error("No se pudo enviar el contenido!");
	return(resultado);
}

/*Envía un string por el socket que recibe como parametro y devuelve la cantidad enviada.*/
int enviarContenidoA(int unSocket, char* unContenido,int unLargo){
	int resultado;
	if((resultado = send(unSocket, unContenido, unLargo,0)) == -1)
		error("No se pudo enviar el contenido!");
	return(resultado);
}
/*----------------------------------------------------------------------------*/
/*Devuelve la cantidad recibida atraves de unSocket y 0 si se cerró.*/
int recibirContenido(int unSocket, char* unNuevoContenido, int unLargo){
	int resultado;
	resultado = recv(unSocket, unNuevoContenido,unLargo,0);

	if(resultado == -1)
		return -1;
	return(resultado);
}

/*----------------------------------------------------------------------------*/
/*                         Funciones Basicas                                  */
/*----------------------------------------------------------------------------*/

/*Crea y devuelve un socket que escuche en la maquina local al puerto que recibe como parametro.*/
int escuchar(unsigned unPort){
	int yes = 1, unSocket = nuevoSocket();
	struct sockaddr *unPtrAAddr = nuevoAddress("",unPort);
	if(setsockopt(unSocket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
		error("No se pudo limpiar!");
	if(bind(unSocket, unPtrAAddr, sizeof(struct sockaddr)) == -1)
		error("No se pudo enlazar!");
	if(listen(unSocket,10) == -1)
		error("No se pudo escuchar!");
	return(unSocket);
}

/*----------------------------------------------------------------------------*/
/*Crea y devuelve un socket que se conecte en al IP y puerto que recibe como parametro.*/
int conectar(char* unStringDeIP, unsigned unPort){

	int unSocket = nuevoSocket();

	struct sockaddr *unPtrAAddr = nuevoAddress(unStringDeIP,unPort);
	if(connect(unSocket,unPtrAAddr,sizeof(struct sockaddr)) == -1){
			printf("No se pudo conectar!\n");
			return (-1);
		}
	return(unSocket);
}

/*----------------------------------------------------------------------------*/
/*Devuelve un socket, resultado de aceptar la conexión del socket que recibe como parámetro. En la estructura que recibe se guarda la dirección de la cual se aceptó la conexión.*/
int aceptar(int unEscuchador, struct sockaddr *unPtrASuSA){
	int aceptado, tamanio_SA = sizeof(struct sockaddr_in);
	aceptado = accept(unEscuchador,unPtrASuSA,&tamanio_SA);
	if(aceptado == -1 )
		error("No se pudo aceptar la conexion!");
	return(aceptado);
}

/*----------------------------------------------------------------------------*/
/*Automatizacion de la función select(). Devuelve el numero de sockets listos para escuchar o -1 si fue detenida por una llamada a sistema.*/
int seleccionar(int fdMax, fd_set *ptrARead_fds, int segundos){
	int resultado;
	struct timeval tv;
	tv.tv_sec = segundos;
	tv.tv_usec = 0;

	if (segundos == 0)
		resultado = select(fdMax+1,ptrARead_fds,NULL,NULL,NULL);
	else
		resultado = select(fdMax+1,ptrARead_fds,NULL,NULL,&tv);	
	if(resultado == -1)
		return(-1);
	return(resultado);
}

/*----------------------------------------------------------------------------*/
/*Cierra el socket y lo retira de la lista master.*/
void cerrar(int unSocket,fd_set *fds_master){
	FD_CLR(unSocket,fds_master);
	close(unSocket);
}

