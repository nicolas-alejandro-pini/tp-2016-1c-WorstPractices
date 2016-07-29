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
	//CU_add_test(suite_umc_swap, "test_write_read_guarda_en_swap()", test_write_read_guarda_en_swap);
	//CU_add_test(suite_umc_swap, "test_write_stack()", test_write_stack);
	//CU_add_test(suite_umc_swap, "test_diapositiva_clock()", test_diapositiva_clock);
	//CU_add_test(suite_umc_swap, "test_programa_base()", test_programa_base);
	CU_add_test(suite_umc_swap, "test_clock()", test_clock);
	CU_add_test(suite_umc_swap, "test_clock_modificado()", test_clock_modificado);
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

		// Al ultimo programa le agrega stack
		if(i==(cant-1)){
			unPageIni.cantidadPaginas += 2; // Tamanio stack
		}

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

	int pid = 1;

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
		buffer = malloc(posR.size + 1);
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
		buffer = malloc(posR.size + 1);
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
		buffer = malloc(posR.size + 1);
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
		buffer = malloc(posR.size + 1);
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
		buffer = malloc(posR.size + 1);
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
		buffer = malloc(posR.size + 1);
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
	posW.nroPagina = 2;
	posW.offset = 0;
	posW.tamanio = 4;
	posW.buffer = malloc(posW.tamanio);
	if(escribirBytes(&posW, gPidActivo))
		log_info("Error al escribir bytes");

	log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
	limpiarEscrituraPagina(posW.buffer, &posW);

	// imprimo memoria
	imprimirMemoriaPrincipal();


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

	buffer = malloc(posR.size + 1);
	if(escribirBytes(&posW, gPidActivo))
		log_info("Error al escribir bytes");

	log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
	limpiarEscrituraPagina(posW.buffer, &posW);


	imprimirMemoriaPrincipal();


	//READ_BTYES_PAGE:
	posR.pagina = 0;
	posR.offset = 0;
	posR.size = 6;

	// /usr
	buffer = malloc(posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);


	imprimirMemoriaPrincipal();

	//READ_BTYES_PAGE:  otro page fault
	posR.pagina = 1;
	posR.offset = 0;
	posR.size = 6;

	// /usr

	buffer = malloc(posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);


	imprimirMemoriaPrincipal();

	//READ_BTYES_PAGE:  otro page fault  , aca debe guardar en swap antes de bajarla
	posR.pagina = 2;
	posR.offset = 0;
	posR.size = 6;

	// /usr

		buffer = malloc(posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);


	imprimirMemoriaPrincipal();

	posR.pagina = 0;    // otro page fault , traigo de vuelta la 0 a ver si grabo
	posR.offset = 0;
	posR.size = 6;

	// /usr

	buffer = malloc(posR.size + 1);
		leerBytes((void*) &buffer, &posR, gPidActivo);
		buffer[posR.size]='\0';
		log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
		log_info("Valor[%s]", (char*) buffer);
		limpiarPosicion(buffer, &posR);


	imprimirMemoriaPrincipal();
}

