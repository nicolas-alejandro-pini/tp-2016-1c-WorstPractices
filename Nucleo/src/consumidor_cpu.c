/*
 * consumidor_cpu.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */
#include "includes/consumidor_cpu.h"

void *consumidor_cpu(int unCliente) {
	stHeaderIPC *unHeaderIPC;
	stMensajeIPC unMensajeIPC;
	stPCB *unPCB;
	t_paquete paquete;
	stDispositivo *unDispositivo;
	stRafaga *unaRafagaIO;
	stSharedVar *unaSharedVar;
	char *dispositivo_name, *identificador_semaforo;
	int error = 0, offset = 0, valor_impresion, socket_consola_to_print;

	while (!error) {
		unPCB = ready_consumidor();
		unPCB->quantum = obtenerEstadoActual().quantum;
		unPCB->quantumSleep = obtenerEstadoActual().quantumSleep;

		unHeaderIPC = nuevoHeaderIPC(EXECANSISOP);
		if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
			log_error("CPU error - No se pudo enviar el PCB[PID - %d]", unPCB->pid);
			liberarHeaderIPC(unHeaderIPC);
			error = 1;
			continue;
		}
		liberarHeaderIPC(unHeaderIPC);
		unHeaderIPC = nuevoHeaderIPC(ERROR);
		if (!recibirHeaderIPC(unCliente, unHeaderIPC)) {
			log_error("CPU error - No se pudo recibir el mensaje");
			liberarHeaderIPC(unHeaderIPC);
			error = 1;
			continue;
		}
		if (unHeaderIPC->tipo == OK) {
			crear_paquete(&paquete, EXECANSISOP);
			serializar_pcb(&paquete, unPCB);

			if (enviar_paquete(unCliente, &paquete)) {
				log_error("CPU error - No se pudo enviar el PCB[%d]", unPCB->pid);
				error = 1;
				continue;
			}
			free_paquete(&paquete);
		}

		if (!recibirMensajeIPC(unCliente, &unMensajeIPC)) {
			log_error("CPU error - No se pudo recibir header");
			error = 1;
			close(unCliente);
			continue;
		} else {
			switch (unMensajeIPC.header.tipo) {
			case IOANSISOP:
				/*Busqueda de dispositivo*/
				dispositivo_name = strdup(unMensajeIPC.contenido);
				int _es_el_dispositivo(stDispositivo *d) {
					return string_equals_ignore_case(d->nombre, dispositivo_name);
				}
				unDispositivo = list_remove_by_condition(obtenerEstadoActual().dispositivos, (void*) _es_el_dispositivo);

				/*Envio confirmacion al CPU*/
				unHeaderIPC = nuevoHeaderIPC(OK);
				if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
					log_error("CPU error - No se pudo enviar header");
					error = 1;
					continue;
				}
				liberarHeaderIPC(unHeaderIPC);
				/*Recibo el PCB*/
				if (!recibir_paquete(unCliente, &paquete)) {
					log_error("CPU error - No se pudo recibir header");
					error = 1;
					continue;
				}
				deserializar_pcb(unPCB, &paquete);
				/*Almacenamos la rafaga de ejecucion de entrada salida*/
				unaRafagaIO = malloc(sizeof(stRafaga));
				unaRafagaIO->pid = unPCB->pid;
				unaRafagaIO->unidades = 2;

				queue_push(unDispositivo->rafagas, unaRafagaIO);

				/*Volvemos a almacenar el dispositivo en la lista*/
				list_add(obtenerEstadoActual().dispositivos, unDispositivo);
				list_add(listaBlock, &unPCB);

				printf("PCB [PID - %d] en estado BLOCK / dispositivo [%s]\n", unPCB->pid,unDispositivo->nombre);
				//log_info("PCB[%d] ingresa a la cola de ejecucion de %s \n", unPCB->pid, unDispositivo->nombre);
				continue;

				break;
			case FINANSISOP:
				/*Termina de ejecutar el PCB, en este caso deberia moverlo a la cola de EXIT para que luego sea liberada la memoria*/
				break;
			case QUANTUMFIN:
				if(recibir_paquete (unCliente, &paquete)){
					log_error("No se pudo recibir el paquete\n");
					error = 1;
					free_paquete(&paquete);
					continue;
				}
				deserializar_pcb(unPCB , &paquete);
				free_paquete(&paquete);
				/*Lo alojamos en la cola de ready para que vuelva a ser tomado por algun CPU*/
				printf("PCB [PID - %d] FIN QUANTUM\n", unPCB->pid);
				ready_productor(unPCB);
				printf("PCB [PID - %d] en estado READY\n", unPCB->pid);
				break;
			case EXECERROR:
				/*Se produjo una excepcion por acceso a una posicion de memoria invalida (segmentation fault), imprimir error
				 * y bajar la consola tambien close (cliente)*/
				break;
			case SIGUSR1CPU:
				/*Se cayo el CPU, se debe replanificar, (continue) */
				break;
			case OBTENERVALOR:
				printf("\n--------------------------------------\n");
				printf("Nuevo pedido de variable compartida...\n");
				/*Valor de la variable compartida, devolver el valor para que el CPU siga ejecutando*/
				if (!recibirContenido(unCliente, unMensajeIPC.contenido,unHeaderIPC->largo)) {
					log_error("CPU error - No se pudo recibir la variable");
					error = 1;
					continue;
				}

				/*TODO: Falta testear*/
				unaSharedVar = obtener_shared_var(listaSharedVars, unMensajeIPC.contenido);
				if(!enviarMensajeIPC(unCliente,unHeaderIPC,(char*)unaSharedVar->valor)){
					log_error("CPU error - No se pudo enviar el valor la variable");
					error = 1;
					continue;
				}
				printf("Se devolvio el valor [%d] de la variable compartida [%s]\n",unaSharedVar->nombre,unaSharedVar->valor);
				printf("\n--------------------------------------\n");
				break;

			case GRABARVALOR:
				printf("\n--------------------------------------\n");
				printf("Nuevo pedido de actualizacion de variable compartida\n");
				/*Me pasa la variable compartida y el valor*/
				unHeaderIPC = nuevoHeaderIPC(OK);
				if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
					log_error("CPU error - No se pudo enviar header");
					error = 1;
					continue;
				}

				offset = 0;
				if (recibir_paquete(unCliente, &paquete)) {
					log_error("No se pudo recibir el paquete\n");
					error = 1;
					continue;
				}

				deserializar_campo(&paquete, &offset, &unaSharedVar, sizeof(stSharedVar));
				grabar_shared_var(listaSharedVars,unaSharedVar->nombre,unaSharedVar->valor);

				printf("Se actualizo con el valor [%d] de la variable compartida [%s]\n",unaSharedVar->nombre,unaSharedVar->valor);
				printf("\n--------------------------------------\n");
				free_paquete(&paquete);
				break;
			case WAIT:
				printf("\n--------------------------------------\n");
				printf("Nuevo pedido de wait de semaforo ");
				/*Wait del semaforo que pasa por parametro*/
				offset = 0;
				if (recibir_paquete(unCliente, &paquete)) {
					log_error("No se pudo recibir el paquete\n");
					error = 1;
					continue;
				}

				deserializar_campo(&paquete, &offset, &identificador_semaforo, sizeof(identificador_semaforo));
				/*TODO: Falta testear*/
				printf("%s\n",identificador_semaforo);
				if(wait_semaforo(listaSem,identificador_semaforo)== EXIT_FAILURE){
					unHeaderIPC = nuevoHeaderIPC(ERROR);
					if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
						log_error("CPU error - No se pudo enviar header");
						error = 1;
						continue;
					}
				}
				printf("\n--------------------------------------\n");
				free_paquete(&paquete);

				break;
			case SIGNAL:
				/*Signal del semaforo que pasa por parametro*/
				printf("\n--------------------------------------\n");
				printf("Nuevo pedido de signal de semaforo ");
				offset = 0;
				if (recibir_paquete(unCliente, &paquete)) {
					log_error("No se pudo recibir el paquete\n");
					error = 1;
					continue;
				}

				deserializar_campo(&paquete, &offset, &identificador_semaforo, sizeof(identificador_semaforo));

				/*TODO: Falta testear*/
				if(signal_semaforo(listaSem,identificador_semaforo)== EXIT_FAILURE){
					unHeaderIPC = nuevoHeaderIPC(ERROR);
					if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
						log_error("CPU error - No se pudo enviar header");
						error = 1;
						continue;
					}
				}

				printf("\n--------------------------------------\n");
				free_paquete(&paquete);
				break;
			case IMPRIMIR:
				/*Me comunico con la correspondiente consola que inicio el PCB*/
				printf("\n--------------------------------------\n");
				printf("Nuevo pedido de impresion...\n");
				if (!recibirContenido(unCliente, unMensajeIPC.contenido, unHeaderIPC->largo)) {
					log_error("CPU error - No se pudo recibir la variable");
					error = 1;
					continue;
				}
				valor_impresion = atoi(unMensajeIPC.contenido);
				if (!recibirMensajeIPC(unCliente, &unMensajeIPC)) {
					log_error("CPU error - No se pudo recibir header");
					error = 1;
					continue;
				}
				socket_consola_to_print = atoi(unMensajeIPC.contenido);
				unHeaderIPC = nuevoHeaderIPC(IMPRIMIR);
				if(!enviarMensajeIPC(socket_consola_to_print,unHeaderIPC,(char*)valor_impresion)){
					log_error("CPU error - No se pudo enviar el valor la variable");
					error = 1;
					continue;
				}
				liberarHeaderIPC(unHeaderIPC);
				unHeaderIPC = nuevoHeaderIPC(OK);
				if(!enviarHeaderIPC(unCliente,unHeaderIPC)){
					log_error("CPU error - No se pudo enviar la confirmacion de impresion");
					error = 1;
					continue;
				}
				liberarHeaderIPC(unHeaderIPC);
				printf("Se imprimio el valor [%d]\n",valor_impresion);
				printf("\n--------------------------------------\n");

				break;

			}

		}

	}
	if (error) {
		/*Lo ponemos en la cola de Ready para que otro CPU lo vuelva a tomar*/
		ready_productor(unPCB);
		liberarHeaderIPC(unHeaderIPC);
		close(unCliente);
		pthread_exit(NULL);
	}

	liberarHeaderIPC(unHeaderIPC);
	pthread_exit(NULL);
	return NULL;
}


