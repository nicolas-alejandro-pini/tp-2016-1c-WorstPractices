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
void test_flush_tlb();

// Auxiliares
void thread_cpu(void *arg);

// Tipos de validacion
//CU_ASSERT_TRUE(value): Verifica que una expresión sea verdadera
//CU_ASSERT_FALSE(value): Verifica que una expresión sea falsa
//CU_ASSERT_EQUAL(actual, expected): Verifica que actual == expected
//CU_ASSERT_NOT_EQUAL(actual, expected)
//CU_ASSERT_STRING_EQUAL(actual, expected): Verifica que dos strings sean equivalentes
//CU_ASSERT_STRING_NOT_EQUAL(actual, expected)
//CU_ASSERT_PTR_EQUAL(actual, expected): Verifica que los punteros sean equivalentes
//CU_ASSERT_PTR_NOT_EQUAL(actual, expected)
//CU_ASSERT_PTR_NULL(value): Verifica que un puntero es NULL
//CU_ASSERT_PTR_NOT_NULL(value)
//CU_ASSERT_NSTRING_EQUAL(actual, expected, count): Verifica que los primeros count caracteres de las cadenas coinciden

#endif
