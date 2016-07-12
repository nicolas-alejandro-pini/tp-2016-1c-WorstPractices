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
t_puntero ultimaPosicionStack = 0;
t_configCPU configuracionInicial; /* Estructura del CPU, contiene los sockets de conexion y parametros. */

stPCB* unPCB; /* Estructura del pcb para ejecutar las instrucciones */

/* EML: Lo comento xq no lo uso por ahora
int mensajeToUMC(int tipoHeader, stPosicion* posicionVariable){

	stHeaderIPC* unHeader;
	t_paquete paquetePosicion;
	int resultado = 0;
	int offset = 0;

	unHeader = nuevoHeaderIPC(tipoHeader);

	enviarHeaderIPC(configuracionInicial.sockUmc, unHeader);

	crear_paquete(&paquetePosicion, tipoHeader);

	serializar_campo(&paquetePosicion, &offset, posicionVariable, sizeof(stPosicion));

	serializar_header(&paquetePosicion);

	if (enviar_paquete(configuracionInicial.sockUmc, &paquetePosicion)) {
		log_error("No se pudo enviar al UMC el paquete para operacion [%d]", tipoHeader);
		resultado = -1;
	}

	free_paquete(&paquetePosicion);

	liberarHeaderIPC(unHeader);

	return resultado;

}
*/

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

	tamanioStack=list_size(unPCB->stack) - 1 ;

	indiceStack = list_get(unPCB->stack, tamanioStack);

	unaVariable = (stVars*)malloc(sizeof(stVars));
	unaVariable->id = identificador_variable;
	unaVariable->posicion_memoria.pagina = 0;
	unaVariable->posicion_memoria.offset = unPCB->offsetStack;
	unaVariable->posicion_memoria.size = TAMANIOVARIABLES;
	list_add(indiceStack->variables,unaVariable);

	indiceStack->pos = tamanioStack + 1;

	list_add(unPCB->stack,indiceStack);

	unPCB->offsetStack = unPCB->offsetStack + TAMANIOVARIABLES;

	return unPCB->offsetStack;
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable ){

	stVars *unaVariable = NULL;
	unaVariable = malloc(sizeof(stVars));

	stIndiceStack *indiceStack;
	indiceStack = malloc(sizeof(stIndiceStack));

	int tamanioStack;

	tamanioStack = list_size(unPCB->stack); //considero que en pila del stack el ultimo es el contexto actual.

	indiceStack = list_get(unPCB->stack, tamanioStack);

	int _es_la_var(stVars *var) {
			return var->id == identificador_variable;
		}

	unaVariable = list_find(indiceStack->variables, (void*) _es_la_var);

	if (unaVariable !=NULL)
		return unaVariable->posicion_memoria.offset;
	else
		return (-1);
}

stPosicion *obtenerPosicion(t_puntero direccion_variable){
	stPosicion *unaPosicion;
	int pagina, offset;

	unaPosicion = (stPosicion*)malloc(sizeof(stPosicion));
	pagina = (direccion_variable/tamanioPaginaUMC);

	offset = (tamanioPaginaUMC * pagina) - direccion_variable;
	unaPosicion->size = TAMANIOVARIABLES;
	unaPosicion->pagina = pagina + unPCB->paginaInicioStack;
	unaPosicion->offset= offset;
	return unaPosicion;
}

t_valor_variable dereferenciar(t_puntero direccion_variable){

	stPosicion *posicionVariable;
	stMensajeIPC unMensaje;
	stHeaderIPC *unHeader;
	uint16_t pagina,offset,tamanio,valor_variable;

	posicionVariable = obtenerPosicion(direccion_variable);

	pagina = posicionVariable->pagina;
	offset = posicionVariable->offset;
	tamanio = posicionVariable->size;

	unHeader = nuevoHeaderIPC(READ_BTYES_PAGE);
	unHeader->largo = sizeof(uint16_t);

	if(!enviarHeaderIPC(configuracionInicial.sockUmc,unHeader)){
		log_error("Error al enviar mensaje de leer bytes intruccion.");
	}

	/*Envio los tres datos a la UMC*/
	send(configuracionInicial.sockUmc,&pagina,sizeof(uint16_t),0);
	send(configuracionInicial.sockUmc,&offset,sizeof(uint16_t),0);
	send(configuracionInicial.sockUmc,&tamanio,sizeof(uint16_t),0);


	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&unMensaje)){
		log_error("No se pudo recibir la variable.");
	}

	if(unMensaje.header.tipo == OK)
		valor_variable = atoi((char*)unMensaje.contenido);
	else{
		imprimirTexto("Error de segmentación al dereferenciar.");
		log_error ("Error de segmentacion");
		configuracionInicial.salir = 1;
		quantum = 0;
		liberarHeaderIPC(unHeader);
		return (-1);
	}

	liberarHeaderIPC(unHeader);

	return valor_variable;


}

