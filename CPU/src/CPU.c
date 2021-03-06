/*
 ============================================================================
 Name        : CPU.c
 Author      : Ezequiel Martinez
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "CPU.h"
#include "cpuSignals.h"

//Variables Globales//

fd_set fds_master;		/* Lista de todos mis sockets. */
fd_set read_fds;		/* Sublista de fds_master. */

int SocketAnterior = 0;
int tamanioPaginaUMC ;
int quantum=0;
int solicitudIO =0;
t_puntero ultimaPosicionStack = 0;
t_configCPU configuracionInicial; /* Estructura del CPU, contiene los sockets de conexion y parametros. */

stPCB* unPCB; /* Estructura del pcb para ejecutar las instrucciones */


void reemplazarBarraN(char* buffer){

	unsigned long int i , largo;

	largo = strlen(buffer);

	for (i=0; i<largo;i++){
		if(buffer[i]=='\n')
			buffer[i]='\0';
	}
	return;
}

/*
 ============================================================================
 Name        : Funciones Primitivas para ANSISOP Program.
 Author      : Ezequiel Martinez
 Inputs      : N/A
 Outputs     : N/A
 Description : Se declaran todas las funciones primitivas.
 ============================================================================
 */

t_puntero definirVariable(t_nombre_variable identificador_variable){

	stIndiceStack *indiceStack;
	stVars *unaVariable;
	int tamanioStack;

	log_debug("Se va a definir la variable %c",identificador_variable);

	tamanioStack=list_size(unPCB->stack);

	indiceStack = list_get(unPCB->stack, tamanioStack - 1);

	unaVariable = (stVars*)malloc(sizeof(stVars));
	unaVariable->id = identificador_variable;

	unaVariable->posicion_memoria.pagina = unPCB->offsetStack / tamanioPaginaUMC;
	unaVariable->posicion_memoria.offset = unPCB->offsetStack % tamanioPaginaUMC;
	unaVariable->posicion_memoria.size = TAMANIOVARIABLES;
	log_debug("Se define variable en el stack, pagina: %d , offset: %d, size: %d",unaVariable->posicion_memoria.pagina, unaVariable->posicion_memoria.offset, unaVariable->posicion_memoria.size);

	list_add(indiceStack->variables,unaVariable);

	unPCB->offsetStack = unPCB->offsetStack + TAMANIOVARIABLES;

	log_debug("Se definio la variable %c",identificador_variable);
	log_debug("Offset de la variable en el stack: %d",unaVariable->posicion_memoria.offset);
	log_debug("Offset stack actual: %d", unPCB->offsetStack);

	return (unPCB->offsetStack - TAMANIOVARIABLES);  // Inicio de la variable en el stack
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable ){

	stVars *unaVariable;
	stIndiceStack *indiceStack;
	unsigned long int cantVars, i;
	int tamanioStack;

	log_debug("Inicio primitiva obtenerPosicionVariable para variable %c.",identificador_variable);

	tamanioStack = list_size(unPCB->stack); //considero que en pila del stack el ultimo es el contexto actual.

	indiceStack = list_get(unPCB->stack, tamanioStack - 1);
	cantVars = list_size(indiceStack->variables);
	for(i = 0; i < cantVars; i++){
		unaVariable = list_get(indiceStack->variables, i);
		if (unaVariable->id == identificador_variable)
			break;
	}

	if (i==cantVars)
		return -1;

	log_debug("Se obtiene la posicion de la variable %c en %d. ",identificador_variable, unaVariable->posicion_memoria.pagina * tamanioPaginaUMC + unaVariable->posicion_memoria.offset);

	return (unaVariable->posicion_memoria.pagina * tamanioPaginaUMC + unaVariable->posicion_memoria.offset);

}

stPosicion *obtenerPosicion(t_puntero direccion_variable){
	stPosicion *unaPosicion;
	int pagina, offset;

	unaPosicion = (stPosicion*)malloc(sizeof(stPosicion));
	pagina = (direccion_variable / tamanioPaginaUMC);
	offset = direccion_variable % tamanioPaginaUMC;
	unaPosicion->size = TAMANIOVARIABLES;
	unaPosicion->pagina = pagina;
	unaPosicion->offset= offset;
	return unaPosicion;
}

t_valor_variable dereferenciar(t_puntero direccion_variable){

	stPosicion *posicionVariable;
	stHeaderIPC *unHeader;
	uint16_t pagina,offset,tamanio;
	t_valor_variable valor_variable;

	log_debug("Inicio primitiva dereferenciar con direccion_variable = %d",direccion_variable);

	posicionVariable = obtenerPosicion(direccion_variable);

	pagina = posicionVariable->pagina + unPCB->paginaInicioStack; // Le sumo la pagina del inicio del stack en la UMC.
	offset = posicionVariable->offset;
	tamanio = posicionVariable->size;

	unHeader = nuevoHeaderIPC(READ_BTYES_PAGE);
	unHeader->largo = sizeof(uint16_t)*3;

	if(!enviarHeaderIPC(configuracionInicial.sockUmc,unHeader)){
		log_error("Error al enviar mensaje de leer bytes intruccion.");
	}

	/*Envio los tres datos a la UMC*/
	send(configuracionInicial.sockUmc,&pagina,sizeof(uint16_t),0);
	send(configuracionInicial.sockUmc,&offset,sizeof(uint16_t),0);
	send(configuracionInicial.sockUmc,&tamanio,sizeof(uint16_t),0);

	log_debug("Se obtiene de la UMC, pagina %d, offset %d, size %d", pagina , offset, tamanio);

	if(!recibirHeaderIPC(configuracionInicial.sockUmc,unHeader)){
		log_error("No se pudo recibir la variable.");
	}

	if(unHeader->tipo == OK){
		recv(configuracionInicial.sockUmc, &valor_variable, unHeader->largo, 0);
		log_debug("Se obtiene de la UMC - Stack, valor de variable %d",valor_variable);
	}else{
		log_error("Error de segmentación al dereferenciar.");
		imprimirTexto("Error de segmentación al asignar.");
		//configuracionInicial.salir = 1;
		quantum = 0;
		unPCB->pc = unPCB->metadata_program->instrucciones_size + 1; // Simula fin de programa por stackoverflow.
		liberarHeaderIPC(unHeader);
		return (-1);
	}

	liberarHeaderIPC(unHeader);

	return valor_variable;

}

