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
	CU_pSuite suite_umc = CU_add_suite("Pruebas UMC:", inicializar_umc, finalizar_umc);
	CU_add_test(suite_umc, "test creacion tlb", test_creacion_tlb);
	CU_add_test(suite_umc, "test reemplazo tlb", test_reemplazo_tlb);
	CU_add_test(suite_umc, "test busqueda tlb", test_busqueda_tlb);
	CU_add_test(suite_umc, "test estructras memoria", test_estructuras_memoria);
}

// Setea el entorno del suite para pruebas
int inicializar_umc(){
	int i;
	int cant_cpu = 4;
	char *msg[] = { "CPU 1", "CPU 2", "CPU 3", "CPU 4"};
	pthread_t threads[cant_cpu];

	for( i=0; i<cant_cpu; i++)
		if( pthread_create(&threads[i], NULL, (void*)&thread_cpu, msg[i]) ){
			perror("Ocurrio un error durante la creacion del Thread.");
		}

	for( i=0; i<cant_cpu; i++)
		pthread_join(threads[i], NULL);


	return EXIT_SUCCESS;
}

// Finaliza el suite de pruebas de la UMC
int finalizar_umc(){
	return EXIT_SUCCESS;
}

// CASOS DE PRUEBA

// Prueba 1
void test_creacion_tlb(){
	stParametro losParametros;
	loadInfo(&losParametros, "umc.conf");

	crearTLB(TLB, 10); //losParametros.entradasTLB);
	imprimirTLB(TLB);
	CU_ASSERT_EQUAL(cantidadRegistrosTLB(TLB), losParametros.entradasTLB);
}

// Prueba 2
void test_reemplazo_tlb(){

}

// Prueba 3
void test_busqueda_tlb(){

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
	char *msg = (char*) arg;
	printf("Thread ID: [%u] | %s\n", (unsigned int)pthread_self(), msg);
}


