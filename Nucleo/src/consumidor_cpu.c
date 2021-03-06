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
		pcb_destroy(unPCB);
		return 0;
	}
	return 1;
}

int bloquear_pcb(stPCB *unPCB, char *dispositivo_name, int dispositivo_time) {
	stDispositivo *unDispositivo = NULL;
	stRafaga *unaRafagaIO = NULL;

	unDispositivo = buscar_dispositivo_io(dispositivo_name);
	if (unDispositivo == NULL)
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
	log_info("PCB [PID - %d] cambia de estado EXEC a BLOCK / dispositivo [%s]", unPCB->pid, unDispositivo->nombre);

	return EXIT_SUCCESS;
}

void *consumidor_cpu(void *param) {
	stHeaderIPC *unHeaderIPC;
	stMensajeIPC unMensajeIPC;
	stPCB *unPCB;
	t_paquete paquete;
	stSharedVar unaSharedVar;
	uint32_t socket_consola_to_print, pid_fin;
	t_valor_variable valor_impresion;
	stConsola * consola;
	stSemaforo *semaforo_request;
	char *dispositivo_name, *identificador_semaforo, *texto_imprimir;
	int error = 0, dispositivo_time, fin_ejecucion;
	int unCliente = *((int *) param);

	while (!error) {
		fin_ejecucion = 0;
		log_info("Esperando pcb para consumir ...");
		unPCB = ready_consumidor();

		if (!consola_activa(unPCB))
			continue;

		unPCB->quantum = obtenerEstadoActual().quantum;
		unPCB->quantumSleep = obtenerEstadoActual().quantumSleep;

		unHeaderIPC = nuevoHeaderIPC(EXECANSISOP);
		unHeaderIPC->largo = sizeof(uint32_t);
		if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
			log_error("No se pudo enviar el PCB[PID - %d]. Se lo devuelve a la cola de Ready", unPCB->pid);
			ready_productor(unPCB); //Devuelvo el PCB a la cola de Ready
			liberarHeaderIPC(unHeaderIPC);
			log_debug("Se cierra el hilo del CPU (Sock: %d)",unCliente);
			close(unCliente);
			pthread_exit(NULL);
		}
		liberarHeaderIPC(unHeaderIPC);
		unHeaderIPC = nuevoHeaderIPC(ERROR);
		if (!recibirHeaderIPC(unCliente, unHeaderIPC)) {
			log_error("No se pudo recibir el mensaje");
			liberarHeaderIPC(unHeaderIPC);
			log_info("Se cierra el hilo del CPU (Sock: %d)",unCliente);
			close(unCliente);
			pthread_exit(NULL);
		}
		if (unHeaderIPC->tipo == OK) {
			crear_paquete(&paquete, EXECANSISOP);
			serializar_pcb(&paquete, unPCB);
			if (enviar_paquete(unCliente, &paquete)) {
				log_error("Error al enviar el PCB [PID - %d]", unPCB->pid);
				log_info("Se cierra el hilo del CPU (Sock: %d)",unCliente);
				close(unCliente);
				pthread_exit(NULL);
				free_paquete(&paquete);
				continue;
			}
			log_debug("PCB [PID - %d] cambia de estado READY a EXEC (CPU - Sock [%d])", unPCB->pid, unCliente);
			pcb_destroy(unPCB);
			free_paquete(&paquete);
		}
		while (!fin_ejecucion) {
			if (!recibirHeaderIPC(unCliente, &unMensajeIPC.header)) {
				log_error("Error al recibir respuesta del CPU... devuelvo el PCB a la cola de ready");
				ready_productor(unPCB); //Devuelvo el PCB a la cola de Ready
				log_info("Se cierra el hilo del CPU (Sock: %d)",unCliente);
				close(unCliente);
				pthread_exit(NULL);
			} else {
				switch (unMensajeIPC.header.tipo) {
				case IOANSISOP:
					log_info("Se recibe pedido de I/O desde la CPU");
					dispositivo_name = malloc(unMensajeIPC.header.largo);
					recv(unCliente, dispositivo_name, unMensajeIPC.header.largo - sizeof(dispositivo_time), 0);
					recv(unCliente, &dispositivo_time, sizeof(dispositivo_time), 0);
					log_info("Nombre dispositivo [%s] | Cantidad de tiempo [%d]", dispositivo_name, dispositivo_time);
					log_debug("Se recibe un PCB a bloquear");

					if (recibir_paquete(unCliente, &paquete)) {
						log_error("No se pudo recibir el paquete");
						free_paquete(&paquete);
						log_debug("Se cierra el hilo del CPU (Sock: %d)",unCliente);
						close(unCliente);
						pthread_exit(NULL);
					}
					unPCB = malloc(sizeof(stPCB));
					deserializar_pcb(unPCB, &paquete);
					free_paquete(&paquete);
					/*Se comprueba que el PCB corresponda a una consola que este conectada, si esta desconectada libera el pcb que pide I/O y el CPU sigue con otro pcb*/
					if (!consola_activa(unPCB)) {
						fin_ejecucion = 1;
						break;
					}
					if (bloquear_pcb(unPCB, dispositivo_name, dispositivo_time) != EXIT_SUCCESS) {
						log_error("No se pudo bloquear el PCB [PID - %d]", unPCB->pid);
						continue;
					}

					fin_ejecucion = 1;

					break;
				case FINANSISOP:
					fin_ejecucion = 1;

					recv(unCliente, &pid_fin, sizeof(uint32_t), 0);
					log_info("Se recibe mensaje de fin de programa desde la CPU (PID: %d)", pid_fin);

					unHeaderIPC = nuevoHeaderIPC(FINPROGRAMA);
					unHeaderIPC->largo = sizeof(uint32_t);
					log_info("Envio de fin de programa a la UMC (Sock: %d)(PID: %d)", obtenerEstadoActual().sockUmc, pid_fin);
					if (!enviarMensajeIPC(obtenerEstadoActual().sockUmc, unHeaderIPC, (char*) &pid_fin)) {
						log_error("Error al enviar el fin de programa a la UMC");
					}
					log_info("Fin de programa enviado satisfactoriamente a la UMC");

					// Le debo enviar el fin de programa a la consola
					consola = obtenerSocketConsolaPorPID(pid_fin);
					if (consola == NULL) {
						log_warning("No se encontro el socket [PID - %d] en la lista", pid_fin);
						break;
					}
					log_debug("Le mando el fin de programa a la consola (Sock: %d)", consola->socket);
					if (!enviarHeaderIPC(consola->socket, unHeaderIPC)) {
						log_error("Error al enviar el fin de programa a la UMC");
					}
					log_info("Fin de programa enviado satisfactoriamente a la Consola");

					free(consola);
					liberarHeaderIPC(unHeaderIPC);
					break;
				case QUANTUMFIN:
					log_info("Solicitud de fin de quantum");
					fin_ejecucion = 1;
					if (recibir_paquete(unCliente, &paquete)) {
						log_error("No se pudo recibir el paquete");
						free_paquete(&paquete);
						log_debug("Se cierra el hilo del CPU (Sock: %d)",unCliente);
						close(unCliente);
						pthread_exit(NULL);
					}
					unPCB = malloc(sizeof(stPCB));
					log_debug("Antes de deserializar [%s]", imprimir_cola());
					deserializar_pcb(unPCB, &paquete);
					free_paquete(&paquete);

					if(consola_activa(unPCB)!= 0){
						/*Lo alojamos en la cola de ready para que vuelva a ser tomado por algun CPU*/
						log_debug("PCB [PID - %d] FIN QUANTUM", unPCB->pid);
						log_debug("Antes de entrar a la cola [%s]", imprimir_cola());
						ready_productor(unPCB);
						log_info("PCB [PID - %d] cambia de estado EXEC a READY", unPCB->pid);
					}

					break;
				case EXECERROR:
					log_debug("Recibido mensaje de EXEC ERROR CPU, falta tratamiento!!!");
					/*Se produjo una excepcion por acceso a una posicion de memoria invalida (segmentation fault), imprimir error
					 * y bajar la consola tambien close (cliente)*/
					break;
				case OBTENERVALOR:
					log_info("Solicitud de valor variable compartida...");
					unMensajeIPC.contenido = malloc(sizeof(unMensajeIPC.header.largo));
					recv(unCliente, unMensajeIPC.contenido, unMensajeIPC.header.largo, 0);
					unaSharedVar = obtener_shared_var((char*) unMensajeIPC.contenido);
					unHeaderIPC = nuevoHeaderIPC(OK);
					unHeaderIPC->largo = sizeof(t_valor_variable);
					if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
						log_error("Error al enviar el header IPC");
						log_info("Se cierra el hilo del CPU (Sock: %d)",unCliente);
						close(unCliente);
						pthread_exit(NULL);
					}
					send(unCliente, (t_valor_variable*) &unaSharedVar.valor, unHeaderIPC->largo, 0);
					liberarHeaderIPC(unHeaderIPC);
					log_info("Se devolvio el valor [%s] de la variable compartida [%d]", unaSharedVar.nombre, unaSharedVar.valor);
					free(unMensajeIPC.contenido);
					break;

				case GRABARVALOR:
					log_info("Solicitud de actualizacion de variable compartida");
					unaSharedVar.nombre = malloc(unMensajeIPC.header.largo - sizeof(t_valor_variable));
					recv(unCliente, unaSharedVar.nombre, unMensajeIPC.header.largo - sizeof(t_valor_variable), 0);
					recv(unCliente, &unaSharedVar.valor, sizeof(t_valor_variable), 0);
					grabar_shared_var(&unaSharedVar);
					log_info("Se actualizo con el valor [%s] de la variable compartida [%d]", unaSharedVar.nombre, unaSharedVar.valor);
					break;
				case WAIT:
					/*Wait del semaforo que pasa por parametro*/
					identificador_semaforo = malloc(unMensajeIPC.header.largo);
					recv(unCliente, identificador_semaforo, unMensajeIPC.header.largo, 0);
					semaforo_request = buscar_semaforo(identificador_semaforo);
					pthread_mutex_lock(&semaforo_request->mutex_bloqueados);
					log_info("Inicio pedido de WAIT de semaforo [%s]", semaforo_request->nombre);
					log_debug(imprimir_cola());
					//if (wait_semaforo(&semaforo_request) == EXIT_FAILURE) {
					if (--(semaforo_request->valor) < 0) {
						//Debe quedar bloqueado ya que el valor del semaforo < 0
						log_debug("Se envia mensaje WAIT_NO_OK al CPU (Sock:%d)",unCliente);
						unHeaderIPC = nuevoHeaderIPC(WAIT_NO_OK);
						if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
							log_error("Error en pedido de WAIT, no se pudo enviar mensaje de WAIT_NO_OK");
						}
						liberarHeaderIPC(unHeaderIPC);
						if (recibir_paquete(unCliente, &paquete)) {
							log_error("No se pudo recibir el paquete");
							log_info("Se cierra el hilo del CPU (Sock: %d)",unCliente);
							close(unCliente);
							pthread_exit(NULL);
						}
						unPCB = malloc(sizeof(stPCB));
						deserializar_pcb(unPCB, &paquete);
						free_paquete(&paquete);
						//Ingresa a la cola de procesos bloqueados del semaforo

						if(consola_activa(unPCB) != 0){
							queue_push(semaforo_request->bloqueados, unPCB);
							log_info("PCB [PID - %d] cambia de estado EXEC a BLOCK, entra a la cola de bloqueados del semaforo [%s]", unPCB->pid, identificador_semaforo);
							fin_ejecucion = 1;
							//El CPU debe agarrar otro PCB de la cola de listos y sigue su ejecucion normal
						}
					}
					else {
						unHeaderIPC = nuevoHeaderIPC(WAIT_OK);
						if (!enviarHeaderIPC(unCliente, unHeaderIPC)) {
							log_error("Error en pedido de WAIT, no se pudo enviar mensaje de WAIT_OK");
						}
						liberarHeaderIPC(unHeaderIPC);
						log_info("Fin pedido de WAIT al semaforo [%s]", identificador_semaforo);
						//Se realizo el WAIT correctamente, sigue su ejecucion normal. Con el mensaje de WAIT_OK el CPU NO debe mandar el PCB
					}
					pthread_mutex_unlock(&semaforo_request->mutex_bloqueados);
					free(identificador_semaforo);
					break;
				case SIGNAL:
					identificador_semaforo = malloc(unMensajeIPC.header.largo);
					recv(unCliente, identificador_semaforo, unMensajeIPC.header.largo, 0);
					semaforo_request = buscar_semaforo(identificador_semaforo);
					pthread_mutex_lock(&semaforo_request->mutex_bloqueados);
					log_info("Inicio pedido de SIGNAL de semaforo [%s]", semaforo_request->nombre);
					log_debug(imprimir_cola());
					//if (signal_semaforo(&semaforo_request) == EXIT_FAILURE) {
					++(semaforo_request->valor);

					while(queue_size(semaforo_request->bloqueados) > 0){
						unPCB = queue_pop(semaforo_request->bloqueados);
						if(consola_activa(unPCB) == 0){
							continue;
						} else {
							log_info("PCB [PID - %d] cambia de estado BLOCK a READY, sale de la cola de bloqueados del semaforo [%s]", unPCB->pid,semaforo_request->nombre);
							ready_productor(unPCB);
							break;
						}
					}

					log_info("Fin pedido de SIGNAL al semaforo [%s]", identificador_semaforo);
					pthread_mutex_unlock(&semaforo_request->mutex_bloqueados);
					break;
				case IMPRIMIR:

					recv(unCliente, &socket_consola_to_print, sizeof(uint32_t), 0);
					recv(unCliente, &valor_impresion, sizeof(t_valor_variable), 0);

					if(buscar_consola_por_socket(socket_consola_to_print) !=0){
						unHeaderIPC = nuevoHeaderIPC(IMPRIMIR);
						unHeaderIPC->largo = sizeof(t_valor_variable);
						log_debug("Se envia al socket [%d] de la consola, el valor [%d] para imprimir", socket_consola_to_print, valor_impresion);
						if (!enviarMensajeIPC(socket_consola_to_print, unHeaderIPC, (char*) &valor_impresion)) {
							log_error("Error al imprimir en consola el valor de la variable");
						}
						log_info("Se proceso una solicitud de impresion...");
						liberarHeaderIPC(unHeaderIPC);
					}
					break;
				case IMPRIMIRTEXTO:
					/*Me comunico con la correspondiente consola que inicio el PCB*/
					log_debug("Solicitud de impresion de texto...");

					recv(unCliente, &socket_consola_to_print, sizeof(uint32_t), 0);
					texto_imprimir = malloc(unMensajeIPC.header.largo - sizeof(uint32_t));
					recv(unCliente, texto_imprimir, unMensajeIPC.header.largo - sizeof(uint32_t), 0);

					if(buscar_consola_por_socket(socket_consola_to_print) !=0){
						unHeaderIPC = nuevoHeaderIPC(IMPRIMIRTEXTO);
						unHeaderIPC->largo = strlen(texto_imprimir) + 1;
						if (!enviarMensajeIPC(socket_consola_to_print, unHeaderIPC, texto_imprimir)) {
							log_error("Error al enviar el texto a imprimir");
						}
						liberarHeaderIPC(unHeaderIPC);
						log_info("Se proceso una solicitud de impresion de texto");
					}
					free(texto_imprimir);
					break;
				}
			}
		}
	}
	pthread_exit(NULL);
}