void asignar(t_puntero direccion_variable, t_valor_variable valor ){

	stPosicion *posicionVariable;
	stHeaderIPC *unHeader;
	uint16_t pagina = 0, offset = 0, tamanio = 0;
	uint16_t tamanioRestante = TAMANIOVARIABLES;
	char valor_parcial[TAMANIOVARIABLES];

	/* Asigno valor parcial */
	valor_parcial[3] = (valor>>24) & 0xFF;
	valor_parcial[2] = (valor>>16) & 0xFF;
	valor_parcial[1] = (valor>>8) & 0xFF;
	valor_parcial[0] = valor & 0xFF;

	valor = *(int *)valor_parcial; // Verifico que haya guardado bien el valor TODO BORRAR
	log_debug("Inicio primitiva asignar para grabar valor [%d]", valor);

	/* Posicion inicial */
	posicionVariable = obtenerPosicion(direccion_variable);
	pagina = posicionVariable->pagina + unPCB->paginaInicioStack;
	offset = posicionVariable->offset;

	while(tamanioRestante > 0){

		if(tamanioRestante + offset > tamanioPaginaUMC){
			tamanio = tamanioPaginaUMC - offset;
		}
		else
		{
			tamanio = tamanioRestante;
		}

		unHeader = nuevoHeaderIPC(WRITE_BYTES_PAGE);
		unHeader->largo = (sizeof(uint16_t) * 3) + tamanio;

		if(!enviarHeaderIPC(configuracionInicial.sockUmc,unHeader)){
			log_error("Error al enviar mensaje de leer bytes intruccion.");
		}

		/*Envio los tres datos a la UMC*/
		send(configuracionInicial.sockUmc,&pagina,sizeof(uint16_t),0);
		send(configuracionInicial.sockUmc,&offset,sizeof(uint16_t),0);
		send(configuracionInicial.sockUmc,&tamanio,sizeof(uint16_t),0);
		send(configuracionInicial.sockUmc,&valor_parcial[TAMANIOVARIABLES - tamanioRestante],tamanio,0);

		log_debug("Se asigna en la UMC: pagina %d, offset %d, size %d ",pagina, offset, tamanio);

		//log_debug("Se asigna en la UMC - Stack, valor: %d",valor);

		unHeader = nuevoHeaderIPC(ERROR);
		if(!recibirHeaderIPC(configuracionInicial.sockUmc, unHeader)){
			log_error("Error al recibir confirmacion de asignar en UMC.");
		}

		if(unHeader->tipo == OK)
				log_debug("Se asigno correctamente.");
			else{
				imprimirTexto("Error de segmentación al asignar.");
				log_error ("Error de segmentacion al asignar.");
				//configuracionInicial.salir = 1;
				unPCB->pc = unPCB->metadata_program->instrucciones_size + 1; // Simula fin de programa por stackoverflow.
				quantum = 0;
			}
		liberarHeaderIPC(unHeader);


		// Actualizo el tamanio restante no enviado
		tamanioRestante -= tamanio;
		// Me posiciono en la pagina de stack siguiente
		pagina++;
		// Seteo offset al inicio de la pagina siguiente
		offset = 0;
	} // While
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){

	stHeaderIPC *unHeaderIPC;
	t_valor_variable resultado = 0;

	//Elimino el barra n que manda el parser //
//	reemplazarBarraN(variable);

	unHeaderIPC = nuevoHeaderIPC(OBTENERVALOR);
	unHeaderIPC->largo = strlen(variable) +1;

	if(!enviarMensajeIPC(configuracionInicial.sockNucleo, unHeaderIPC, (char *) variable)){
		log_error("No se pudo enviar la variable %s", variable);
		liberarHeaderIPC(unHeaderIPC);
	}

	if(!recibirHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC)){
		log_error("No se pudo recibir el header");
		liberarHeaderIPC(unHeaderIPC);
	}

	if((unHeaderIPC->tipo == OK) && (unHeaderIPC->largo== sizeof(t_valor_variable))){
		recv(configuracionInicial.sockNucleo,&resultado,unHeaderIPC->largo,0);
	}

	liberarHeaderIPC(unHeaderIPC);
	return resultado;
}

int asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){

	stHeaderIPC *unHeaderIPC;

	//Elimino el barra n que manda el parser //
//	reemplazarBarraN(variable);

	//Hago el envío de la variabe con su valor
	unHeaderIPC = nuevoHeaderIPC(GRABARVALOR);
	unHeaderIPC->largo = strlen(variable) + 1 + sizeof(t_valor_variable);
	if(!enviarHeaderIPC(configuracionInicial.sockNucleo, unHeaderIPC)){
		log_error("No se pudo enviar el mensaje para grabar el valor");
		return -1;
	}

	send(configuracionInicial.sockNucleo,variable, strlen(variable) + 1,0);
	send(configuracionInicial.sockNucleo,&valor, sizeof(t_valor_variable),0);

	liberarHeaderIPC(unHeaderIPC);

	return valor;
}

