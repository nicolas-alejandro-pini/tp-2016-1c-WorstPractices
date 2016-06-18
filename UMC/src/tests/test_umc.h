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
#include <CUnit/Basic.h>


void agregar_tests();
int inicializar_umc();
int finalizar_umc();

int test_unit_umc();
void test_tlb();
void test_estructuras_memoria();

#endif
