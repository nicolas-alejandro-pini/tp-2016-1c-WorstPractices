/*
 * Pagina.c
 *
 *  Created on: 4/5/2016
 *      Author: utnso
 */

#include "Marco.h"

void *inicializarPrograma(stIni* ini){

	/* TODO inicializarSwap */
	inicializarSwap(ini->sPI);
	/* TODO guardarEnTabla */
	guardarEnTabla(ini->sPI->cantidadPaginas);

	pthread_exit(NULL);
}
void *leerBytes(stRead* unaLectura){
	/* Si NO esta la pagina disponible */
	if (estaPaginaDisponible(unaLectura->sPos->pagina)!=0){
		//pid = elegirReemplazo();
		cambiarContexto(unaLectura->sPos->pagina);
	}
	/*reservo memoria para devolver buffer leido*/
	void *leido = calloc(unaLectura->sPos->size,1);
	leido = leerMemoria(unaLectura->sPos->pagina,unaLectura->sPos->offset, unaLectura->sPos->size);

	/* TODO devolver por socket */

	pthread_exit(NULL);
}
void *escribirBytes(stWrite* unaEscritura){
	/* TODO escribirBytes */
	return 0;
}
void *finalizarPrograma(stEnd *fin){
	/* TODO finalizarPrograma */
	pthread_exit(NULL);
}
int cambiarContexto(uint16_t pagina){
	/* TODO cambiar contexto */

	return 0;
}
int elegirReemplazo(int cantidad){
	return 0;
}
int hayMarcoslibres(int cantidad){
	return 0;
}
int estaPaginaDisponible(uint16_t pagina){
	return 0;
}

int guardarEnTabla(uint16_t cantidadPaginas){
	return 0;
}
void realizarAccionUMC(unsigned int tipo, char* contenido, uint16_t socket, pthread_attr_t attr){

	pthread_t tid;

	stIni *ini;
	stWrite *wr;
	stRead *read;
	stEnd *end;

	switch(tipo){

		/* TODO incluir pedidos de la consola UMC */
		case INICIALIZAR_PROGRAMA:

			ini = (stIni*)calloc(1,sizeof(stIni));
			ini->socketResp = socket;
			ini->sPI= (stPageIni*)contenido;

			pthread_create(&tid,&attr,(void*)inicializarPrograma,ini);

			break;

		case READ_BTYES_PAGE:

			read = (stRead*)calloc(1,sizeof(stRead));
			read->socketResp = socket;
			read->sPos = (stPosicion*)contenido;

			pthread_create(&tid,&attr,(void*)leerBytes,read);

			break;

		case WRITE_BYTES_PAGE:

			wr = (stWrite*)calloc(1,sizeof(stWrite));
			wr->socketResp = socket;
			wr->sEP = (stEscrituraPagina*)contenido;

			pthread_create(&tid,&attr,(void*)escribirBytes,wr);

			break;

		case FINPROGRAMA:

			end = calloc(1,sizeof(stEnd));
			end->socketResp = socket;
			end->pid = atoi(contenido);

			pthread_create(&tid,&attr,(void*)finalizarPrograma,end);
			break;

		default:
			printf("\nSe recibio una peticion con un codigo desconocido...%i\n", tipo);
			/*enviarMensajeIPC(unSocket,nuevoHeaderIPC(OK),"UMC: Solicitud recibida.");*/
			/*enviarMensajeIPC(elEstadoActual.sockSwap,nuevoHeaderIPC(OK),"UMC: Confirmar recepcion.");*/
			break;

	}
	/*Cierro switch(unMensaje.header.tipo)*/
}