void irAlLabel(t_nombre_etiqueta etiqueta){

	log_debug("Llamada a la primitiva irAlLabel para etiqueta [%s] .",etiqueta);
	t_puntero_instruccion ptr_instruccion;

	//Elimino el barra n que manda el parser //
//	reemplazarBarraN((char*)etiqueta);

	ptr_instruccion = metadata_buscar_etiqueta(etiqueta,unPCB->metadata_program->etiquetas,unPCB->metadata_program->etiquetas_size);

	log_debug("Se carga el pc con la posicion [%d] del label [%s].",ptr_instruccion,etiqueta);
	unPCB->pc = ptr_instruccion;
}

void llamarFuncionConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	/*Creamos una nuevo indice de stack correspondiente al nuevo call de la funcion*/
	/*Buscamos la etiqueta definida en el metadata, para actualizar el program counter del PCB*/
	log_debug("Llamada a la primitiva llamarFuncionConRetorno para etiqueta [%s] y retornar en [%d]",etiqueta , donde_retornar);
	stIndiceStack *unIndiceStack;
	unIndiceStack = (stIndiceStack*) malloc(sizeof(stIndiceStack));
	unIndiceStack->argumentos = list_create();
	unIndiceStack->pos = list_size(unPCB->stack) + 1;
	unIndiceStack->variables = list_create();
	//unIndiceStack->retPosicion = (uint32_t)donde_retornar;
	unIndiceStack->retPosicion = unPCB->pc;
	unIndiceStack->retVar.pagina = donde_retornar / tamanioPaginaUMC; // Pagina logica dentro del stack
	unIndiceStack->retVar.offset = donde_retornar % tamanioPaginaUMC;
	unIndiceStack->retVar.size = TAMANIOVARIABLES;
	list_add(unPCB->stack,unIndiceStack);

	// tiene contexto
	irAlLabel(etiqueta);
}
void retornar(t_valor_variable retorno){
	log_debug("Llamada a funcion retornar con valor de retorno [%d]", retorno);
	stIndiceStack *unIndiceStack;

	/*Sacamos del stack la variable a retornar*/
	unIndiceStack = list_remove(unPCB->stack,list_size(unPCB->stack) - 1);
	/*Actualizamos el program counter del pcb*/
	unPCB->pc = unIndiceStack->retPosicion;

//	unIndiceStack->retVar.pagina = unPCB->offsetStack / tamanioPaginaUMC;
//	log_debug("Se define variable de retorno, pagina: %d",unPCB->offsetStack / tamanioPaginaUMC);
//	unIndiceStack->retVar.offset= unPCB->offsetStack % tamanioPaginaUMC;
//	log_debug("Se define variable de retorno, offset: %d", unPCB->offsetStack % tamanioPaginaUMC);
//	unIndiceStack->retVar.size = TAMANIOVARIABLES;
//	log_debug("Se define variable de retorno, size: %d", TAMANIOVARIABLES);
 	asignar((unIndiceStack->retVar.pagina * tamanioPaginaUMC ) + unIndiceStack->retVar.offset, retorno);

}

void imprimir(t_valor_variable valor_mostrar){

	stHeaderIPC* unHeaderPrimitiva;

	unHeaderPrimitiva = nuevoHeaderIPC(IMPRIMIR);
	unHeaderPrimitiva->largo = sizeof(uint32_t) + sizeof(t_valor_variable);

	if(!enviarHeaderIPC(configuracionInicial.sockNucleo, unHeaderPrimitiva)){
		log_error("Error al enviar mensaje de IMPRIMIR.");
		return;
	}

	send(configuracionInicial.sockNucleo, &unPCB->socketConsola, sizeof(uint32_t), 0);
	send(configuracionInicial.sockNucleo, &valor_mostrar, sizeof(t_valor_variable), 0);

	log_debug("Se envio al nucleo el valor %d para ser impreso.", valor_mostrar);

	liberarHeaderIPC(unHeaderPrimitiva);
	return;
}

void imprimirTexto(char* texto){

	log_debug("Inicio primitiva imprimirTexto con valor:  %s",texto);
	stHeaderIPC* unHeaderPrimitiva;

	unHeaderPrimitiva = nuevoHeaderIPC(IMPRIMIRTEXTO);
	unHeaderPrimitiva->largo = strlen(texto) + 1 + sizeof(uint32_t);

	if(!enviarHeaderIPC(configuracionInicial.sockNucleo, unHeaderPrimitiva)){
		log_error("No se pudo enviar al Nucleo el texto: ", texto);
		return;
	}

	send(configuracionInicial.sockNucleo, &unPCB->socketConsola, sizeof(uint32_t), 0);
	send(configuracionInicial.sockNucleo, texto, strlen(texto) + 1, 0);

	log_debug("Se envio al nucleo la orden de imprimir el texto.");

	liberarHeaderIPC(unHeaderPrimitiva);

	return;
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	stHeaderIPC* unHeaderPrimitiva;

	//Elimino el barra n que manda el parser //
//	reemplazarBarraN(dispositivo);

	unHeaderPrimitiva = nuevoHeaderIPC(IOANSISOP);
	unHeaderPrimitiva->largo = strlen(dispositivo) + 1 + sizeof(int);

	enviarHeaderIPC(configuracionInicial.sockNucleo, unHeaderPrimitiva);

	send(configuracionInicial.sockNucleo, dispositivo,strlen(dispositivo) + 1, 0);
	send(configuracionInicial.sockNucleo, &tiempo,sizeof(int), 0);

	liberarHeaderIPC(unHeaderPrimitiva);

	quantum = 0; //Termino el quantum para devolver el pcb por IO
	solicitudIO = 1; // Flag globla para devolver PCB por bloqueo.

	return;
}

