/*
 * test_tlb.c
 *
 *  Created on: 17/6/2016
 *      Author: Nicolas Pini
 */
#include "test_umc.h"

#include "../Memoria.h"
#include "../TLB.h"
#include "../Parametros.h"
#include "../UMC.h"

int test_unit_umc() {

	CU_initialize_registry();
	agregar_tests();
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}

// Agrego los test al suite de pruebas del UMC
void agregar_tests(){
	CU_pSuite suite_umc = CU_add_suite("Pruebas UMC:", NULL, NULL);
	CU_add_test(suite_umc, "test creacion tlb", test_creacion_tlb);
	CU_add_test(suite_umc, "test reemplazo tlb", test_reemplazo_tlb);
	CU_add_test(suite_umc, "test busqueda tlb", test_busqueda_tlb);
	CU_add_test(suite_umc, "test estructras memoria", test_estructuras_memoria);
}

// Setea el entorno del suite para pruebas
int inicializar_umc(){
	int i;
	stParametro losParametros;
	loadInfo(&losParametros, "umc.conf");
	//crearTLB(TLB, losParametros.entradasTLB);
	//imprimirTLB(TLB);

	for( i=0; i<CANT_CPU; i++)
		if( pthread_create(&tests_threads[i], NULL, (void*)&thread_cpu, &i) ){
			perror("Ocurrio un error durante la creacion del Thread.");
		}

	return EXIT_SUCCESS;
}

// Finaliza el suite de pruebas de la UMC
int finalizar_umc(){
	int i;
	for( i=0; i<CANT_CPU; i++)
		pthread_join(tests_threads[i], NULL);

	destruirTLB(TLB);
	return EXIT_SUCCESS;
}

// CASOS DE PRUEBA

// Prueba 1
void test_creacion_tlb(){
	stParametro losParametros;
	loadInfo(&losParametros, "umc.conf");
	crearTLB(losParametros.entradasTLB);
	imprimirTLB();
	CU_ASSERT_EQUAL(cantidadRegistrosTLB(), losParametros.entradasTLB);
	destruirTLB();
}

// Prueba 2
void test_reemplazo_tlb(){
	int i;
	stRegistroTLB reg;
	stParametro losParametros;
	loadInfo(&losParametros, "umc.conf");
	crearTLB(5); //losParametros.entradasTLB);

	for(i=1 ; i<TEST_REEMPLAZO_X_PROC+1; i++){
		reg.pid = 1;
		reg.pagina = i;
		reg.marco = i+100;
		reemplazarValorTLB(reg);
		imprimirTLB();
		sleep(1);
	}
	destruirTLB(TLB);
}

// Prueba 3
void test_busqueda_tlb(){
	int i;
	uint16_t *frame;
	stRegistroTLB reg;
	stParametro losParametros;
	loadInfo(&losParametros, "umc.conf");
	crearTLB(5); //losParametros.entradasTLB);

	// cargo tabla
	for(i=1 ; i<TEST_REEMPLAZO_X_PROC+1; i++){
		reg.pid = 1;
		reg.pagina = i;
		reg.marco = i+100;
		reemplazarValorTLB(reg);
	}
	imprimirTLB();
	//Inicial
	//Pid | Pagina | Marco | LastUsed
	//[1][6][106][4]
	//[1][7][107][3]
	//[1][8][108][2]
	//[1][9][109][1]
	//[1][10][110][0]
	buscarEnTLB(1, 6, &frame);
	printf("pid:1, pagina:6, frame:[%d]\n", *frame);
	buscarEnTLB(1, 7, &frame);
	printf("pid:1, pagina:7, frame:[%d]\n", *frame);
	buscarEnTLB(1, 8, &frame);
	printf("pid:1, pagina:8, frame:[%d]\n", *frame);
	buscarEnTLB(1, 9, &frame);
	printf("pid:1, pagina:9, frame:[%d]\n", *frame);
	//Pid | Pagina | Marco | LastUsed
	//[1][6][106][4]
	//[1][7][107][3]
	//[1][8][108][2]
	//[1][9][109][1]
	//[1][10][110][4]
	imprimirTLB();
	destruirTLB(TLB);
}

// Prueba X
void test_estructuras_memoria(){
	int i;
	void *memoria;
	int pagina = 20;
	char string[pagina]; // en mi pagina guardo strings por ahora
	int cant_paginas = 10;

	// Inicializo Memoria Disponible
	memoria = inicializarMemoriaDisponible(pagina, cant_paginas);

	// Verifico que pueda escribir en todas las paginas
	for(i=0; i<cant_paginas; i++)
	{
		//escribirMemoria(i, 0, 10, memoria[i]);
		sprintf(string,"Soy pagina [%d]", i);
		strcpy((void*)memoria + (pagina*i), string);
	}

	// Pueda leer en todas las paginas
	for(i=0; i<cant_paginas; i++)
	{
		//leerMemoria(memoria[i], 0, 10);
		sprintf(string,"Soy pagina [%d]", i);
		CU_ASSERT_STRING_EQUAL((char*)memoria + (pagina*i), &string);
	}
	fflush(stdout);

}

/// AUXILIARES
void thread_cpu(void *arg){
	int i;
	stRegistroTLB reg;
	int *pid = (int*) arg;

	for(i=0 ; i<TEST_REEMPLAZO_X_PROC; i++){
		reg.pid = *pid;
		reg.pagina = i;
		reg.marco = i+100;
		//reemplazarValorTLB(reg);
	}
	printf("Thread ID: [%u] | [%d]\n", (unsigned int)pthread_self(), *pid);
	//imprimirTLB(TLB);
	sleep(1);

}


