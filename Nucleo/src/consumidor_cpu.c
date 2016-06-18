/*
 * consumidor_cpu.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */
#include "consumidor_cpu.h"

void consumidor_cpu(void *args) {
	struct thread_cpu_arg_struct *current_args = args;

	stHeaderIPC *unHeaderIPC;
	stMensajeIPC unMensajeIPC;
	stPCB *unPCB;
	t_paquete paquete;
	stDispositivo *unDispositivo;
	stRafaga *unaRafagaIO;
	char *dispositivo_name;
	int error = 0;

	while (!error) {
		unPCB = ready_consumidor();
		unPCB->quantum = current_args->estado->quantum;
		unPCB->quantumSleep = current_args->estado->quantumSleep;

		unHeaderIPC = nuevoHeaderIPC(EXECANSISOP);
		if (!enviarHeaderIPC(current_args->socketCpu, unHeaderIPC)) {
			log_error("CPU error - No se pudo enviar el PCB[%d]", unPCB->pid);
			error = 1;
			liberarHeaderIPC(unHeaderIPC);
			ready_productor(&unPCB);
			close(current_args->socketCpu);
			continue;
		}

		if (!recibirHeaderIPC(current_args->socketCpu, unHeaderIPC)) {
			log_error("CPU error - No se pudo recibir el mensaje");
			error = 1;
			ready_productor(&unPCB);
			close(current_args->socketCpu);
			continue;
		}

		if (unHeaderIPC->tipo == OK) {
			crear_paquete(&paquete, EXECANSISOP);
			serializar_pcb(&paquete, unPCB);

			if (enviar_paquete(current_args->socketCpu, &paquete)) {
				log_error("CPU error - No se pudo enviar el PCB[%d]", unPCB->pid);
				error = 1;
				ready_productor(&unPCB);
				close(current_args->socketCpu);
				continue;
			}

			free_paquete(&paquete);

		}

		if (!recibirMensajeIPC(current_args->socketCpu, &unMensajeIPC)) {
			log_error("CPU error - No se pudo recibir header");
			error = 1;
			close(current_args->socketCpu);
			continue;
		} else {
			switch (unMensajeIPC.header.tipo) {
			case IOANSISOP:
				/*Busqueda de dispositivo*/
				dispositivo_name = strdup(unMensajeIPC.contenido);
				int _es_el_dispositivo(stDispositivo *d) {
					return string_equals_ignore_case(d->nombre, dispositivo_name);
				}
				unDispositivo = list_remove_by_condition(current_args->estado->dispositivos, (void*) _es_el_dispositivo);

				/*Envio confirmacion al CPU*/
				unHeaderIPC = nuevoHeaderIPC(OK);
				if (!enviarHeaderIPC(current_args->socketCpu, unHeaderIPC)) {
					log_error("CPU error - No se pudo enviar header");
					error = 1;
					close(current_args->socketCpu);
					continue;
				}
				/*Recibo el PCB*/
				if (!recibir_paquete(current_args->socketCpu, &paquete)) {
					log_error("CPU error - No se pudo recibir header");
					error = 1;
					close(current_args->socketCpu);
					continue;
				}

				deserializar_pcb(&unPCB, &paquete);

				/*Almacenamos la rafaga de ejecucion de entrada salida*/
				unaRafagaIO = malloc(sizeof(stRafaga));
				unaRafagaIO->pid = unPCB->pid;
				unaRafagaIO->unidades = 2;

				queue_push(unDispositivo->rafagas, unaRafagaIO);

				/*Volvemos a almacenar el dispositivo en la lista*/
				list_add(current_args->estado->dispositivos, unDispositivo);
				list_add(listaBlock, &unPCB);

				//log_info("PCB[%d] ingresa a la cola de ejecucion de %s \n", unPCB->pid, unDispositivo->nombre);

				continue;

				break;
			case FINANSISOP:
				/*Termina de ejecutar el PCB, en este caso deberia moverlo a la cola de EXIT para que luego sea liberada la memoria*/
				break;
			case QUANTUMFIN:

				if(recibir_paquete (current_args->socketCpu, &paquete)){
					log_error("No se pudo recibir el paquete\n");
					error = 1;
					close(current_args->socketCpu);
					continue;
				}

				deserializar_pcb(&unPCB , &paquete);
				printf("PCB[%d] termino con su quantum\n", unPCB->pid);
				ready_productor(unPCB);
				free_paquete(&paquete);

				break;
			case EXECERROR:
				/*Se produjo una excepcion por acceso a una posicion de memoria invalida (segmentation fault), imprimir error
				 * y bajar la consola tambien close (cliente)*/
				break;
			case SIGUSR1CPU:
				/*Se cayo el CPU, se debe replanificar, (continue) */
				break;
			case OBTENERVALOR:
				/*Valor de la variable compartida, devolver el valor para que el CPU siga ejecutando*/
				break;
			case GRABARVALOR:
				/*Me pasa la variable compartida y el valor*/
				break;
			case WAIT:
				/*Bloquea con el semaforo que pasa por parametro*/
				break;
			case SIGNAL:
				/*libera con el semaforo que pasa por parametro*/
				break;

			}

		}

	}

	if (error) {
		/*Lo ponemos en la cola de Ready para que otro CPU lo vuelva a tomar*/
		ready_productor(&unPCB);
		log_info("PCB[%d] vuelve a ingresar a la cola de Ready \n", unPCB->pid);
	}

	liberarHeaderIPC(unHeaderIPC);
}
