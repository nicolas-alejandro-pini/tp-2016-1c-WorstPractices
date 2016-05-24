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

	stHeaderIPC * unHeader = malloc(sizeof(stHeaderIPC));
	nuevoID(unHeader->id);
	unHeader->tipo = unTipo;
	unHeader->largo = 0;
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


/*----------------------------------------------------------------------------*/
/*                         Manejo de paquetes                                 */
/*----------------------------------------------------------------------------*/
int recibir_paquete(int sockfd, t_paquete *paquete){
	int32_t size, size_buffer;
	int32_t size_block = sizeof(t_buffer);
	t_header header;
	t_buffer *data;

	// recibo header
	if(recibir_header(sockfd, &header))
		return EXIT_FAILURE;

	// obtengo payload
	size_buffer = header.length * size_block;
	data = malloc(size_buffer);
	size = recv(sockfd, data, size_buffer, MSG_WAITALL);

	if(size == 0)  // CONNECTION CLOSED
	{
		header.type = CONNECTION_CLOSED;
		header.length = 0;
	}

	if(size == size_buffer)
	{
		// copio header y puntero data
		memcpy(&(paquete->header), &header, sizeof(t_header));
		paquete->data = (t_buffer*) data;
		return EXIT_SUCCESS;
	}

	perror("recv_payload en recibirPaquete");
	free(data);
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
	t_header header;

	memcpy(&header, &(paquete->header), sizeof(t_header));
	size_paquete = (header.length * sizeof(t_buffer)) + sizeof(t_header);
	while(enviados < header.length)
	{
		ret = send(sockfd, paquete->data, size_paquete , 0);
		if(ret < 0)
			return EXIT_FAILURE;
		enviados += ret;
	}
	return EXIT_SUCCESS;
}


t_buffer* serializar_header(t_paquete *paquete, int32_t *offset){
	t_buffer *data = paquete->data;
	t_header header_host;
	t_header header_net;
	int32_t offset_ = 0, tmp_size = 0;

	// copio header paquete
	memcpy(&(header_host), &(paquete->header), sizeof(t_header));
	header_net.type = htons(header_host.type);
	header_net.length = htonl(header_host.length);

	// copio header en buffer
	offset_ = 0;
	memcpy(data, &(header_net.type), tmp_size = sizeof(header_net.type));
	offset_ += (tmp_size / sizeof(t_buffer));
	memcpy(data + offset_, &(header_net.length), tmp_size = sizeof(header_net.length));
	offset_ += (tmp_size / sizeof(t_buffer));
	*offset = offset_;
	return data;
}

void deserializar_header(t_header *buf_header, int32_t *offset, t_header *header){
	t_header header_host;
	//memcpy(&header_net, &buf_header, sizeof(t_header));
	header_host.type = ntohs(buf_header->type);
	header_host.length = ntohl(buf_header->length);

	memcpy(&(header->type), &(header_host.type) , sizeof(header_host.type));
	memcpy(&(header->length), &(header_host.length), sizeof(header_host.length));
}

t_buffer* crear_paquete(t_paquete *paquete, t_htons type, int32_t size_buffer){
	t_header header;
	int32_t buffer_length_buffer = size_buffer / sizeof(t_buffer);

	// reservo memoria para el header + buffer
	t_buffer *pbuffer = malloc(sizeof(t_header) + size_buffer);
	paquete->data = pbuffer;

	// seteo header y lo copio en el paquete
	header.type = type;
	header.length = buffer_length_buffer;

	// Copio el header al paquete
	memcpy(&(paquete->header), &header, sizeof(t_header));
	return pbuffer;
}

int32_t* serializar_campo(t_buffer *data, int32_t *offset, void *campo, int32_t size){
	int32_t block = sizeof(t_buffer);
	uint16_t buffer_net, buffer_host;
	uint32_t buffer_net32, buffer_host32;

	if(size == sizeof(uint16_t))
	{
		memcpy(&buffer_host, campo, block);
		buffer_net = htons(buffer_host);
		memcpy(data + (*offset)*block, &buffer_net, block);
		*offset += 1;
	}
	// t_htonl
	memcpy(&buffer_host32, campo, block*2);
	buffer_net32 = htonl(buffer_host32);
	memcpy(data + (*offset)*block, &buffer_net32, block*2);
	*offset += 2;
	return offset;
}

int32_t* deserializar_campo(t_buffer *data, int32_t *offset, void *campo, int32_t size){
	int32_t block = sizeof(t_buffer);
	uint16_t buffer_net, buffer_host;
	uint32_t buffer_net32, buffer_host32;

	if(size == sizeof(uint16_t))
	{
		memcpy(&buffer_net, data + (*offset), block);
		buffer_host = ntohs(buffer_net);
		memcpy(campo, &buffer_host, block);
		*offset += 1;
	}
	memcpy(&buffer_net32, data + (*offset), block*2);
	buffer_host32 = ntohl(buffer_net32);
	memcpy(campo, &buffer_host32, block*2);
	*offset += 2;

	return offset;
}

void free_paquete(t_paquete *paquete){
	free(paquete->data);
}






/************* funciones especificas ********************/

int serializar_configUMC(t_paquete *paquete, t_UMCConfig *self){
	int32_t offset = 0;
	t_buffer *data;

	data = serializar_header(paquete, &offset);
	serializar_campo(data, &offset, &(self->paginasXProceso), sizeof(self->paginasXProceso));
	serializar_campo(data, &offset, &(self->tamanioPagina), sizeof(self->tamanioPagina));

	return offset;
}

int deserializar_configUMC(t_UMCConfig *self, t_paquete *paquete){
	int offset = 0;
	t_buffer *data = paquete->data;
	deserializar_campo(data, &offset, &(self->paginasXProceso), sizeof(self->paginasXProceso));
	deserializar_campo(data, &offset, &(self->tamanioPagina), sizeof(self->tamanioPagina));
	return EXIT_SUCCESS;
}