void asignar(t_puntero direccion_variable, t_valor_variable valor ){

	stPosicion *posicionVariable;
	stHeaderIPC *unHeader;
	uint16_t pagina, offset,tamanio;

	posicionVariable = obtenerPosicion(direccion_variable);

	pagina = posicionVariable->pagina;
	offset = posicionVariable->offset;
	tamanio = posicionVariable->size;

	unHeader = nuevoHeaderIPC(WRITE_BYTES_PAGE);
	unHeader->largo = sizeof(uint16_t);

	if(!enviarHeaderIPC(configuracionInicial.sockUmc,unHeader)){
		log_error("Error al enviar mensaje de leer bytes intruccion.");
	}

	/*Envio los tres datos a la UMC*/
	send(configuracionInicial.sockUmc,&pagina,sizeof(uint16_t),0);
	send(configuracionInicial.sockUmc,&offset,sizeof(uint16_t),0);
	send(configuracionInicial.sockUmc,&tamanio,sizeof(uint16_t),0);
	send(configuracionInicial.sockUmc,&valor,sizeof(uint16_t),0);

	unHeader = nuevoHeaderIPC(ERROR);
	if(!recibirHeaderIPC(configuracionInicial.sockUmc, unHeader)){

		log_error("Error al recibir confirmacion de asignar en UMC.");
	}

	if(unHeader->tipo == OK)
			log_info("Se asigno correctamente.");
		else{
			imprimirTexto("Error de segmentación al asignar.");
			log_error ("Error de segmentacion al asignar");
			configuracionInicial.salir = 1;
			quantum = 0;
			liberarHeaderIPC(unHeader);

		}
	liberarHeaderIPC(unHeader);

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){

	stHeaderIPC *unHeaderIPC;
	stMensajeIPC unMensajeIPC;
	t_valor_variable resultado;

	unHeaderIPC = nuevoHeaderIPC(OBTENERVALOR);
	unHeaderIPC = strlen(variable)+1;
	if(!enviarMensajeIPC(configuracionInicial.sockNucleo,unHeaderIPC,(char)variable)){
		printf("No se pudo enviar la variable %s",(char)variable);
	}

	if(!recibirMensajeIPC(configuracionInicial.sockNucleo,&unMensajeIPC)){
		printf("No se pudo recibir la variable %s",(char*)variable);
	}

	resultado = atoi(unMensajeIPC.contenido);

	return resultado;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){

	stHeaderIPC *unHeaderIPC;
	t_paquete paquete;
	stSharedVar sharedVar;
	t_valor_variable resultado;
	int offset = 0;

	unHeaderIPC = nuevoHeaderIPC(GRABARVALOR);
	if(!recibirHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC)){
		printf("No se pudo enviar el mensaje para grabar el valor");
	}

	if(unHeaderIPC->tipo== OK){

		sharedVar.nombre = &variable;
		sharedVar.valor = valor;

		crear_paquete(&paquete, GRABARVALOR);
		serializar_campo(&paquete, &offset, &sharedVar, sizeof(stSharedVar));
		serializar_header(&paquete);

		if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
			log_error("No se pudo enviar el SharedVar al Nucleo.");
		}

		free_paquete(&paquete);

		liberarHeaderIPC(unHeaderIPC);
	}else{
		/*TODO: Esto ver de manejarlo, tenemos que hacer validaciones en caso de que no se puedan enviar los mensajes*/
		return -1;
	}

	return resultado;
}

void irAlLabel(t_nombre_etiqueta etiqueta){
	t_puntero_instruccion ptr_instruccion;
	ptr_instruccion = metadata_buscar_etiqueta(etiqueta,unPCB->metadata_program->etiquetas,unPCB->metadata_program->etiquetas_size);
	unPCB->pc = ptr_instruccion;
}

void llamarFuncionConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	/*Creamos una nuevo indice de stack correspondiente al nuevo call de la funcion*/
	/*Buscamos la etiqueta definida en el metadata, para actualizar el program counter del PCB*/
	printf("Llamada a llamarFuncionConRetorno\n");
	stIndiceStack *unIndiceStack;
	irAlLabel(etiqueta);
	unIndiceStack = (stIndiceStack*) malloc(sizeof(stIndiceStack));
	unIndiceStack->argumentos = list_create();
	unIndiceStack->pos = list_size(unPCB->stack) + 1;
	unIndiceStack->variables = list_create();
	unIndiceStack->retPosicion = (uint32_t)donde_retornar;
	list_add(unPCB->stack,unIndiceStack);
}
void retornar(t_valor_variable retorno){
	printf("Llamada a funcion retornar\n");
	stIndiceStack *unIndiceStack;
	/*Sacamos del stack la variable a retornar*/
	unIndiceStack = list_remove(unPCB->stack,list_size(unPCB->stack));
	/*Actualizamos el program counter del pcb*/
	unPCB->pc = unIndiceStack->retPosicion;
	asignar(unIndiceStack->retVar.offset,retorno);
}

void imprimir(t_valor_variable valor_mostrar){

	stHeaderIPC* unHeaderPrimitiva;

	unHeaderPrimitiva = nuevoHeaderIPC(IMPRIMIR);
	unHeaderPrimitiva->largo = sizeof(t_valor_variable);

	if(!enviarMensajeIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva, (char*)valor_mostrar))
		log_error("Error al enviar mensaje de IMPRIMIR.");

	unHeaderPrimitiva = nuevoHeaderIPC(CONSOLA);
	unHeaderPrimitiva = sizeof(uint32_t);

	if(!enviarMensajeIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva, (char*)unPCB->socketConsola))
			log_error("Error al enviar mensaje de IMPRIMIR.");

	unHeaderPrimitiva = nuevoHeaderIPC(ERROR);

	if(!recibirHeaderIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva)){
		log_error("Error al recibir la confirmacion del nucleo.");
	}

	if(unHeaderPrimitiva->tipo != OK)
		log_error("Error de primitiva IMPRIMIR");

	log_info("Se imprimio correctamente la variable.");

	liberarHeaderIPC(unHeaderPrimitiva);

}

void imprimirTexto(char* texto){

	stHeaderIPC* unHeaderPrimitiva;

	unHeaderPrimitiva = nuevoHeaderIPC(IMPRIMIRTEXTO);
	unHeaderPrimitiva->largo = strlen(texto) + 1;

	if(!enviarMensajeIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva,texto))
			log_error("No se pudo enviar al Nucleo el texto: ",texto);


	unHeaderPrimitiva = nuevoHeaderIPC(CONSOLA);
		unHeaderPrimitiva = sizeof(uint32_t);

	if(!enviarMensajeIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva, (char*)unPCB->socketConsola))
			log_error("Error al enviar mensaje de IMPRIMIR.");

	unHeaderPrimitiva = nuevoHeaderIPC(ERROR);

	if(!recibirHeaderIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva)){
		log_error("Error al recibir la confirmacion del nucleo.");
	}

	if(unHeaderPrimitiva->tipo != OK)
		log_error("Error de primitiva IMPRIMIR");

	log_info("Se imprimio correctamente la variable.");

	liberarHeaderIPC(unHeaderPrimitiva);

}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	stHeaderIPC* unHeaderPrimitiva;
	t_paquete paquete;
	stIO dispositivoIO;
	int offset = 0;

	dispositivoIO.nombre = dispositivo;
	dispositivoIO.tiempo = tiempo;

	unHeaderPrimitiva = nuevoHeaderIPC(IOANSISOP);

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva);

	crear_paquete(&paquete, IOANSISOP);

	serializar_campo(&paquete, &offset, &dispositivoIO.nombre, sizeof(dispositivoIO.nombre));
	serializar_campo(&paquete, &offset, &dispositivoIO.tiempo, sizeof(dispositivoIO.tiempo));

	serializar_header(&paquete);

	if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
		log_error("No se pudo enviar el paquete para primitiva IO");
	}

	free_paquete(&paquete);

	liberarHeaderIPC(unHeaderPrimitiva);

	quantum = 0;

}