void wait(t_nombre_semaforo identificador_semaforo){

	stHeaderIPC* unHeaderPrimitiva;

	//Elimino el barra n que manda el parser //
//	reemplazarBarraN(identificador_semaforo);

	unHeaderPrimitiva = nuevoHeaderIPC(WAIT);
	unHeaderPrimitiva->largo = strlen(identificador_semaforo) + 1;

	enviarHeaderIPC(configuracionInicial.sockNucleo, unHeaderPrimitiva);

	send(configuracionInicial.sockNucleo,identificador_semaforo,strlen(identificador_semaforo) + 1,0 );


	if(!recibirHeaderIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva)){
		log_error("Error al recibir ok del wait.");
		configuracionInicial.salir = 1;
	}

	//En el caso que haya un error con el wait devuelvo el pcb al nucleo.//
	if(unHeaderPrimitiva->tipo == WAIT_NO_OK){
		quantum = 0; //Termino el quantum para devolver el pcb.
		solicitudIO = 1; // Flag global para devolver PCB por bloqueo.
		log_debug ("Recibi un WAIT_NO_OK del Nucleo. Quantum [%d]", quantum);
	}

	liberarHeaderIPC(unHeaderPrimitiva);
}

void signal_cpu(t_nombre_semaforo identificador_semaforo){

	stHeaderIPC* unHeaderPrimitiva;

	//Elimino el barra n que manda el parser //
//	reemplazarBarraN(identificador_semaforo);

	unHeaderPrimitiva = nuevoHeaderIPC(SIGNAL);
	unHeaderPrimitiva->largo = strlen(identificador_semaforo) + 1;

	enviarHeaderIPC(configuracionInicial.sockNucleo, unHeaderPrimitiva);

	send (configuracionInicial.sockNucleo, identificador_semaforo,strlen(identificador_semaforo) + 1, 0 );

	liberarHeaderIPC(unHeaderPrimitiva);
}

/**
 * Finaliza un programa AnSISOP
 *
 */
void finalizar(){
	quantum=0;
}

AnSISOP_funciones AnSISOP_functions = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dereferenciar,
		.AnSISOP_asignar				= asignar,
		.AnSISOP_imprimir				= imprimir,
		.AnSISOP_imprimirTexto			= imprimirTexto,
		.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida = asignarValorCompartida,
		.AnSISOP_irAlLabel				= irAlLabel,
		.AnSISOP_llamarConRetorno		= llamarFuncionConRetorno,
		.AnSISOP_retornar				= retornar,
		.AnSISOP_entradaSalida			= entradaSalida,
		.AnSISOP_finalizar				= finalizar
};

AnSISOP_kernel kernel_functions = {
		.AnSISOP_wait		= wait,
		.AnSISOP_signal		= signal_cpu
};



/*
 ============================================================================
 Name        : cargarConf
 Author      : Ezequiel Martinez
 Inputs      : Recibe archivo de configuracion y nombre del archivo.
 Outputs     : N/A
 Description : Funcion para cargar los parametros del archivo de configuración
 ============================================================================
 */
void cargarConf(t_configCPU* config,char* file_name){

	t_config* miConf = config_create ("cpu.conf"); /*Estructura de configuracion*/

	config->salir = 0; //Inicializo OK el flag.

	if (config_has_property(miConf,"NUCLEO_IP")) {
		config->ipNucleo = config_get_string_value(miConf,"NUCLEO_IP");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","NUCLEO_IP");
		config->salir = 1;
		exit(EXIT_FAILURE);
	}

	if (config_has_property(miConf,"PUERTO_NUCLEO")) {
		config->puertoNucleo = config_get_int_value(miConf,"PUERTO_NUCLEO");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_NUCLEO");
		config->salir = 1;
		exit(EXIT_FAILURE);
	}

	if (config_has_property(miConf,"UMC_IP")) {
		config->ipUmc= config_get_string_value(miConf,"UMC_IP");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","UMC_IP");
		config->salir = 1;
		exit(EXIT_FAILURE);
	}

	if (config_has_property(miConf,"PUERTO_UMC")) {
		config->puertoUmc = config_get_int_value(miConf,"PUERTO_UMC");
	} else {
		log_error("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_UMC");
		config->salir = 1;
		exit(EXIT_FAILURE);
	}

}

/*
 =========================================================================================
 Name        : cpuHandShake
 Author      : Ezequiel Martinez
 Inputs      : Recibe el socket, un mensaje para identificarse y un tipo para el header.
 Outputs     : Retorna -1 en caso de error y si no hay error devuelve el socket.
 Description : Funcion para cargar los parametros del archivo de configuración
 =========================================================================================
 */
