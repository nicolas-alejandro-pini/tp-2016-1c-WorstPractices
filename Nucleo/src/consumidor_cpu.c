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
	if (buscar_consola(unPCB->pid) == 0) {
		log_info("PCB [PID - %d] corresponde a un ansisop abortado, se procede a finalizar el PCB ...", unPCB->pid);
		unHeaderIPC = nuevoHeaderIPC(FINPROGRAMA);
		unHeaderIPC->largo = sizeof(uint32_t);
		if (!enviarMensajeIPC(obtenerEstadoActual().sockUmc, unHeaderIPC, (char*) &unPCB->pid)) {
			log_error("Error al enviar el fin de programa a la UMC");
			return (-4);
		}
		liberarHeaderIPC(unHeaderIPC);
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

void *consumidor_cpu(void *param) {
	stHeaderIPC *unHeaderIPC;
	stMensajeIPC unMensajeIPC;
	stPCB *unPCB;
	t_paquete paquete;
	stSharedVar *unaSharedVar;
	uint32_t socket_consola_to_print,pid_fin;
	t_valor_variable valor_impresion;
	stConsola * consola;
	char *dispositivo_name, *identificador_semaforo, *texto_imprimir;
	int error = 0, offset = 0, dispositivo_time,fin_ejecucion;
	int unCliente = *((int *)param);

	while (!error) {
		fin_ejecucion = 0;
		unPCB = ready_consumidor();

		if(!consola_activa(unPCB))
			continue;

		unPCB->quantum = obtenerEstadoActual().quantum;
		unPCB->quantumSleep = obtenerEstadoActual().quantumSleep;

		unHeaderIPC = nuevoHeaderIPC(EXECANSISOP);
		unHeaderIPC->largo = sizeof(uint32_t);
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
			printf("PCB [PID - %d] READY a EXEC\n", unPCB->pid);
			free_paquete(&paquete);
		}
		while(fin_ejecucion==0&&error!=1){
		if (!recibirHeaderIPC(unCliente, &unMensajeIPC.header)) {
			log_error("Error al recibir respuesta del CPU");
			error = 1;
			close(unCliente);
			continue;
		} else {
			switch (unMensajeIPC.header.tipo) {
			case IOANSISOP:
				log_info("Recibido mensaje de I/O desde la CPU");
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
				printf("PCB [PID - %d] READY a EXEC\n", unPCB->pid);
				if(!consola_activa(unPCB)){
					continue;
				}
				if(bloquear_pcb(unPCB,dispositivo_name,dispositivo_time)!=EXIT_SUCCESS){
					log_error("No se pudo bloquear el PCB [PID - %d]",unPCB->pid);
					continue;
				}

				break;
			case FINANSISOP:
				fin_ejecucion = 1;
				recv(unCliente, &pid_fin, sizeof(uint32_t), 0);
				log_info("Recibido mensaje de fin de programa desde la CPU (PID: %d)", pid_fin);

				unHeaderIPC = nuevoHeaderIPC(FINPROGRAMA);
				unHeaderIPC->largo = sizeof(uint32_t);
				log_info("Le mando el fin de programa a la UMC (Sock: %d)(PID: %d)", obtenerEstadoActual().sockUmc, pid_fin);
				if (!enviarMensajeIPC(obtenerEstadoActual().sockUmc, unHeaderIPC, (char*)&pid_fin)) {
					log_error("Error al enviar el fin de programa a la UMC");
				}
				log_info("Fin de programa enviado satisfactoriamente a la UMC");

				// Le debo enviar el fin de programa a la consola
				consola = obtenerSocketConsolaPorPID(pid_fin);
				log_info("Le mando el fin de programa a la consola (Sock: %d)", consola->socket);
				if (!enviarHeaderIPC(consola->socket, unHeaderIPC)) {
					log_error("Error al enviar el fin de programa a la UMC");
				}
				log_info("Fin de programa enviado satisfactoriamente a la Consola");

				free(consola);
				liberarHeaderIPC(unHeaderIPC);
				break;
			case QUANTUMFIN:
				log_info("Recibido mensaje de fin de quantum desde la CPU");
				fin_ejecucion = 1;
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
				log_info("Recibido mensaje de EXEC ERROR CPU, falta tratamiento!!!");
				/*Se produjo una excepcion por acceso a una posicion de memoria invalida (segmentation fault), imprimir error
				 * y bajar la consola tambien close (cliente)*/
				break;
			case OBTENERVALOR:
				log_info("Nuevo pedido de variable compartida...");

				/*TODO: Falta testear*/
				unaSharedVar = obtener_shared_var(unMensajeIPC.contenido);
				if(!enviarMensajeIPC(unCliente,unHeaderIPC,(char*)unaSharedVar->valor)){
					log_error("Error al enviar el valor la variable");
					error = 1;
					continue;
				}
				log_info("Se devolvio el valor [%s] de la variable compartida [%d]\n",unaSharedVar->nombre,unaSharedVar->valor);
				break;

			case GRABARVALOR:
				log_info("Nuevo pedido de actualizacion de variable compartida\n");
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
				log_info("Se actualizo con el valor [%s] de la variable compartida [%d]\n",unaSharedVar->nombre,unaSharedVar->valor);
				free_paquete(&paquete);
				break;
			case WAIT:
				log_info("Nuevo pedido de wait de semaforo ");
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

				break;
			case SIGNAL:
				/*Signal del semaforo que pasa por parametro*/
				log_info("Nuevo pedido de signal de semaforo ");
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
				free_paquete(&paquete);
				break;
			case IMPRIMIR:
				/*Me comunico con la correspondiente consola que inicio el PCB*/
				log_info("Nuevo pedido de impresion...");
				recv(unCliente, &socket_consola_to_print, sizeof(uint32_t), 0);
				recv(unCliente, &valor_impresion, sizeof(t_valor_variable), 0);

				unHeaderIPC = nuevoHeaderIPC(IMPRIMIR);
				unHeaderIPC->largo = sizeof(t_valor_variable);
				printf("Se envia al socket [%d] de la consola, el valor [%d] para imprimir\n",socket_consola_to_print,valor_impresion);
				if(!enviarMensajeIPC(socket_consola_to_print,unHeaderIPC,(char*)&valor_impresion)){
					log_error("Error al imprimir en consola el valor de la variable");
				}
				liberarHeaderIPC(unHeaderIPC);

				break;
			case IMPRIMIRTEXTO:
				/*Me comunico con la correspondiente consola que inicio el PCB*/
				log_info("Nuevo pedido de impresion...");

				recv(unCliente, &socket_consola_to_print, sizeof(uint32_t), 0);
				recv(unCliente, &texto_imprimir, unHeaderIPC->largo - sizeof(uint32_t), 0);

				unHeaderIPC = nuevoHeaderIPC(IMPRIMIRTEXTO);
				unHeaderIPC->largo = strlen(texto_imprimir)+1;
				if(!enviarMensajeIPC(socket_consola_to_print,unHeaderIPC,texto_imprimir)){
					log_error("Error al enviar el texto a imprimir");
				}
				liberarHeaderIPC(unHeaderIPC);
				log_info("Se imprimio el valor [%d]\n",valor_impresion);
				break;
			}
		}
		}
	}
	if (error) {
		/*Lo ponemos en la cola de Ready para que otro CPU lo vuelva a tomar*/
		ready_productor(unPCB);
		log_info("PCB [PID - %d] EXEC a READY\n", unPCB->pid);
		liberarHeaderIPC(unHeaderIPC);
		close(unCliente);
		pthread_exit(NULL);
	}

	liberarHeaderIPC(unHeaderIPC);
	pthread_exit(NULL);
	return NULL;
}