void wait(t_nombre_semaforo identificador_semaforo){

	stHeaderIPC* unHeaderPrimitiva;
	t_paquete paquete;

	int offset = 0;

	unHeaderPrimitiva = nuevoHeaderIPC(WAIT);

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva);

	crear_paquete(&paquete, WAIT);

	serializar_campo(&paquete, &offset, &identificador_semaforo, sizeof(identificador_semaforo));

	serializar_header(&paquete);

	if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
		log_error("No se pudo enviar el paquete para primitiva WAIT");
	}

	free_paquete(&paquete);
	liberarHeaderIPC(unHeaderPrimitiva);
}

void signal_cpu(t_nombre_semaforo identificador_semaforo){

	stHeaderIPC* unHeaderPrimitiva;
	t_paquete paquete;

	int offset = 0;

	unHeaderPrimitiva = nuevoHeaderIPC(SIGNAL);

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva);

	crear_paquete(&paquete, SIGNAL);

	serializar_campo(&paquete, &offset, &identificador_semaforo, sizeof(identificador_semaforo));

	serializar_header(&paquete);

	if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
		log_error("No se pudo enviar el paquete para primitiva WAIT");
	}

	free_paquete(&paquete);

	liberarHeaderIPC(unHeaderPrimitiva);
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
};

