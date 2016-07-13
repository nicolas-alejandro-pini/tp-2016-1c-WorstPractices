/*
 * test_nucleo.h
 *
 *  Created on: 12/7/2016
 *      Author: utnso
 */

#ifndef TESTS_TEST_NUCLEO_H_
#define TESTS_TEST_NUCLEO_H_
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <CUnit/Basic.h>
#include "../includes/Nucleo.h"

int test_unit_nucleo();
void agregar_tests_nucleo();
void inicializar_nucleo();
int finalizar_nucleo();
void cargar_pcbs_bloqueados();
stDispositivo *buscar_dispositivo_io_test(char *dispositivo_name);

/*Pruebas*/
void test_colas_dispositivos();
void test_cola_ready();
void test_pcb_bloqueados();
#endif /* TESTS_TEST_NUCLEO_H_ */


