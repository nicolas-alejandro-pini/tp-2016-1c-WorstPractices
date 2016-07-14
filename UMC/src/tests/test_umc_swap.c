#include "test_umc_swap.h"
#include <stdlib.h>
#include "../Memoria.h"
#include "../TLB.h"
#include "../Parametros.h"
#include "../UMC.h"
#include "../TablaMarcos.h"

stParametro losParametros;
uint32_t gPidActivo; // En el thread de cada CPU

void agregar_tests_con_swap(){
	CU_pSuite suite_umc_swap = CU_add_suite("Pruebas UMC + SWAP:", inicializar_umc_swap, finalizar_umc_swap); // NULL, NULL);
	CU_add_test(suite_umc_swap, "test conexion y envio de programa con swap", test_base_umc_swap);
	CU_add_test(suite_umc_swap, "test_cambio_de_contexto()", test_cambio_de_contexto);
	//CU_add_test(suite_umc_swap, "test_read_bytes_page()", test_read_bytes_page);
	//CU_add_test(suite_umc_swap, "test_write_bytes_page()", test_write_bytes_page);
	CU_add_test(suite_umc_swap, "test_write_read_guarda_en_swap()", test_write_read_guarda_en_swap);
}

int inicializar_umc_swap(){

	loadInfo(&losParametros, "umc.conf");
	losParametros.entradasTLB = 0;  // SIN TLB  o tambien TLB = NULL;
	crearTLB(losParametros.entradasTLB);
	creatListaDeTablas(); // TablaMarcos global
	// Memoria principal en memoria.h
	memoriaPrincipal = inicializarMemoriaPrincipal(losParametros.frameSize, losParametros.frames);

	losParametros.sockSwap = conectar(losParametros.ipSwap, losParametros.puertoSwap);
	/* Inicio el handShake con el servidor */
	if (losParametros.sockSwap != -1){
		if (swapHandShake(losParametros.sockSwap, "SOYUMC", SOYUMC) == -1)
		{
			log_info("CONNECTION_ERROR - No se recibe un mensaje correcto en Handshake con Swap");
			fflush(stdout);
			log_info("SOCKET_ERROR - No se recibe un mensaje correcto");
			close(losParametros.sockSwap);
		}
		else
		{
			FD_SET(losParametros.sockSwap, &(fds_master));
			log_info("OK - Swap conectado.");
			fflush(stdout);

		}
	}

	return EXIT_SUCCESS;
}

int finalizar_umc_swap(){
	/* Me desconecto del Swap */
	close(losParametros.sockSwap);
	loadInfo_destruir(&losParametros);
	destruirMemoriaPrincipal();

	return EXIT_SUCCESS;
}

void test_base_umc_swap(){
	stHeaderIPC *unHeader;
	int i=0;
	int cant=1;
	char *programa[] = {"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend"};

	/** Cargo 5 programas **/
	stPageIni unPageIni;

	for(i=0; i< cant; i++){
		unPageIni.processId = i+1;
		unPageIni.cantidadPaginas = calcular_cantidad_paginas(strlen(programa[i]),losParametros.frameSize);
		unPageIni.programa = programa[i];

		crearTabla(unPageIni.processId, unPageIni.cantidadPaginas);

		if(inicializarSwap(&unPageIni) == EXIT_FAILURE){
			log_error("No se pudo enviar el codigo a Swap");
			unHeader=nuevoHeaderIPC(ERROR);
			enviarHeaderIPC(losParametros.sockSwap, unHeader);
			CU_ASSERT_TRUE(false);
		}
		CU_ASSERT_TRUE(true);
	}
}

void test_cambio_de_contexto(){

	uint32_t pid = 1;
	stMensajeIPC unMensaje;
	unMensaje.header.largo = sizeof(uint32_t);
	unMensaje.contenido = malloc(sizeof(uint32_t));
	memcpy(unMensaje.contenido, &pid, sizeof(uint32_t));

	gPidActivo = cambiarContexto(&unMensaje);

	free(unMensaje.contenido);

}

