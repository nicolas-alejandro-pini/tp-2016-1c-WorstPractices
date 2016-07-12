#include "test_umc_swap.h"

#include "../Memoria.h"
#include "../TLB.h"
#include "../Parametros.h"
#include "../UMC.h"

void agregar_tests_con_swap(){
	CU_pSuite suite_umc_swap = CU_add_suite("Pruebas UMC + SWAP:", NULL, NULL);
	CU_add_test(suite_umc_swap, "test conexion y envio de programa con swap", test_base_umc_swap);
}

void test_base_umc_swap(){
	stParametro losParametros;
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

	/* Me desconecto del Swap */
	close(losParametros.sockSwap);

	destruirMemoriaPrincipal();
}