AnSISOP_kernel kernel_functions = {
		.AnSISOP_signal		= wait,
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

		if (config_has_property(miConf,"NUCLEO_IP")) {
			config->ipNucleo = config_get_string_value(miConf,"NUCLEO_IP");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","NUCLEO_IP");
			exit(-2);
		}

		if (config_has_property(miConf,"PUERTO_NUCLEO")) {
			config->puertoNucleo = config_get_int_value(miConf,"PUERTO_NUCLEO");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_NUCLEO");
			exit(-2);
		}

		if (config_has_property(miConf,"UMC_IP")) {
			config->ipUmc= config_get_string_value(miConf,"UMC_IP");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","UMC_IP");
			exit(-2);
		}

		if (config_has_property(miConf,"PUERTO_UMC")) {
			config->puertoUmc = config_get_int_value(miConf,"PUERTO_UMC");
		} else {
			printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n","PUERTO_UMC");
			exit(-2);
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
		log_info("SOCKET_ERROR - No se recibe un mensaje correcto.");
		liberarHeaderIPC(headerIPC);
		fflush(stdout);
		return(-1);
	}
	printf("HandShake mensaje recibido %ld\n", headerIPC->tipo);

	if (headerIPC->tipo == QUIENSOS)
	{
		stHeaderIPCQuienSos = nuevoHeaderIPC(tipoHeader);
		if(!enviarHeaderIPC(socket,stHeaderIPCQuienSos)){
			log_info("No se pudo enviar el MensajeIPC.");
			liberarHeaderIPC(stHeaderIPCQuienSos);
			return (-1);
		}
		liberarHeaderIPC(stHeaderIPCQuienSos);
	}
	liberarHeaderIPC(headerIPC);

	headerIPC = nuevoHeaderIPC(ERROR); // Libero primer recibir (al hacer un nuevoHeader pierde la referencia de la memoria allocada antes
	if(!recibirHeaderIPC(socket,headerIPC)){
			log_info("SOCKET_ERROR - No se recibe un mensaje correcto.");
			fflush(stdout);
			liberarHeaderIPC(headerIPC);
			return (-1);
	}
	log_info("HandShake: mensaje recibido: ",headerIPC->tipo);
	fflush(stdout);

	if(headerIPC->tipo == OK)
	{
		log_info("Conexión establecida con id: ",tipoHeader);
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

	log_info("Conectando con: ",aQuien);
	fflush(stdout);
	socket = conectar(IP, puerto);

	// Inicio el handShake con el servidor //
	if (socket != -1)
	{
		if (cpuHandShake(socket, CONNECTCPU) != -1)
			return socket;
	}

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
	stVars *unaVar;
	stIndiceStack *stack;
	stPosicion *argumento;

	recibir_paquete (configuracionInicial.sockNucleo, &paquete);

	type = obtener_paquete_type(&paquete);

	//if (cargarPCB(unMensaje.contenido) != -1)
	if (type == EXECANSISOP)
	{
		log_info("Comiendo a deserealizar el PCB.");
		unPCB = (stPCB*)malloc(sizeof(stPCB));
		deserializar_pcb(unPCB , &paquete);

		//log_info("PCB de ANSIPROG cargado. /n");
		free_paquete(&paquete);
		log_info("Recibi correctamente el PCB del nucleo.");
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

	stMensajeIPC unMensaje;
	stHeaderIPC *unHeader = NULL;

	uint16_t paginaToUMC;
	uint16_t startToUMC = startRequest;
	uint16_t sizeToUMC = sizeRequest;
	int sizeAux;
	int pagina;

	int cantidadPaginas = ((startRequest + sizeRequest) / tamanioPaginaUMC) + 1;

	for (pagina=1;pagina<=cantidadPaginas;pagina++)
	{

		if (startToUMC <= (pagina * tamanioPaginaUMC)) //Si la posicion de offset no se encuentra en la pagina paso a la siguiente.
		{
			sizeAux = (pagina*tamanioPaginaUMC) - (startRequest + sizeRequest);

			if (sizeAux < 0)
				sizeToUMC = pagina*tamanioPaginaUMC - startToUMC;


			paginaToUMC = calcularPaginaInstruccion(pagina);

			unHeader = nuevoHeaderIPC(READ_BTYES_PAGE);
			unHeader->largo = sizeof(uint16_t);

			if(!enviarHeaderIPC(configuracionInicial.sockUmc,unHeader)){
				log_error("Error al enviar mensaje de leer bytes pagina.");
			}

			/*Envio los tres datos a la UMC*/
			send(configuracionInicial.sockUmc,&paginaToUMC,sizeof(uint16_t),0);
			log_info("CPU To UMC - Pagina pedida: ",(char*)paginaToUMC);
			send(configuracionInicial.sockUmc,&startToUMC,sizeof(uint16_t),0);
			log_info("CPU To UMC - Offset pedid: ",(char*)startToUMC);
			send(configuracionInicial.sockUmc,&sizeToUMC,sizeof(uint16_t),0);
			log_info("CPU To UMC - Size pedido: ",(char*)sizeToUMC);

			/*Me quedo esperando que vuelva el contenido*/
			if(!recibirMensajeIPC(configuracionInicial.sockUmc, &unMensaje )){
				log_error("Error al recibir mensaje de bytes intruccion.");
			}else{

				log_info("Recibi de la UMC la instrucción.",unMensaje.contenido);

				if(instruccion==NULL){
					instruccion = malloc(sizeof(char)*(strlen(unMensaje.contenido) + 1 ));
					strcpy(instruccion, unMensaje.contenido);

				}else{
					string_append (&instruccion,unMensaje.contenido);
				}

				startToUMC = startToUMC + sizeToUMC;
				sizeToUMC = sizeRequest - sizeToUMC;
			}

		}

	}
	liberarHeaderIPC(unHeader);
	//Imprimi la instrucción solicitada//
	log_info(instruccion);
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

	instruccion = getInstruccion(unPCB->metadata_program->instrucciones_serializado[programCounter].start,
								 unPCB->metadata_program->instrucciones_serializado[programCounter].offset);

	if (instruccion != NULL){
		analizadorLinea(strdup(instruccion), &AnSISOP_functions, &kernel_functions);
	}else{
		printf("Error: fallo la ejecución de instrucción.\n");
		return EXIT_FAILURE;
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

	if (unPCB->metadata_program->instrucciones_size < unPCB->pc) //Si la cantidad total de instrucciones menor al pc significa que termino el programa.
		unHeaderIPC = nuevoHeaderIPC(FINANSISOP);
	else
		unHeaderIPC = nuevoHeaderIPC(QUANTUMFIN);

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

	crear_paquete(&paquete, EXECANSISOP);
	serializar_pcb(&paquete, unPCB);

	if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
		log_error("No se pudo enviar el PCB al Nucleo[%d]", unPCB->pid);
		resultado = -1;
	}

	free_paquete(&paquete);

	liberarHeaderIPC(unHeaderIPC);

	return resultado;
}

int cambiarContextoUMC(int pid){

	stMensajeIPC unMensajeIPC;
	unMensajeIPC.header.tipo = CAMBIOCONTEXTO;
	unMensajeIPC.header.largo = sizeof(pid);

	if(!enviarMensajeIPC(configuracionInicial.sockUmc,&unMensajeIPC.header, (char*) &pid))
		return EXIT_FAILURE;

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
int main(void) {

	stHeaderIPC *unHeaderIPC;
	t_UMCConfig *configUMC;
	int unSocket;
	int quantumSleep=0;
	char* temp_file = "cpu.log";

	//Primero instancio el log
	t_log* logger = log_create(temp_file, "CPU",-1, LOG_LEVEL_INFO);

	log_info("Iniciando el proceo CPU..."); /* prints CPU Application */

	init_signal_handler(&configuracionInicial);

	// Limpio las listas //
	FD_ZERO(&(fds_master));
	FD_ZERO(&(read_fds));

	/***** Cargo la configuracion desde el archivo cpu.conf ********/
	log_info("Cargando configuracion de CPU.");

	cargarConf(&configuracionInicial, CFGFILE);

	log_info("Configuración OK.");

	/***** Lanzo conexión con el Nucleo ********/

	log_info("Conectando al Nucleo...");

	configuracionInicial.sockNucleo = cpuConectarse(configuracionInicial.ipNucleo, configuracionInicial.puertoNucleo, "Nucleo");

	if (configuracionInicial.sockNucleo != -1){
		FD_SET(configuracionInicial.sockNucleo,&(fds_master));
		configuracionInicial.socketMax = configuracionInicial.sockNucleo;
		SocketAnterior = configuracionInicial.socketMax;
		log_info("OK - Nucleo conectado.");
		fflush(stdout);
		log_info("Nucleo conectado.");

	}	//Fin de conexion al Nucleo//


	/***** Lanzo conexión con el UMC ********/

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

		log_info("Error al recibir paginas de UMC.");
		configuracionInicial.salir = 1;
	}

	log_info("Recibiendo de la UMC el tamaño de pagina.");

	tamanioPaginaUMC = configUMC->tamanioPagina; //Guardo el tamaño de la pagina de la umc.

	log_info("Recibí tamaño de pagina.",tamanioPaginaUMC);

	//Fin de conexion al UMC//

	while(configuracionInicial.salir == 0)
	{
		read_fds = fds_master;
		if(seleccionar(configuracionInicial.socketMax,&read_fds,1) == -1)
		{
			log_info("Error Preparando el Select con CPU.\n");
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
						log_info("Se desconecto el Servidor Nucleo.\n");
						fflush(stdout);
						configuracionInicial.sockNucleo = -1;
						configuracionInicial.salir=1;

						FD_CLR(unSocket,&fds_master);
						close (unSocket);

						if (unSocket > configuracionInicial.socketMax){
							SocketAnterior = configuracionInicial.socketMax;
							configuracionInicial.socketMax = unSocket;
						}

						log_info("Se perdio conexion con Nucleo.");

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

						log_info("Se desconecto el UMC.\n");
						fflush(stdout);
					}
				}
				else
				{
					switch(unHeaderIPC->tipo)
					{
						case EXECANSISOP:

							log_info("Respondiendo solicitud ANSIPROG...");

							unHeaderIPC = nuevoHeaderIPC(OK);
							enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

							if (cargarPCB() != -1)
							{
								//Cambio el contexto con la UMC
								if(cambiarContextoUMC(unPCB->pid)!=EXIT_SUCCESS){
									log_info("Error al cambiar de contexto...");
									break;
								}

								/* Obtengo el quantum del programa */
								quantum = unPCB->quantum;

								if (quantum <= 0){
									printf("Error en Quantum definido. /n");
									break;
								}

								quantumSleep = unPCB->quantumSleep;

								//Ejecuto las instrucciones defidas por quamtum

								while (quantum > 0 && unPCB->pc <= unPCB->metadata_program->instrucciones_size){
									if(ejecutarInstruccion() == OK)
									{
										sleep(quantumSleep);
										quantum --; 	/* descuento un quantum para proxima ejecución */
										unPCB->pc ++; 	/* actualizo el program counter a la siguiente posición */

									}

								}

								if (devolverPCBalNucleo() == -1){

									log_info("Error al devolver PCB de ANSIPROG...");
									configuracionInicial.salir = 1;
									break;

								}

							}else
								log_info("Error en lectura ANSIPROG...");

							printf("AnSISOP fin de Ejecucion por Quantum");
							break;


/*						case SIGUSR1:

							log_info("Respondiendo solicitud SIGUSR1...");

							unHeaderIPC = nuevoHeaderIPC(SIGUSR1CPU);
							 Notifico al nucleo mi desconexion
							enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

							configuracionInicial.salir = 1;
							break;*/

					}
				}

			}

		}
	}

	//Informo al Nucleo que estoy terminando
	unHeaderIPC = nuevoHeaderIPC(SIGUSR1CPU);
	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);
	liberarHeaderIPC(unHeaderIPC);

	cerrarSockets(&configuracionInicial);
	log_info("CPU: Fin del programa");
	log_destroy(logger);
	return EXIT_SUCCESS;
}