void test_read_bytes_page(){
	stPosicion posR;
	char *buffer = NULL;

	/* Tamaño pagina 20
	 *
	 #!/usr/bin/ansisop\
	 nbegin\n#primero de
	 claro las variables
	 \nvariables a, b\na
 	 = 20\nprint a\nend
	*/

	imprimirMemoriaPrincipal();

	//READ_BTYES_PAGE:
	posR.pagina = 0;
	posR.offset = 2;
	posR.size = 4;
	// /usr

	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	// /bin
	posR.pagina = 1;
	posR.offset = 6;
	posR.size = 4;

	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*)&buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	// /ansisop
	posR.pagina = 2;
	posR.offset = 4;
	posR.size = 5;
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*)&buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	// /ansisop ==> obtener "begin" con page fault
	posR.pagina = 3;
	posR.offset = 3;
	posR.size = 5;
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*)&buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	// /ansisop ==> obtener "print" con page fault y reemplazo
	posR.pagina = 4;
	posR.offset = 0;
	posR.size = 1;
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*)&buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	// /ansisop
	posR.pagina = 1;
	posR.offset = 1;
	posR.size = 5;
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*)&buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}


}

void test_write_bytes_page(){

	/* Tamaño pagina 20
	 *
	 #!/usr/bin/ansisop\
	 nbegin\n#primero de
	 claro las variables
	 \nvariables a, b\na
 	 = 20\nprint a\nend
	*/
	//WRITE_BYTES_PAGE:

	imprimirMemoriaPrincipal();

	stEscrituraPagina posW;
	posW.nroPagina = 0;
	posW.offset = 0;
	posW.tamanio = 4;


	if(1 == gPidActivo){
		reservarPosicion((void*)&posW.buffer, posW.tamanio + 1);

		if(escribirBytes(&posW, gPidActivo))
			log_info("Error al escribir bytes");

		log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
		limpiarEscrituraPagina(posW.buffer, &posW);

		// imprimo memoria
		imprimirMemoriaPrincipal();
	}

}


//PUERTO=50002
//IP_SWAP=0
//PUERTO_SWAP=6000
//MARCOS=5
//MARCOS_SIZE=20
//MARCOS_X_PROC=2
//ENTRADAS_TLB=5
//RETARDO=1
//ALGORITMO=CLOCK

void test_write_read_guarda_en_swap(){

	stEscrituraPagina posW;
	stPosicion posR;
	char *buffer = NULL;

	imprimirMemoriaPrincipal();


	//WRITE_BYTES_PAGE:
	posW.nroPagina = 0;
	posW.offset = 0;
	posW.tamanio = 4;

	if(1 == gPidActivo){
		reservarPosicion((void*)&posW.buffer, posW.tamanio + 1);

		if(escribirBytes(&posW, gPidActivo))
			log_info("Error al escribir bytes");

		log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
		limpiarEscrituraPagina(posW.buffer, &posW);
	}

	imprimirMemoriaPrincipal();


	//READ_BTYES_PAGE:
	posR.pagina = 0;
	posR.offset = 0;
	posR.size = 6;

	// /usr
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	//READ_BTYES_PAGE:  otro page fault
	posR.pagina = 1;
	posR.offset = 0;
	posR.size = 6;

	// /usr
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	//READ_BTYES_PAGE:  otro page fault  , aca debe guardar en swap antes de bajarla
	posR.pagina = 2;
	posR.offset = 0;
	posR.size = 6;

	// /usr
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();

	posR.pagina = 0;    // otro page fault , traigo de vuelta la 0 a ver si grabo
	posR.offset = 0;
	posR.size = 6;

	// /usr
	if(1 == gPidActivo){
		reservarPosicion((void*)&buffer, posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);
	}

	imprimirMemoriaPrincipal();
}


void test_fin_programa(){
    //FINPROGRAMA:

	//finalizarPrograma(pidActivo, socket);
}

/** DEL NUCLEO **/
int calcular_cantidad_paginas(int size_programa,int tamanio_paginas){
	int cant=0;
	if(size_programa%tamanio_paginas > 0)
		cant++;
	return ((int)(size_programa/tamanio_paginas) + cant);
}


