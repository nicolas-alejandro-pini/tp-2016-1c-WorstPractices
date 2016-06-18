/*
 * test_tlb.c
 *
 *  Created on: 17/6/2016
 *      Author: Nicolas Pini
 */
#include "test_umc.h"

#include "../Memoria.h"

int test_unit_umc() {

	CU_initialize_registry();
	agregar_tests();
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}

void agregar_tests(){
	CU_pSuite suite_umc = CU_add_suite("Pruebas UMC:", inicializar_umc, finalizar_umc);
	CU_add_test(suite_umc, "test tlb", test_tlb);
	CU_add_test(suite_umc, "test estructras memoria", test_estructuras_memoria);
}

int inicializar_umc(){
	if(0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int finalizar_umc(){
	return EXIT_SUCCESS;
}

// CASOS DE PRUEBA
void test_tlb(){

}

void test_estructuras_memoria(){
	int i;
	void *memoria;
	int pagina = 20;
	char string[pagina]; // en mi pagina guardo strings por ahora
	int cant_paginas = 10;
	memoria = inicializarMemoriaDisponible(pagina, cant_paginas);

	// Verifico que pueda escribir en todas las paginas
	for(i=0; i<cant_paginas; i++){
		sprintf(string,"Soy pagina [%d]", i);
		strcpy((void*)memoria + (pagina*i), string);
		//escribirMemoria(i, 0, 10, memoria[i]);
	}

	// Pueda leer en todas las paginas
	for(i=0; i<cant_paginas; i++)
	{
		sprintf(string,"Soy pagina [%d]", i);
		//strcpy(string, (char*)memoria + (pagina*i));
		//printf("[%d][%s]\n", i, string);
		CU_ASSERT_STRING_EQUAL((char*)memoria + (pagina*i), &string);
		//leerMemoria(memoria[i], 0, 10);
	}
	fflush(stdout);

}