int cpuHandShake (int socket, int tipoHeader)
{
	stHeaderIPC *headerIPC, *stHeaderIPCQuienSos;  // Hace falta definirlo porque dentro de QUIEN SOS el nuevo header pisa la referencia de stHeaderIPC
	headerIPC = nuevoHeaderIPC(ERROR);             // reserva memoria para recibir el header
	if(!recibirHeaderIPC(socket,headerIPC)){       // stHeaderIPC *stHeaderIPC ,no puede tener el mismo nombre que la estructura
		log_debug("SOCKET_ERROR - No se recibe un mensaje correcto.");
		liberarHeaderIPC(headerIPC);
		fflush(stdout);
		return(-1);
	}
	log_debug("HandShake mensaje recibido %d\n", headerIPC->tipo);

	if (headerIPC->tipo == QUIENSOS)
	{
		stHeaderIPCQuienSos = nuevoHeaderIPC(tipoHeader);
		if(!enviarHeaderIPC(socket,stHeaderIPCQuienSos)){
			log_debug("No se pudo enviar el MensajeIPC.");
			liberarHeaderIPC(stHeaderIPCQuienSos);
			return (-1);
		}
		liberarHeaderIPC(stHeaderIPCQuienSos);
	}
	liberarHeaderIPC(headerIPC);

	headerIPC = nuevoHeaderIPC(ERROR); // Libero primer recibir (al hacer un nuevoHeader pierde la referencia de la memoria allocada antes
	if(!recibirHeaderIPC(socket,headerIPC)){
			log_debug("SOCKET_ERROR - No se recibe un mensaje correcto.");
			fflush(stdout);
			liberarHeaderIPC(headerIPC);
			return (-1);
	}
	log_debug("HandShake: mensaje recibido: ",headerIPC->tipo);
	fflush(stdout);

	if(headerIPC->tipo == OK)
	{
		log_debug("Conexión establecida con id: ",tipoHeader);
		fflush(stdout);
		liberarHeaderIPC(headerIPC);
		return socket;
	}
	liberarHeaderIPC(headerIPC);   // Libera segundo recibir
	return (-1);
}

/*
 =========================================================================================
 Name        : cpuConectarse()
 Author      : Ezequiel Martinez
 Inputs      : Recibe IP y Puerto.
 Outputs     : Retorna -1 en caso de error y si no hay error devuelve el socket.
 Description : Realiza la conexión con un servidor.
 =========================================================================================
 */
int cpuConectarse(char* IP, int puerto, char* aQuien){

	int socket = 0;

	log_debug("Conectando con: %s ",aQuien);
	fflush(stdout);
	socket = conectar(IP, puerto);

	// Inicio el handShake con el servidor //
	if (socket != -1)
	{
		if (cpuHandShake(socket, CONNECTCPU) != -1)
			return socket;
	}

	//Caso de Error//
	configuracionInicial.salir = 1;
	return (-1); // Retorna -1 si no se pudo crear el socket o fallo el handshake

}

/*
 =========================================================================================
 Name        : cerrarSockets()
 Author      : Ezequiel Martinez
 Inputs      : Recibe puntero a estructura.
 Outputs     : Retorna -1 en caso de error y si no hay error devuelve el socket.
 Description : Cierra todos los sockets existentes en la lista.
 =========================================================================================
 */
void cerrarSockets(t_configCPU *configuracionInicial){

	int unSocket;
	for(unSocket=3; unSocket <= configuracionInicial->socketMax; unSocket++)
		if(FD_ISSET(unSocket,&(fds_master)))
			close(unSocket);

	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));
}

/*
 =========================================================================================
 Name        : cargarPCB()
 Author      : Ezequiel Martinez
 Inputs      : N/A.
 Outputs     : Retorna -1 en caso de no poder cargar el PCB.
 Description : Funcion para cargar el PCB del programa ANSISOP.
 =========================================================================================
 */
int cargarPCB(void){

	t_paquete paquete;
	int type;

	recibir_paquete (configuracionInicial.sockNucleo, &paquete);

	type = obtener_paquete_type(&paquete);

	//if (cargarPCB(unMensaje.contenido) != -1)
	if (type == EXECANSISOP)
	{
		log_debug("Comiendo a deserealizar el PCB.");
		unPCB = (stPCB*)malloc(sizeof(stPCB));
		deserializar_pcb(unPCB , &paquete);

		//log_debug("PCB de ANSIPROG cargado. /n");
		free_paquete(&paquete);
		log_debug("Recibi correctamente el PCB del nucleo.");
		solicitudIO = 0;
		return 0;
	}else{
		log_error("Error al recibir el PCB del nucleo.");
		return EXIT_FAILURE;
	}


}

/*
 =========================================================================================
 Name        : calcularPaginaInstruccion()
 Author      : Ezequiel Martinez
 Inputs      : Recibe la dirección logica
 Outputs     : .
 Description : Funcion para obtener
 =========================================================================================
 */
int calcularPaginaInstruccion (int paginaLogica){

	int paginaFisica = 0; //Definimos que el codigo arranca en la pagina 0.
	int i;

	for (i=1; paginaLogica > i;i++)
		paginaFisica++;

	return paginaFisica;
}

/*
 =========================================================================================
 Name        : getInstruccion()
 Author      : Ezequiel Martinez
 Inputs      : Recibe el comienzo de la instruccion y el size.
 Outputs     : Retorna string de la instruccion.
 Description : Funcion para obtener instruccion del progracma ANSISOP en memoria.
 =========================================================================================
 */
