/*
 * consumidor_cpu.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */
#include "includes/consumidor_cpu.h"



/*Corroboramos  si el pcb corresponde a una consola activa*/
int consola_activa(stPCB *unPCB) {
	stHeaderIPC *unHeaderIPC;
	if (buscar_consola_activa(unPCB->pid) == 0) {
		unHeaderIPC = nuevoHeaderIPC(FINPROGRAMA);
		if (!enviarMensajeIPC(obtenerEstadoActual().sockUmc, unHeaderIPC, (char*) unPCB->pid)) {
			log_error("Error al enviar el fin de programa a la UMC");
			return (-4);
		}
		pcb_destroy(unPCB);
		return 0;
	}

	return 1;
}

int bloquear_pcb(stPCB *unPCB,char *dispositivo_name,int dispositivo_time){
	stDispositivo *unDispositivo = NULL;
	stRafaga *unaRafagaIO = NULL;

	unDispositivo = buscar_dispositivo_io(dispositivo_name);
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
	agregar_pcb_listaBlock(unPCB);
	printf("PCB [PID - %d] en estado BLOCK / dispositivo [%s]\n", unPCB->pid,unDispositivo->nombre);

	return EXIT_SUCCESS;
}

void *consumidor_cpu(int unCliente) {
	stHeaderIPC *unHeaderIPC;
	stMensajeIPC unMensajeIPC;
	stPCB *unPCB;
	t_paquete paquete;
	stSharedVar *unaSharedVar;
	char *dispositivo_name, *identificador_semaforo, *texto_imprimir;
	int error = 0, offset = 0, valor_impresion, socket_consola_to_print, dispositivo_time;

	while (!error) {
		unPCB = ready_consumidor();

		if(!consola_activa(unPCB))
			continue;

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
				log_error("Error al enviar el PCB [PID - %d]", unPCB->pid);
				free_paquete(&paquete);
				error = 1;
				continue;
			}
			free_paquete(&paquete);
		}

		if (!recibirMensajeIPC(unCliente, &unMensajeIPC)) {
			log_error("Error al recibir respuesta del CPU");
			error = 1;
			close(unCliente);
			continue;
		} else {
			switch (unMensajeIPC.header.tipo) {
			case IOANSISOP:
				/*Se recibe el nombre del dispositivo y tiempo*/
				if(recibir_paquete (unCliente, &paquete)){
					log_error("No se pudo recibir el paquete\n");
					error = 1;
					free_paquete(&paquete);
					continue;
				}
				deserializar_campo(&paquete, &offset, &dispositivo_name, sizeof(dispositivo_name));
				deserializar_campo(&paquete, &offset, &dispositivo_time, sizeof(dispositivo_time));

				free_paquete(&paquete);
				if (recibir_paquete(unCliente, &paquete)) {
					log_error("No se pudo recibir el paquete\n");
					error = 1;
					free_paquete(&paquete);
					continue;
				}
				deserializar_pcb(unPCB, &paquete);
				free_paquete(&paquete);
				/*Se comprueba que el PCB corresponda a una consola que este conectada, si esta desconectada libera el pcb que pide I/O y el CPU sigue con otro pcb*/
				if(!consola_activa(unPCB)){
					continue;
				}
				if(bloquear_pcb(unPCB,dispositivo_name,dispositivo_time)!=EXIT_SUCCESS){
					log_error("No se pudo bloquear el PCB [PID - %d]",unPCB->pid);
					continue;
				}

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
				printf("PCB [PID - %d] EXEC a READY\n", unPCB->pid);
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

				/*TODO: Falta testear*/
				unaSharedVar = obtener_shared_var(unMensajeIPC.contenido);
				if(!enviarMensajeIPC(unCliente,unHeaderIPC,(char*)unaSharedVar->valor)){
					log_error("Error al enviar el valor la variable");
					error = 1;
					continue;
				}
				printf("Se devolvio el valor [%s] de la variable compartida [%d]\n",unaSharedVar->nombre,unaSharedVar->valor);
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
				grabar_shared_var(unaSharedVar->nombre,&unaSharedVar->valor);
				printf("Se actualizo con el valor [%s] de la variable compartida [%d]\n",unaSharedVar->nombre,unaSharedVar->valor);
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
				free_paquete(&paquete);
				/*TODO: Falta testear*/
				printf("%s\n",identificador_semaforo);
				wait_semaforo(identificador_semaforo);
				printf("\n--------------------------------------\n");

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
				if(signal_semaforo(identificador_semaforo)== EXIT_FAILURE){
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
				printf("Se imprimio el valor [%d]\n",valor_impresion);
				printf("\n--------------------------------------\n");

				break;
			case IMPRIMIRTEXTO:
				/*Me comunico con la correspondiente consola que inicio el PCB*/
				printf("\n--------------------------------------\n");
				printf("Nuevo pedido de impresion...\n");
				texto_imprimir = unMensajeIPC.contenido;
				if (!recibirMensajeIPC(unCliente, &unMensajeIPC)) {
					log_error("Error al recibir el mensaje de impresion");
					error = 1;
					continue;
				}
				socket_consola_to_print = atoi(unMensajeIPC.contenido);
				unHeaderIPC = nuevoHeaderIPC(IMPRIMIRTEXTO);
				if(!enviarMensajeIPC(socket_consola_to_print,unHeaderIPC,texto_imprimir)){
					log_error("Error al enviar el texto a imprimir");
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
		printf("PCB [PID - %d] EXEC a READY\n", unPCB->pid);
		liberarHeaderIPC(unHeaderIPC);
		close(unCliente);
		pthread_exit(NULL);
	}

	liberarHeaderIPC(unHeaderIPC);
	pthread_exit(NULL);
	return NULL;
}


