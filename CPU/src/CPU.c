/*
 ============================================================================
 Name        : CPU.c
 Author      : Ezequiel Martinez
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int cpuDameSocket (void){
	//EML: Las IP y Puerto recibirlo por parametro.
	struct sockaddr_in direccionServidor;
		direccionServidor.sin_family = AF_INET;
		direccionServidor.sin_addr.s_addr = inet_addr("127.0.0.1");
		direccionServidor.sin_port = htons(8081); //Le indico el puerto en el que escucha.

		int cliente = socket(AF_INET, SOCK_STREAM, 0);

		if(connect(cliente, (void*)&direccionServidor, sizeof(direccionServidor))!=0)
			return 0;//El connect si devuelve 0 es por un error.
		else
			return cliente;
}

int cpuConectarseAlNucleo(void){

	int idNucleo = 0;
	puts("Conectando al nucleo...\n");
	idNucleo = cpuDameSocket();
	return idNucleo;
}

int cpuConectarseAlUmc(void){

	int idUMC = 0;
	puts("Conectando al UMC...\n");
	idUMC = cpuDameSocket();
	return idUMC;
}

int main(void) {
	int nucleo = 0;
	//int umc = 0;
	puts("CPU Application"); /* prints CPU Application */

	nucleo = cpuConectarseAlNucleo(); //EML: Pendiente- Leer un .conf y pasar ip y puerto.
	//umc = cpuConectarseAlUmc();

	if (nucleo != 0){
		puts("Conectado con el nucleo.");
		while(1){
			char mensaje[1000];
			scanf("%s", mensaje);
			send (nucleo, mensaje, strlen(mensaje), 0); //Pruebo comunicacion con un server.
		}
	}else
		perror("Error al conectarme con el nucleo.");

	return EXIT_SUCCESS;
}