char* getInstruccion (int startRequest, int sizeRequest){

	char* instruccion = NULL;
	char* instruccionTemp = NULL;

	stMensajeIPC unMensaje;
	stHeaderIPC *unHeader = NULL;
	uint16_t sizeToUMC = 0;
	uint16_t startToUMC = 0;
	uint16_t sizeRequestLeft = 0;
	int pagina = 0;

	/* Estado inicial */
	int cantidadPaginas = ((startRequest + sizeRequest) / tamanioPaginaUMC) + 1;
	int paginaInicial = startRequest / tamanioPaginaUMC;
	startToUMC = startRequest;
	sizeRequestLeft = sizeRequest;

	pagina=paginaInicial;

	while(pagina<cantidadPaginas && sizeRequestLeft > 0)
	{
		/* Offset */
		startToUMC %= tamanioPaginaUMC;

		/* Size */
		if((sizeRequestLeft / tamanioPaginaUMC) > 0)
			/* Caso pagina completa */
			sizeToUMC = tamanioPaginaUMC;
		else
			/* Caso request completo */
			sizeToUMC = sizeRequestLeft;

		/* Si el offset + size se pasan del tamaño de la pagina pido lo que puedo */
		if((startToUMC+sizeToUMC) > tamanioPaginaUMC)
			sizeToUMC = tamanioPaginaUMC - startToUMC;

		unHeader = nuevoHeaderIPC(READ_BTYES_PAGE);
		unHeader->largo = sizeof(uint16_t)*3;

		if(!enviarHeaderIPC(configuracionInicial.sockUmc,unHeader)){
			log_error("Error al enviar mensaje de leer bytes pagina.");
			return NULL;
		}
		liberarHeaderIPC(unHeader);

		/*Envio los tres datos a la UMC*/
		send(configuracionInicial.sockUmc,&pagina,sizeof(uint16_t),0);
		log_debug("CPU To UMC - Pagina pedida: %d",pagina);
		send(configuracionInicial.sockUmc,&startToUMC,sizeof(uint16_t),0);
		log_debug("CPU To UMC - Offset pedid: %d",startToUMC);
		send(configuracionInicial.sockUmc,&sizeToUMC,sizeof(uint16_t),0);
		log_debug("CPU To UMC - Size pedido: %d",sizeToUMC);

		/*Me quedo esperando que vuelva el contenido*/
		if(!recibirMensajeIPC(configuracionInicial.sockUmc, &unMensaje )){
			log_error("Se cerro la conexion con la UMC.");
			return NULL;
		}

		/* Valido que la respuesta de la UMC tenga el largo correcto y sea OK */
		if(unMensaje.header.tipo != OK || unMensaje.header.largo != sizeToUMC)
		{
			log_error("Error al recibir mensaje de bytes intruccion.");
			return NULL;

		}

		instruccionTemp =(char*) malloc(sizeToUMC + 1 );
		memcpy(instruccionTemp, unMensaje.contenido, sizeToUMC);
		*(instruccionTemp + sizeToUMC) = '\0';

		if (strlen(instruccionTemp) >0 && instruccionTemp != NULL){

			log_debug("Recibi de la UMC la instrucción Temporal: %s",instruccionTemp);

			if(instruccion==NULL){
			instruccion =(char*) malloc(sizeToUMC + 1 );
			strcpy(instruccion, instruccionTemp);

			}else{
				string_append (&instruccion,instruccionTemp);

			}
		}else{
			log_error("Recibi de la UMC la instrucción Temporal nula");
			free(instruccionTemp);
			configuracionInicial.salir = 1;
			return NULL;
		}

		free(instruccionTemp);
		free(unMensaje.contenido);  // Por cada recv hace un malloc

		startToUMC += sizeToUMC;
		sizeRequestLeft -= sizeToUMC;
		pagina++;
	}
	//Imprimi la instrucción solicitada//
	reemplazarBarraN(instruccion);
	log_info("Instruccion a ejecutar: [%s]",instruccion);
	return instruccion;

}

/*
 =========================================================================================
 Name        : ejecutarInstruccion()
 Author      : Ezequiel Martinez
 Inputs      : N/A
 Outputs     : Retorna 1 en caso de haber algun error.
 Description : Ejecuta una instrucción del PCB.
 =========================================================================================
 */
int ejecutarInstruccion(void){
	int programCounter = unPCB->pc;
	char* instruccion = NULL;

	log_info("Ejecutar instruccion Pid[%d]", unPCB->pid);

	instruccion = getInstruccion(unPCB->metadata_program->instrucciones_serializado[programCounter].start,
								 unPCB->metadata_program->instrucciones_serializado[programCounter].offset);

	if (instruccion != NULL){
		unPCB->pc ++; 	/* actualizo el program counter a la siguiente posición */
		analizadorLinea(strdup(instruccion), &AnSISOP_functions, &kernel_functions);
	}else{
		log_error("Error: fallo la ejecución de instrucción.\n");
		configuracionInicial.salir = 1;
		return EXIT_FAILURE;
	}

	if(strcmp(instruccion, "end")==0){
		unPCB->pc = unPCB->metadata_program->instrucciones_size + 1;

	}
	free(instruccion);
	return OK;

}

/*
 =========================================================================================
 Name        : devolverPCBalNucleo()
 Author      : Ezequiel Martinez
 Inputs      : N/A
 Outputs     : Retorna -1 en caso de haber algun error.
 Description : Envia el PCB al Nucleo con la informacion actualizada.
 =========================================================================================
 */
