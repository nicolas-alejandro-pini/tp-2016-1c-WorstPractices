/*
 * test_umc.h
 *
 *  Created on: 17/6/2016
 *      Author: Nicolas Pini
 */
#ifndef TESTS_H_
#define TESTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <CUnit/Basic.h>
#define CANT_CPU 1
#define TEST_REEMPLAZO_X_PROC 10

pthread_t tests_threads[CANT_CPU];

int test_unit_umc();
int inicializar_umc();
int finalizar_umc();
void agregar_tests();

// Pruebas
void test_creacion_tlb();
void test_reemplazo_tlb();
void test_busqueda_tlb();
void test_estructuras_memoria();

// Auxiliares
void thread_cpu(void *arg);

#endif
