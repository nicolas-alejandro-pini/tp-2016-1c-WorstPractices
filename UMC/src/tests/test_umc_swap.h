/*
 * test_umc_swap.h
 *
 *  Created on: 12/7/2016
 *      Author: utnso
 */

#ifndef TESTS_TEST_UMC_SWAP_H_
#define TESTS_TEST_UMC_SWAP_H_

#include "test_umc.h"

int inicializar_umc_swap();
int finalizar_umc_swap();
void agregar_tests_con_swap();
int calcular_cantidad_paginas(int size_programa,int tamanio_paginas);


/* Pruebas */
void test_base_umc_swap();
void test_cambio_de_contexto();
void test_read_bytes_page();
void test_write_bytes_page();
void test_fin_programa();


#endif /* TESTS_TEST_UMC_SWAP_H_ */