int devolverPCBalNucleo(void){

	stHeaderIPC *unHeaderIPC;
	t_paquete paquete;
	int resultado=  0;

	log_info("PCB PID[%d] Devolviendo PCB al Nucleo.", unPCB->pid);
	log_debug("PCB PID[%d] - Stack Inicio[%ld] Stack offset[%ld]", unPCB->pid,  unPCB->paginaInicioStack, unPCB->offsetStack);

	if (unPCB->metadata_program->instrucciones_size <= unPCB->pc) //Si la cantidad total de instrucciones menor al pc significa que termino el programa.
	{
		log_info("PCB PID[%d] - Fin de Programa.", unPCB->pid);
		log_debug("PCB PID[%d] con fin de programa pc[%d] instrucciones size[%d]", unPCB->pid, unPCB->pc, unPCB->metadata_program->instrucciones_size);
		unHeaderIPC = nuevoHeaderIPC(FINANSISOP);
		if(enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC) <= 0){
			log_error("Error al enviar mensaje de FINANSISOP");
			return -1;
		}
		if(send(configuracionInicial.sockNucleo,&unPCB->pid,sizeof(uint32_t),0)!=sizeof(uint32_t)){
			log_error("Error al enviar el PID");
			return -1;
		}
	}
	else
	{
		if (solicitudIO == 0 ){
			log_info("PCB PID[%d] - Quantum finalizado.", unPCB->pid);
			log_debug("PCB PID[%d] Quantum finalizado. pc[%d] instrucciones size[%d]", unPCB->pid, unPCB->pc, unPCB->metadata_program->instrucciones_size);
			unHeaderIPC = nuevoHeaderIPC(QUANTUMFIN);
			if(enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC)<=0){
				log_error("Error al enviar mensaje de FINQUANTUM");
				return -1;
			}
			liberarHeaderIPC(unHeaderIPC);
		}


		crear_paquete(&paquete, EXECANSISOP);
		serializar_pcb(&paquete, unPCB);

		if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
			log_error("No se pudo enviar el PCB al Nucleo[%d]", unPCB->pid);
			resultado = -1;
		}

		free_paquete(&paquete);
	}

	pcb_destroy(unPCB);
	return resultado;
}

int cambiarContextoUMC(uint32_t pid){

	log_info("PID[%d] Cambio de contexto.", pid);

	stMensajeIPC unMensajeIPC;
	unMensajeIPC.header.tipo = CAMBIOCONTEXTO;
	unMensajeIPC.header.largo = sizeof(uint32_t);
	stHeaderIPC* unHeaderConfirmacion = nuevoHeaderIPC(ERROR);

	if(!enviarMensajeIPC(configuracionInicial.sockUmc,&unMensajeIPC.header, (char*) &pid)){
		log_error("Error al enviar cambio de contexto a la UMC..");
		return EXIT_FAILURE;
	}

	if (!recibirHeaderIPC(configuracionInicial.sockUmc, unHeaderConfirmacion)) {
		log_error("Error al recibir confirmacion de cambio de contexto de la UMC");
		return EXIT_FAILURE;
	}

	if(OK != unHeaderConfirmacion->tipo){
		/* El pid es el enviado el CPU y lo confirma en el largo del header
		 *  para no usar un enviarMensaje */
		log_debug("La UMC rechaza atender el pid[%d] por no tener marcos disponibles y ser un proceso nuevo", pid);
		return EXIT_FAILURE;
	}

	log_debug("La UMC acepto atender el pid[%d]", unHeaderConfirmacion->largo);
	liberarHeaderIPC(unHeaderConfirmacion);
	return EXIT_SUCCESS;
}

/*
 =========================================================================================
 Name        : main()
 Author      : Ezequiel Martinez
 Inputs      : N/A.
 Outputs     : N/A.
 Description : Proceso principal del programa.
 =========================================================================================
 */
