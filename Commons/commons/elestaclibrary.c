/*
 * elestaclibrary.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
#include "elestaclibrary.h"

int recibirConfigUMC(int unSocket, stUMCConfig *UMCConfig){
	int iRecv;
	int size_umcConfig = sizeof(stUMCConfig);
	iRecv = recv(unSocket, UMCConfig, sizeof(stUMCConfig), 0);
	if(iRecv < size_umcConfig)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int enviarConfigUMC(int unSocket, int frameSize, int frameByProc){
	int iSent;
	int size_umcConfig = sizeof(stUMCConfig);
	stUMCConfig UMCConfig;

	UMCConfig.paginasXProceso = frameByProc;
	UMCConfig.tamanioPagina = frameSize;

	iSent = send(unSocket, &UMCConfig, sizeof(stUMCConfig), 0);
	if(iSent < size_umcConfig)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
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




