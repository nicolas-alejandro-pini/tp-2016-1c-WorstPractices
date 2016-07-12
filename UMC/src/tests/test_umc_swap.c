#include "test_umc_swap.h"
#include <stdlib.h>
#include "../Memoria.h"
#include "../TLB.h"
#include "../Parametros.h"
#include "../UMC.h"

stParametro losParametros;

void agregar_tests_con_swap(){
	CU_pSuite suite_umc_swap = CU_add_suite("Pruebas UMC + SWAP:", inicializar_umc_swap, finalizar_umc_swap); // NULL, NULL);
	CU_add_test(suite_umc_swap, "test conexion y envio de programa con swap", test_base_umc_swap);
}

int inicializar_umc_swap(){

	loadInfo(&losParametros, "umc.conf");
	losParametros.entradasTLB = 0;  // SIN TLB
	TablaMarcos = NULL;  // Lista global de tablas
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

/** DEL NUCLEO **/
int calcular_cantidad_paginas(int size_programa,int tamanio_paginas){
	int cant=0;
	if(size_programa%tamanio_paginas > 0)
		cant++;
	return ((int)(size_programa/tamanio_paginas) + cant);
}