int main(int argc, char *argv[]) {

	stHeaderIPC *unHeaderIPC;
	t_UMCConfig *configUMC;
	int unSocket;
	int quantumSleep=0;
	char* temp_file = "cpu.log";
	int log_level = LOG_LEVEL_INFO;

	// Nivel de logueo por parametro//
		if(argv[1]){
			if(strcmp(argv[1], "debug")==0)
				log_level=LOG_LEVEL_DEBUG;
		}

	//Primero instancio el log
	t_log* logger = log_create(temp_file, "CPU",-1, log_level);

	log_info("Iniciando el proceso CPU..."); /* prints CPU Application */

	init_signal_handler(&configuracionInicial);

	// Limpio las listas //
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/***** Cargo la configuracion desde el archivo cpu.conf ********/
	log_info("Cargando configuracion de CPU.");

	cargarConf(&configuracionInicial, CFGFILE);

	log_info("Nucleo IP[%s] Puerto[%d]", configuracionInicial.ipNucleo, configuracionInicial.puertoNucleo);
	log_info("UMC IP[%s] Puerto[%d]", configuracionInicial.ipUmc, configuracionInicial.puertoUmc);
	log_debug("Quantum[%d]", configuracionInicial.quantum);

	/***** Lanzo conexión con el Nucleo ********/

	log_info("Conectando al Nucleo...");

	configuracionInicial.sockNucleo = cpuConectarse(configuracionInicial.ipNucleo, configuracionInicial.puertoNucleo, "Nucleo");

	if (configuracionInicial.salir == 0 && configuracionInicial.sockNucleo != -1){
		FD_SET(configuracionInicial.sockNucleo,&(fds_master));
		configuracionInicial.socketMax = configuracionInicial.sockNucleo;
		SocketAnterior = configuracionInicial.socketMax;
		log_debug("OK - Nucleo conectado.");
		fflush(stdout);
		log_info("Nucleo conectado.");

	}	//Fin de conexion al Nucleo//


	/***** Lanzo conexión con el UMC ********/

	if (configuracionInicial.salir == 0){

		log_info("Conectando al UMC...");

		configuracionInicial.sockUmc = cpuConectarse(configuracionInicial.ipUmc, configuracionInicial.puertoUmc, "UMC");

		if (configuracionInicial.sockUmc > 0){

			FD_SET(configuracionInicial.sockUmc,&(fds_master));
			configuracionInicial.socketMax = configuracionInicial.sockUmc;
			SocketAnterior = configuracionInicial.socketMax;
			log_info("OK - UMC conectada.");
			fflush(stdout);

		}

		//Recibo tamanio de pagina UMC//

		configUMC = malloc(sizeof(t_UMCConfig));

		if(recibirConfigUMC(configuracionInicial.sockUmc, configUMC )!=0){

			log_debug("Error al recibir paginas de UMC.");
			configuracionInicial.salir = 1;
		}

		log_debug("Recibiendo de la UMC el tamaño de pagina.");

		tamanioPaginaUMC = configUMC->tamanioPagina; //Guardo el tamaño de la pagina de la umc.

		log_info("Recibí tamaño de pagina: %d",tamanioPaginaUMC);

		//Fin de conexion al UMC//

	}

	while(configuracionInicial.salir == 0)
	{
		read_fds = fds_master;
		if(seleccionar(configuracionInicial.socketMax,&read_fds,1) == -1)
		{
			log_debug("Error Preparando el Select con CPU.\n");
			configuracionInicial.salir = 1;
		}

		for(unSocket=0;unSocket<=configuracionInicial.socketMax;unSocket++)
		{
			if(FD_ISSET(unSocket,&read_fds))
			{
				unHeaderIPC = nuevoHeaderIPC(ERROR);
				if (!recibirHeaderIPC(unSocket,unHeaderIPC))/* Si se cerro un Cliente. */
				{
					if (configuracionInicial.sockNucleo == unSocket)
					{
						log_debug("Se desconecto el Servidor Nucleo.\n");
						fflush(stdout);
						configuracionInicial.sockNucleo = -1;
						configuracionInicial.salir=1;

						FD_CLR(unSocket,&fds_master);
						close (unSocket);

						if (unSocket > configuracionInicial.socketMax){
							SocketAnterior = configuracionInicial.socketMax;
							configuracionInicial.socketMax = unSocket;
						}

						log_error("Se perdio conexion con Nucleo.");

					}else if (configuracionInicial.sockUmc == unSocket)
					{
						configuracionInicial.sockUmc = -1;
						configuracionInicial.salir = 1;

						FD_CLR(unSocket,&fds_master);
						close (unSocket);

						if (unSocket > configuracionInicial.socketMax){
							SocketAnterior = configuracionInicial.socketMax;
							configuracionInicial.socketMax = unSocket;
						}

						log_error("Se desconecto el UMC.\n");
						fflush(stdout);
					}
				}
				else
				{
					switch(unHeaderIPC->tipo)
					{
						case EXECANSISOP:

							log_info("Respondiendo solicitud ANSISOP...");

							unHeaderIPC = nuevoHeaderIPC(OK);
							enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

							if (cargarPCB() != -1)
							{
								//Cambio el contexto con la UMC
								if(cambiarContextoUMC(unPCB->pid)!=EXIT_SUCCESS){
									log_debug("Devuelvo PCB al nucleo para que sea atendido en otro momento");
									if (devolverPCBalNucleo() == -1){
										log_debug("Error al devolver PCB de ANSIPROG...");
										configuracionInicial.salir = 1;
										break;
									}
									break;
								}

								/* Obtengo el quantum del programa */
								quantum = unPCB->quantum;
								log_info("Quantum a ejecutar [%d].", quantum );

								if (quantum <= 0){
									log_error("Error en Quantum definido.");
									break;
								}

								quantumSleep = unPCB->quantumSleep;

								//Ejecuto las instrucciones defidas por quamtum

								while (quantum > 0 && unPCB->pc <= unPCB->metadata_program->instrucciones_size){
									if(ejecutarInstruccion() == OK)
									{
										log_info("Quantum sleep [%d].", quantumSleep );
										usleep(quantumSleep*1000);
										quantum --; 	/* descuento un quantum para proxima ejecución */

										//Solo imprimo el quantum restante cuando no sea por una salida controlada//
										if(quantum > 0)
											log_info("Quantum a ejecutar [%d].", quantum );

									}else{
										log_error("Error al ejecutar la instrucción.");
										configuracionInicial.salir =1;
										quantum = 0;
									}

								}
								//Si no hubo error devuelvo PCB al nucleo//
								if (devolverPCBalNucleo() == -1){

									log_debug("Error al devolver PCB de ANSIPROG...");
									configuracionInicial.salir = 1;
									break;
								}
								log_info("Se devolvio PCB al nucleo.");

							}else
								log_debug("Error en lectura ANSIPROG...");

							log_info("AnSISOP fin de Ejecucion por Quantum");
							break;

						default:
							log_debug("Otra fruta");
							break;

						}  // Switch
				} // else

			} // if FD_isset

		} // for socket max
	} // while salir

	if (configuracionInicial.sockNucleo != -1){
		//Informo al Nucleo que estoy terminando
		unHeaderIPC = nuevoHeaderIPC(SIGUSR1CPU);
		enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);
		liberarHeaderIPC(unHeaderIPC);
	}

	cerrarSockets(&configuracionInicial);
	log_info("Se finaliza el proceso CPU");
	log_destroy(logger);
	return EXIT_SUCCESS;
}
