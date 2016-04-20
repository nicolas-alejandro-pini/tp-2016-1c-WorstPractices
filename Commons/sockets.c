/*----------------------------------------------------------------------------*/
/*                --------------------------------------------                */
/*                 BIBLIOTECA PARA MANEJO DE SOCKETS EN ANSI C                */
/*                --------------------------------------------                */
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

static void AlarmHandler(int sig)
{
	sTimeout = 1;
}

/*----------------------------------------------------------------------------*/
/*                         Funciones Privadas                                 */
/*----------------------------------------------------------------------------*/

void error(char* unDescriptorDeError)
/* Imprime el error en pantalla y termina la ejecución del programa. */
{
	printf("<<<ERROR:[%s]>>>\n",unDescriptorDeError);
	/*exit(EXIT_FAILURE);*/
}

/*----------------------------------------------------------------------------*/

int nuevoSocket()
/* Devuelve un nuevo socket del tipo STREAM. */
{
	int unSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(unSocket == -1)
		error("No se pudo crear el socket!");
	return(unSocket);
}

/*----------------------------------------------------------------------------*/

struct sockaddr *nuevoAddress(char* unStringDeIP, unsigned unPort)

/* Devuelve un puntero a un nuevo address del tipo sockaddr seteado. Si unStringDeIP es la cadena vacía, entonces el address tendrá la dirección de IP local. */
{
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

char *stringAddress(const struct sockaddr *unSA)
/*Devuelve un string con el contenido del address.*/
{
	/*Casteo el sockaddr a sockaddr_in.*/
	struct sockaddr_in *unSA_in = (struct sockaddr_in *) unSA;
	/*Cargo los strings temporales.*/
	char *unStringAddress, elPort[6], *elIP = inet_ntoa(unSA_in->sin_addr);
	sprintf(elPort,"%d",ntohs(unSA_in->sin_port));
	/*Reservo la memoria para el stringAddress.*/
	unStringAddress = (char*) malloc(sizeof(elIP) + sizeof(elPort) + 10);
	/*Cargo los datos en el stringAddress.*/
	strcat(unStringAddress,"[");
	strcat(unStringAddress,elIP);
	strcat(unStringAddress,":");
	strcat(unStringAddress,elPort);
	strcat(unStringAddress,"]");
	/*Retorno el stringAddress.*/
	return(unStringAddress);
}

/*----------------------------------------------------------------------------*/

char *nuevoID()
/*Devuelve una cadena que representa un numero aleatorio de 15 cifras.*/
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

/*----------------------------------------------------------------------------*/

stHeader nuevoHeader(const char unTipo, const char unTTL)
/* Devuelve un nuevo header con los campos cargados. Los campos "hops" y "largo" se setean en 0.*/
{
	int i;
	stHeader unHeader;
	char *unID = nuevoID();
	for(i = 0;i <16;i++) unHeader.id[i] = *(unID+i);
	unHeader.tipo = unTipo;
	unHeader.ttl = unTTL;
	unHeader.hops = 0;
	unHeader.largo = 0;
	return (unHeader);
}

/*----------------------------------------------------------------------------*/

char *stringHeader(void* elHeader)
{
    stHeader *unPtrAHeader;
    int i = 1, j;
    char *unTipo, *unTTL, *unHops, *unLargo, *unID, *unString = (char *) malloc (200);
/*    for(j=0; j<200; j++)
	unString[j]='\0'; */
	memset(unString,'\0',200);

    unPtrAHeader = (stHeader*) elHeader;
    unTipo = (char *) malloc(10);
    unTTL  = (char *) malloc(10);
    unHops = (char *) malloc(10);
    unLargo = (char *) malloc(10);

    unID = unPtrAHeader->id;
    sprintf(unTipo,"%03d",unPtrAHeader->tipo);
    sprintf(unTTL,"%03d",unPtrAHeader->ttl);
    sprintf(unHops,"%03d",unPtrAHeader->hops);
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
    while(*unTTL)
    {
        unString[i] = *unTTL++; i++;
    }
    unString[i++] = '|';
    while(*unHops)
    {
        unString[i] = *unHops++; i++;
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

stHeader HeaderString(const char *cadena)
{
	char temp[10];
	int i = 1,s= 0;
	stHeader unHeader;

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

	while (cadena[i] != '|')
	  {
	   unHeader.ttl = (cadena[i]);
	   i++;
	  }

	i++;

	while (cadena[i] != '|')
	  {
	    unHeader.hops = (cadena[i]);
	    i++;
	  }

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

int enviarHeader(int unSocket, const stHeader unHeader)
/*Envía unHeader por el socket que recibe como parametro y devuelve la cantidad enviada.*/
{
	int resultado;
	if((resultado = send(unSocket, &unHeader, sizeof(stHeader),0)) == -1)
		error("No se pudo enviar el header!");
	return(resultado);
}

/*----------------------------------------------------------------------------*/

int recibirHeader(int unSocket, stHeader* nuevoPtrAHeader)
/*Devuelve el tamaño del header recibido atraves de unSocket y 0 si se cerró.*/
{
	int resultado, tamanioHeader = sizeof(stHeader);
	resultado = recv(unSocket, nuevoPtrAHeader,tamanioHeader,0);
	if(resultado == -1)
		return -1;
	return(resultado);
}

/*----------------------------------------------------------------------------*/

int enviarContenido(int unSocket, char* unContenido)
/*Envía un string por el socket que recibe como parametro y devuelve la cantidad enviada.*/
{
	int resultado;
	if((resultado = send(unSocket, unContenido, strlen(unContenido)+1,0)) == -1)
		error("No se pudo enviar el contenido!");
	return(resultado);
}
int enviarContenidoA(int unSocket, char* unContenido,int unLargo)
/*Envía un string por el socket que recibe como parametro y devuelve la cantidad enviada.*/
{

	int resultado;
	if((resultado = send(unSocket, unContenido, unLargo,0)) == -1)
		error("No se pudo enviar el contenido!");
	return(resultado);
}
/*----------------------------------------------------------------------------*/

int recibirContenido(int unSocket, char* unNuevoContenido, int unLargo)
/*Devuelve la cantidad recibida atraves de unSocket y 0 si se cerró.*/
{
	int resultado;
	resultado = recv(unSocket, unNuevoContenido,unLargo,0);

	if(resultado == -1)
		return -1;
	return(resultado);
}

/*----------------------------------------------------------------------------*/
/*                         Funciones Basicas                                  */
/*----------------------------------------------------------------------------*/

int escuchar(unsigned unPort)
/*Crea y devuelve un socket que escuche en la maquina local al puerto que recibe como parametro.*/
{
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

int conectar(char* unStringDeIP, unsigned unPort)
/*Crea y devuelve un socket que se conecte en al IP y puerto que recibe como parametro.*/
{

	int unSocket = nuevoSocket();

	struct sockaddr *unPtrAAddr = nuevoAddress(unStringDeIP,unPort);
	if(connect(unSocket,unPtrAAddr,sizeof(struct sockaddr)) == -1){
			printf("No se pudo conectar!\n");
			return (-1);
		}
	return(unSocket);
}

/*----------------------------------------------------------------------------*/

int aceptar(int unEscuchador, struct sockaddr *unPtrASuSA)
/*Devuelve un socket, resultado de aceptar la conexión del socket que recibe como parámetro. En la estructura que recibe se guarda la dirección de la cual se aceptó la conexión.*/
{
	int aceptado, tamanio_SA = sizeof(struct sockaddr_in);
	aceptado = accept(unEscuchador,unPtrASuSA,&tamanio_SA);
	if(aceptado == -1 )
		error("No se pudo aceptar la conexion!");
	return(aceptado);
}

/*----------------------------------------------------------------------------*/

int seleccionar(int fdMax, fd_set *ptrARead_fds, int segundos)
/*Automatizacion de la función select(). Devuelve el numero de sockets listos para escuchar o -1 si fue detenida por una llamada a sistema.*/
{
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

int enviarMensaje(int unSocket,stHeader unHeader, char* unContenido)
/*Envía primero unHeader y luego unContenido por el socket que recibe como parametro. Devuelve 0 si hubieron errores y el tamaño del contenido enviado -sin contar el header- si no.*/
{
	int unLargo = strlen(unContenido)+1;
	unHeader.largo = unLargo;
	if (!enviarHeader(unSocket, unHeader))
		return(0);
	return(enviarContenido(unSocket,unContenido));
}

int enviarMensajeA(int unSocket,stHeader unHeader, char* unContenido)
/*Envía primero unHeader y luego unContenido por el socket que recibe como parametro. Devuelve 0 si hubieron errores y el tamaño del contenido enviado -sin contar el header- si no.*/
{
	if (!enviarHeader(unSocket, unHeader))
		return(0);

	return(enviarContenidoA(unSocket,unContenido,unHeader.largo));
}

/*----------------------------------------------------------------------------*/

int recibirMensaje(int unSocket, stMensaje* unNuevoMensaje)
/*Devuelve la cantidad recibida -sin contar el header- si se pudo recibir un mensaje atraves de unSocket y 0 si este se cerro.*/
{
	if(!(recibirHeader(unSocket, &(unNuevoMensaje->header))))
		return(0);

	/*printf("Recibiendo contenido\n");*/

	if (unNuevoMensaje->header.largo > 0 )
		return(recibirContenido(unSocket,unNuevoMensaje->contenido,unNuevoMensaje->header.largo));
	else {
	
		return 1;
	}
}

/*----------------------------------------------------------------------------*/
void cerrar(int unSocket,fd_set *fds_master)
/*Cierra el socket y lo retira de la lista master.*/
{
	FD_CLR(unSocket,fds_master);
	close(unSocket);
}

/*----------------------------------------------------------------------------*/

int enviarTodo (int unSocket, void* unContenido, int totalAEnviar)
/*Devuelve la cantidad enviada; 0 si el socket se cerró y -1 si hubo errores por señal.*/
{
	int bytes, enviadoHastaAhora = 0;
	while(enviadoHastaAhora < totalAEnviar)
	{
		bytes = send(unSocket, unContenido+enviadoHastaAhora,totalAEnviar,0);
		if(bytes < 0)
			return(-1);
		if(bytes == 0)
			return(0);
		enviadoHastaAhora+=bytes;
	}
	return(enviadoHastaAhora);
}


/*----------------------------------------------------------------------------*/

int enviarTodoMensaje(int unSocket, stHeader *unPtrAHeader, char *unContenido)
{
	int bytes;
	if((bytes = enviarTodo(unSocket,unPtrAHeader,sizeof(stHeader))) < 0)
	{
		if(bytes == 0)
			return(0);
		return(-1);
	}else
	{
		return(enviarTodo(unSocket,unContenido,strlen(unContenido)+1));
	}
}
