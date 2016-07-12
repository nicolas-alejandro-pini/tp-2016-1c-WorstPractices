#include "test_umc_swap.h"
#include <stdlib.h>
#include "../Memoria.h"
#include "../TLB.h"
#include "../Parametros.h"
#include "../UMC.h"

stParametro losParametros;
uint32_t gPidActivo; // En el thread de cada CPU

void agregar_tests_con_swap(){
	CU_pSuite suite_umc_swap = CU_add_suite("Pruebas UMC + SWAP:", inicializar_umc_swap, finalizar_umc_swap); // NULL, NULL);
	CU_add_test(suite_umc_swap, "test conexion y envio de programa con swap", test_base_umc_swap);
	CU_add_test(suite_umc_swap, "test_cambio_de_contexto()", test_cambio_de_contexto);
	CU_add_test(suite_umc_swap, "test_read_bytes_page()", test_read_bytes_page);

}

int inicializar_umc_swap(){

	loadInfo(&losParametros, "umc.conf");
	TablaMarcos = NULL;  // Lista global de tablas
	losParametros.entradasTLB = 0;  // SIN TLB  o tambien TLB = NULL;
	crearTLB(losParametros.entradasTLB);
	creatListaDeTablas(TablaMarcos); // TablaMarcos global
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
	int cant=5;
	char *programa[] = {"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend",
			"#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend"};

	/** Cargo 5 programas **/
	stPageIni unPageIni;

	for(i=0; i< cant; i++){
		unPageIni.processId = i+1;
		unPageIni.cantidadPaginas = calcular_cantidad_paginas(strlen(programa[i])+1,losParametros.frameSize);
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

	stMensajeIPC unMensaje;
	unMensaje.header.largo = sizeof(uint32_t);
	unMensaje.contenido = malloc(sizeof(uint32_t));
	*(unMensaje.contenido) = 1;

	gPidActivo = cambiarContexto(&unMensaje);

}

void test_read_bytes_page(){
	stPosicion posR;

	//READ_BTYES_PAGE:
	posR.pagina = 0;
	posR.offset = 10;
	posR.size = 7;

	if(1 == gPidActivo)
		leerBytes(&posR, gPidActivo, losParametros.sockSwap);

}

void test_write_bytes_page(){

	stEscrituraPagina posW;

    //WRITE_BYTES_PAGE:

	posW.nroPagina = 0;
	posW.offset = 0;
	posW.tamanio = 0;
	// (posW.buffer, posW.tamanio)
	//posW =(stEscrituraPagina*)(unMensaje.contenido);
	//escribirBytes(&posW, pidActivo, socket);

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