void test_write_stack(){

}
void cargarTabla(stNodoListaTP *tablaPaginas, uint16_t pagina){

	uint16_t marco;
	stRegistroTP *reg;

	// page fault
	marco=obtenerMarcoLibre();
	agregarFrameATablaMarcos(marco, tablaPaginas, pagina);

	reg = obtenerRegistroTabladePaginas(tablaPaginas, pagina);
	reg->bitModificado=1;
	log_info("Pagina[%d] Marco[%d] cargados inicualmente en tabla", pagina, marco);
	log_info("puntero clock pagina [%d]", tablaPaginas->punteroClock);
	log_info("Flag de Second Chance [%d]", obtenerRegistroTabladePaginas(tablaPaginas, pagina)->bit2ndChance);

}
 void actualizarTabla(stNodoListaTP *tablaPaginas, stRegistroTP *victima, uint16_t pagina){
	stRegistroTP *registroPresencia = NULL;

	// Seteo los bits de la pagina del pageFault como entrante
	registroPresencia = obtenerRegistroTabladePaginas(tablaPaginas, pagina);
	registroPresencia->bit2ndChance = 1;
	registroPresencia->bitModificado = 0;
	registroPresencia->bitPresencia = 1;
	registroPresencia->marco = victima->marco;  // guardo la direccion del marco a usar

	// Seteo bits de la victima en 0
	victima->bit2ndChance = 0;
	victima->bitModificado = 0;
	victima->bitPresencia = 0;
	victima->marco = 0;  // ya lo guarde, ahora no tiene marco asignado

	log_info("puntero clock pagina [%d]", tablaPaginas->punteroClock);
	log_info("Pagina pedida [%d]", pagina);
	log_info("Marco usado [%d]", registroPresencia->marco);
	log_info("Flag de Second Chance [%d]", registroPresencia->bit2ndChance);

	return;
}
void test_clock(){
	stRegistroTP *victima; //, *registro;
	stNodoListaTP *tablaPaginas;
	uint16_t pagina;

	tablaPaginas = buscarPID(gPidActivo);

	//Cargo tabla con 3 marcos por proceso como max
	pagina = 0;
	cargarTabla(tablaPaginas, pagina);
	pagina = 1;
	cargarTabla(tablaPaginas, pagina);
	pagina = 2;
	cargarTabla(tablaPaginas, pagina);

	//verifico que la victima sea la esperada
	// paginas pedidas
	//		0	 1	 2	 3	 0	 1	 4	 0	 1	 2	 3	 4
	//	M1	01	01	00	31	31	30	41	41	40	40	40	41
	//	M2 		11	10	10	01	00	00	01	00	21	21	21
	//	M3 			20	20	20	10	10	10	10	10	31	31
	//		PF	PF	PF	PF	PF	PF	PF			PF	PF

	log_info("---------------------------------------------Evaluo reemplazo clock----------------------------------------------------");

	pagina = 3;
	victima = EjecutarClock(tablaPaginas,pagina);
	CU_ASSERT_EQUAL(victima->marco, 1);
	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	// modifico la tabla pero no la memoria
	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 0;
	victima = EjecutarClock(tablaPaginas,pagina);
	CU_ASSERT_EQUAL(victima->marco, 2); //-----------------> FAIL
	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	// modifico la tabla pero no la memoria
	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 1;
	victima = EjecutarClock(tablaPaginas,pagina);
	CU_ASSERT_EQUAL(victima->marco, 3); //-----------------> FAIL
	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	// modifico la tabla pero no la memoria
	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 4;
	victima = EjecutarClock(tablaPaginas,pagina);
	CU_ASSERT_EQUAL(victima->marco, 1); //-----------------> FAIL
	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	// modifico la tabla pero no la memoria
	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 0;// no hay page fault
//	victima = EjecutarClock(tablaPaginas,pagina);
//	CU_ASSERT_EQUAL(victima->marco, 2);
//	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
//	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
//	// modifico la tabla pero no la memoria
//	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 1;// no hay page fault
//	victima = EjecutarClock(tablaPaginas,pagina);
//	CU_ASSERT_EQUAL(victima->marco, );
//	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
//	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
//	// modifico la tabla pero no la memoria
//	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 2;
	victima = EjecutarClock(tablaPaginas,pagina);
	CU_ASSERT_EQUAL(victima->marco, 2); //-----------------> FAIL
	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	// modifico la tabla pero no la memoria
	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 3;
	victima = EjecutarClock(tablaPaginas,pagina);
	CU_ASSERT_EQUAL(victima->marco, 3); //-----------------> FAIL
	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	// modifico la tabla pero no la memoria
	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
	pagina = 4; // no hay page fault
//	victima = EjecutarClock(tablaPaginas,pagina);
//	CU_ASSERT_EQUAL(victima->marco, );
//	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
//	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
//	// modifico la tabla pero no la memoria
//	actualizarTabla(tablaPaginas, victima, pagina);
	log_info("----------------------------------------------------------------------------------------------------");
}
void test_clock_modificado(){
	stRegistroTP *victima; //, *registro;
		stNodoListaTP *tablaPaginas;
		uint16_t pagina;

		tablaPaginas = buscarPID(gPidActivo);

		//Cargo tabla con 3 marcos por proceso como max
		pagina = 0;
		cargarTabla(tablaPaginas, pagina);
		pagina = 1;
		cargarTabla(tablaPaginas, pagina);
		pagina = 2;
		cargarTabla(tablaPaginas, pagina);


		//verifico que la victima sea la esperada
		// paginas pedidas
		//		 0	 1	 2	 3	 0	 1	 4	 0	 1	 2	 3	 4
		//		PSM	PSM	PSM	PSM	PSM	PSM	PSM	PSM	PSM	PSM	PSM	PSM
		//	M1	010	010	010	310	31	30	41	41	40	40	40	41
		//	M2 		110	111	111	01	00	00	01	00	21	21	21
		//	M3 			210	210	20	10	10	10	10	10	31	31
		//		PF	PF	PF	PF	PF	PF	PF			PF	PF

		log_info("---------------------------------------------Evaluo reemplazo clock----------------------------------------------------");

		//		PSM
		// M1	010 -> 310
		// M2	111
		// M3	210
		pagina = 3;
		victima = EjecutarClockModificado(tablaPaginas,pagina);
		CU_ASSERT_EQUAL(victima->marco, 1);
		CU_ASSERT_EQUAL(victima->bit2ndChance, 1);
		CU_ASSERT_EQUAL(victima->bitModificado, 0);
		CU_ASSERT_EQUAL(victima->bitPresencia, 1);
		// modifico la tabla pero no la memoria
		actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		//		PSM
		// M1	310
		// M2	111
		// M3	210
		pagina = 0;
		victima = EjecutarClockModificado(tablaPaginas,pagina);
		CU_ASSERT_EQUAL(victima->marco, 2); //-----------------> FAIL
		CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
		CU_ASSERT_EQUAL(victima->bitPresencia, 1);
		// modifico la tabla pero no la memoria
		actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		pagina = 1;
		victima = EjecutarClockModificado(tablaPaginas,pagina);
		CU_ASSERT_EQUAL(victima->marco, 3); //-----------------> FAIL
		CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
		CU_ASSERT_EQUAL(victima->bitPresencia, 1);
		// modifico la tabla pero no la memoria
		actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		pagina = 4;
		victima = EjecutarClockModificado(tablaPaginas,pagina);
		CU_ASSERT_EQUAL(victima->marco, 1); //-----------------> FAIL
		CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
		CU_ASSERT_EQUAL(victima->bitPresencia, 1);
		// modifico la tabla pero no la memoria
		actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		pagina = 0;// no hay page fault
	//	victima = EjecutarClockModificado(tablaPaginas,pagina);
	//	CU_ASSERT_EQUAL(victima->marco, 2);
	//	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	//	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	//	// modifico la tabla pero no la memoria
	//	actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		pagina = 1;// no hay page fault
	//	victima = EjecutarClockModificado(tablaPaginas,pagina);
	//	CU_ASSERT_EQUAL(victima->marco, );
	//	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	//	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	//	// modifico la tabla pero no la memoria
	//	actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		pagina = 2;
		victima = EjecutarClockModificado(tablaPaginas,pagina);
		CU_ASSERT_EQUAL(victima->marco, 2); //-----------------> FAIL
		CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
		CU_ASSERT_EQUAL(victima->bitPresencia, 1);
		// modifico la tabla pero no la memoria
		actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		pagina = 3;
		victima = EjecutarClockModificado(tablaPaginas,pagina);
		CU_ASSERT_EQUAL(victima->marco, 3); //-----------------> FAIL
		CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
		CU_ASSERT_EQUAL(victima->bitPresencia, 1);
		// modifico la tabla pero no la memoria
		actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");
		pagina = 4; // no hay page fault
	//	victima = EjecutarClockModificado(tablaPaginas,pagina);
	//	CU_ASSERT_EQUAL(victima->marco, );
	//	CU_ASSERT_EQUAL(victima->bit2ndChance, 0);
	//	CU_ASSERT_EQUAL(victima->bitPresencia, 1);
	//	// modifico la tabla pero no la memoria
	//	actualizarTabla(tablaPaginas, victima, pagina);
		log_info("----------------------------------------------------------------------------------------------------");

}
void test_diapositiva_clock(){
	//PUERTO=50002
	//IP_SWAP=0
	//PUERTO_SWAP=6000
	//MARCOS=5
	//MARCOS_SIZE=20
	//MARCOS_X_PROC=3
	//ENTRADAS_TLB=5
	//RETARDO=1
	//ALGORITMO=CLOCK

	stPosicion posR;
	char *buffer = NULL;

	imprimirMemoriaPrincipal();

	posR.pagina = 0;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 1;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 2;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 3;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 0;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 1;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 4;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 0;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 1;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 2;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 3;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 4;    // page fault
	posR.offset = 0;
	posR.size = 4;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();
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
void test_programa_base(){
	/* Tamaño pagina 20
	 *
	 #!/usr/bin/ansisop\   pag0
	 nbegin\n#primero de   pag1
	 claro las variables   pag2
	 \nvariables a, b\na   pag3
 	 = 20\nprint a\nend    pag4
	*/
	stPosicion posR;
	stEscrituraPagina posW;
	char *buffer = NULL;

	imprimirMemoriaPrincipal();

	posR.pagina = 3;    // page fault
	posR.offset = 1;
	posR.size = 8;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	posR.pagina = 3;    // page fault
	posR.offset = 9;
	posR.size = 10;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	// Pide escribir en pagina stack
	imprimirMemoriaPrincipal();

	//WRITE_BYTES_PAGE:
	posW.nroPagina = 5;
	posW.offset = 0;
	posW.tamanio = 4;

	buffer = malloc(posR.size + 1);
	if(escribirBytes(&posW, gPidActivo))
		log_info("Error al escribir bytes");

	log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
	limpiarEscrituraPagina(posW.buffer, &posW);

	imprimirMemoriaPrincipal();

	posW.nroPagina = 5;
	posW.offset = 4;
	posW.tamanio = 4;

	posW.buffer = malloc(posW.tamanio + 1);
	if(escribirBytes(&posW, gPidActivo))
		log_info("Error al escribir bytes");

	log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
	limpiarEscrituraPagina(posW.buffer, &posW);

	// lee pagina 4 la siguiente instruccion
	imprimirMemoriaPrincipal();

	posR.pagina = 4;    // page fault
	posR.offset = 1;
	posR.size = 8;
	buffer = malloc(posR.size + 1);
	leerBytes((void*) &buffer, &posR, gPidActivo);
	buffer[posR.size]='\0';
	log_info("Pagina[%d] Offset[%d] Size[%d]", posR.pagina, posR.offset, posR.size);
	log_info("Valor[%s]", (char*) buffer);
	limpiarPosicion(buffer, &posR);

	imprimirMemoriaPrincipal();

	//WRITE_BYTES_PAGE:
	posW.nroPagina = 5;
	posW.offset = 8;
	posW.tamanio = 4;

	buffer = malloc(posR.size + 1);
	if(escribirBytes(&posW, gPidActivo))
		log_info("Error al escribir bytes");

	log_info("Pagina[%d] Offset[%d] Size[%d]", posW.nroPagina, posW.offset, posW.tamanio);
	limpiarEscrituraPagina(posW.buffer, &posW);
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


