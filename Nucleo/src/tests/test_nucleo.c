/*
 * test_nucleo.c
 *
 *  Created on: 12/7/2016
 *      Author: utnso
 */
#include "test_nucleo.h"
#include "../includes/Nucleo.h"
#include "../includes/nucleo_config.h"
#include <commons/collections/list.h>
#include <commons/pcb.h>
#include <CUnit/CUnit.h>

stEstado elEstadoActual;
t_list *listaBlock;
int cantidadDePaginasCodigo = 1;

int test_unit_nucleo() {

	CU_initialize_registry();
	//agregar_tests();
	agregar_tests_nucleo();
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}


void agregar_tests_nucleo(){
	CU_pSuite suite_nucleo = CU_add_suite("Prueba Nucleo:", inicializar_nucleo, finalizar_nucleo); // NULL, NULL);
	CU_add_test(suite_nucleo, "test_pcb_bloqueados()", test_pcb_bloqueados);
	CU_add_test(suite_nucleo, "test_colas_dispositivos()", test_colas_dispositivos);
	CU_add_test(suite_nucleo, "PCBs bloqueados pasan a Ready", test_colas_dispositivos);
}

void inicializar_nucleo(){
	elEstadoActual.path_conf="nucleo.conf";
	loadInfo(&elEstadoActual,0);
	listaBlock = list_create();
	inicializarThreadsDispositivos(&elEstadoActual);
	cargar_pcbs_bloqueados();
}

void cargar_pcbs_bloqueados(){
	stMensajeIPC unMensajeIPC;
	stPCB *unPCB1;
	stPCB *unPCB2;
	stPCB *unPCB3;
	stPCB *unPCB4;
	char* programa = "#!/usr/bin/ansisop\nbegin\n#primero declaro las variables\nvariables a, b\na = 20\nprint a\nend";
	unMensajeIPC.contenido = programa;

	unPCB1 = crear_pcb(0, cantidadDePaginasCodigo, elEstadoActual.stackSize, programa);
	unPCB2 = crear_pcb(0, cantidadDePaginasCodigo, elEstadoActual.stackSize, programa);
	unPCB3 = crear_pcb(0, cantidadDePaginasCodigo, elEstadoActual.stackSize, programa);
	unPCB4 = crear_pcb(0, cantidadDePaginasCodigo, elEstadoActual.stackSize, programa);
	bloquear_pcb_test(unPCB1,"Disco1",2);
	bloquear_pcb_test(unPCB2,"Disco1",3);
	bloquear_pcb_test(unPCB3,"Impresora",1);
	bloquear_pcb_test(unPCB4,"Disco4",2);

}

void test_colas_dispositivos(){
	int i = 0, cant_rafagas_disco1 = 0, cant_rafagas_disco4 = 0, cant_rafagas_impresora = 0;
	stDispositivo *unDispositivo = NULL;
	for (i = 0; i < list_size(elEstadoActual.dispositivos); ++i) {
		unDispositivo = list_get(elEstadoActual.dispositivos, i);
		if (unDispositivo->nombre == "Disco1") {
			for (i = 0; i < list_size(unDispositivo->rafagas); ++i) {
				cant_rafagas_disco1++;
			}
		}else if(unDispositivo->nombre == "Disco4"){
			for (i = 0; i < list_size(unDispositivo->rafagas); ++i) {
				cant_rafagas_disco4++;
			}
		}else if(unDispositivo->nombre == "Impresora"){
			for (i = 0; i < list_size(unDispositivo->rafagas); ++i) {
				cant_rafagas_impresora++;
			}
		}
	}
	free(unDispositivo);
	CU_ASSERT_TRUE((cant_rafagas_disco1+cant_rafagas_disco4+cant_rafagas_impresora)==8);
}

void test_cola_ready(){
	stPCB *unPCB;
	int vacio = 0;
	int count_pcb_ready = 0;
	while(vacio==0){
		unPCB = ready_consumidor();
		count_pcb_ready++;
	}
	CU_ASSERT_TRUE(count_pcb_ready==4);

}


void test_pcb_bloqueados(){
	int var;
	int count_block = 0;
	stPCB *unPCB;
	for (var = 0; var < list_size(listaBlock); ++var) {
		unPCB = list_get(listaBlock,var);
		printf("PCB [PID - %d]\n",unPCB->pid);
		count_block++;
		pcb_destroy(unPCB);
	}
	CU_ASSERT_TRUE(count_block==4);

}

int bloquear_pcb_test(stPCB *unPCB,char *dispositivo_name,int dispositivo_time){
	stDispositivo *unDispositivo = NULL;
	stRafaga *unaRafagaIO = NULL;

	unDispositivo = buscar_dispositivo_io_test(dispositivo_name);
	if(unDispositivo==NULL)
		return EXIT_FAILURE;

	/*Almacenamos la rafaga de ejecucion de entrada salida*/
	unaRafagaIO = malloc(sizeof(stRafaga));
	unaRafagaIO->pid = unPCB->pid;
	unaRafagaIO->unidades = dispositivo_time;

	pthread_mutex_lock(&unDispositivo->mutex); // Se lockea el acceso a la cola
	queue_push(unDispositivo->rafagas, unaRafagaIO);
	unDispositivo->numInq++;
	pthread_mutex_unlock(&unDispositivo->mutex);	// Se desbloquea el acceso a la cola
	pthread_mutex_unlock(&unDispositivo->empty);	// Comienzo de espera de consumidor

	/*Agregamos el pcb a la lista de bloqueados*/
	list_add(listaBlock, unPCB);
	printf("PCB [PID - %d] en estado BLOCK / dispositivo [%s]\n", unPCB->pid,unDispositivo->nombre);

	return EXIT_SUCCESS;
}

stDispositivo *buscar_dispositivo_io_test(char *dispositivo_name){
	stDispositivo *unDispositivo = NULL;
	/*Busqueda de dispositivo de I/O*/
	int _es_el_dispositivo(stDispositivo *d) {
		return string_equals_ignore_case(d->nombre, dispositivo_name);
	}
	unDispositivo = list_find(&elEstadoActual.dispositivos, (void*) _es_el_dispositivo);
	return unDispositivo;
}

int finalizar_nucleo(){
	return EXIT_SUCCESS;
}

