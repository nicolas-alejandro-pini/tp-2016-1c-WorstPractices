/*
 * elestaclibrary.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include "elestaclibrary.h"

//ToDo: Implementar con la libreria socketsIPCIRC serializada

int recibirConfigUMC(int unSocket, stUMCConfig *UMCConfig){
	uint16_t bytes_recv_length, bytes_recv_data, *length;
	t_stream stream;
	void *buffer = malloc(sizeof(stream.length));

	bytes_recv_length = recv(unSocket, buffer, sizeof(stream.length), 0);
	if(bytes_recv_length < 0) {
		perror("Error recibirConfigUMC - recv length");
		free(buffer);
		return -1;
	}
	if(bytes_recv_length == 0){
		perror("Error recibirConfigUMC - cliente cerro la conexion");
		free(buffer);
		return -1;
	}
	length = (uint16_t*) buffer;
	stream.length = length[0];

	buffer = realloc(buffer, sizeof(stream.length) + stream.length + 1);

	bytes_recv_data = recv(unSocket, buffer, stream.length, MSG_WAITALL);
	if( bytes_recv_data < 0){
		perror("Error recibirConfigUMC - recv data");
		free(buffer);
		return -1;
	}
	if(bytes_recv_data == 0){
		perror("Error recibirConfigUMC - cliente cerro la conexion");
		free(buffer);
		return -1;
	}
	UMCConfig = deserializarConfigUMC(&stream);
	return 0;
}

int enviarConfigUMC(int unSocket, int frameSize, int frameByProc){
	stUMCConfig UMCConfig;
	UMCConfig.paginasXProceso = frameByProc;
	UMCConfig.tamanioPagina = frameSize;

	t_stream *stream = serializarConfigUMC(&UMCConfig);
	int ret = -1;
	ret = send(unSocket, stream->data, stream->length, 0);
	if(ret == -1){
		perror("Error al enviar la config UMC");
		free(stream);
		return -1;
	}
	printf("Configuracion UMC enviada\n");
	return 0;
}

t_stream* serializarConfigUMC(stUMCConfig *self){
	t_data *data = malloc( sizeof(uint16_t) + sizeof(uint16_t));
	t_stream *stream = malloc( sizeof(t_stream));
	int offset = 0, tmp_size = 0;

	memcpy(data, &self->paginasXProceso, tmp_size = sizeof(uint16_t));
	offset = tmp_size;
	memcpy(data + offset, &self->tamanioPagina, tmp_size = sizeof(uint16_t));

	stream->length = offset + tmp_size;
	stream->data = data;

	return stream;
}

stUMCConfig* deserializarConfigUMC(t_stream *stream){
	stUMCConfig *self = malloc( sizeof(stUMCConfig));
	int offset = 0, tmp_size = 0;

	memcpy(&self->paginasXProceso, stream->data, tmp_size = sizeof(uint16_t));
	offset = tmp_size;
	memcpy(&self->tamanioPagina, stream->data + offset, tmp_size = sizeof(uint16_t));
	offset += tmp_size;  // debe coincidir con el total de bytes enviados.

	return self;
}




