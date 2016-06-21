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


//Variables Globales//

fd_set fds_master;		/* Lista de todos mis sockets. */
fd_set read_fds;		/* Sublista de fds_master. */

int SocketAnterior = 0;
int tamanioPaginaUCM ;
t_puntero ultimaPosicionStack = 0;
t_configCPU configuracionInicial; /* Estructura del CPU, contiene los sockets de conexion y parametros. */

stPCB* unPCB; /* Estructura del pcb para ejecutar las instrucciones */


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

	tamanioStack=list_size(unPCB->stack);

	indiceStack->variables = (t_list*)malloc(sizeof(t_list)+sizeof(stVars));
	indiceStack->variables = list_create();
	unaVariable = (stVars*)malloc(sizeof(stVars));
	unaVariable->id = identificador_variable;
	//unaVariable->posicion_memoria = (stPosicion*)malloc(sizeof(stPosicion));
	unaVariable->posicion_memoria.pagina=0;
	unaVariable->posicion_memoria.offset= ultimaPosicionStack;
	unaVariable->posicion_memoria.size = TAMANIOVARIABLES;
	list_add(indiceStack->variables,unaVariable);

	indiceStack->pos = tamanioStack + 1;

	list_add(unPCB->stack,indiceStack);

	ultimaPosicionStack = ultimaPosicionStack + TAMANIOVARIABLES;

	return ultimaPosicionStack;
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable ){

	stMensajeIPC mensajePrimitiva;
	t_puntero posicionVariable;

	enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(POSICIONVARIABLE),identificador_variable);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajePrimitiva)){
		printf("Error: Falló la obtencion de posicion de la variable %s.\n", identificador_variable);
		return posicionVariable;
	}

	if (mensajePrimitiva.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

	}

	//free(mensajePrimitiva);
	return posicionVariable;

}


t_valor_variable dereferenciar(t_puntero direccion_variable){

	stMensajeIPC mensajePrimitiva;
	t_valor_variable valor;

	/*TODO Serializar el mensaje de estructura */
	char* estructuraSerializada;

	enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(VALORVARIABLE),estructuraSerializada);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajePrimitiva)){
		printf("Error: Fallo en deferenciar variable.\n");
		return NULL;
	}

	if (mensajePrimitiva.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

	}

	//free(mensajePrimitiva);
	return valor;


}

void asignar(t_puntero direccion_variable, t_valor_variable valor ){

	stMensajeIPC mensajePrimitiva;

	//enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(ASIGNARVARIABLE),estructuraSerializada);

	if(!recibirMensajeIPC(configuracionInicial.sockUmc,&mensajePrimitiva)){
		printf("Error: Fallo en asignacion de la variable.\n");

	}

	if (mensajePrimitiva.header.tipo == OK) {

		/*TODO Deserializar el mensaje*/

	}

	//free(mensajePrimitiva); /*TODO Arreglar el free de las estructuras*/

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){

	stHeaderIPC *unHeaderIPC;
	stMensajeIPC unMensaje;
	t_paquete paquete;
	stSharedVar* sharedVar;
	int type, offset=0;

	t_valor_variable resultado;

	unHeaderIPC = nuevoHeaderIPC(OBTENERVALOR);

	unMensaje.contenido = &variable;

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

	sharedVar->nombre = variable;
	sharedVar->valor = 0;

	crear_paquete(&paquete, OBTENERVALOR);
	serializar_campo(&paquete, &offset, sharedVar, sizeof(stSharedVar));



	if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
		log_error("No se pudo enviar el SharedVar al Nucleo.");
	}

	recibir_paquete (configuracionInicial.sockNucleo, &paquete);

	type = obtener_paquete_type(&paquete);

	if (type != OBTENERVALOR)
	{
		log_error("Fallo al recibir paquete del nucleo.");
		free_paquete(&paquete);
		return (-1);
	}

	deserializar_pcb(&sharedVar , &paquete);

	resultado = sharedVar->valor;

	free_paquete(&paquete);

	liberarHeaderIPC(unHeaderIPC);

	return resultado;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){

	stHeaderIPC *unHeaderIPC;
	t_paquete paquete;
	stSharedVar sharedVar;
	int offset = 0;


	t_valor_variable resultado;

	unHeaderIPC = nuevoHeaderIPC(GRABARVALOR);

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

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

	return resultado;
}

t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta){

	t_puntero_instruccion PUNTERO;
	printf("Llamo a irAlLabel");
	return PUNTERO;
}

t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_puntero donde_retornar, t_puntero_instruccion linea_en_ejecuccion){

	t_puntero_instruccion PUNTERO;
	printf("Llamo a llamarFuncion");
	return PUNTERO;
}

void retornar(t_valor_variable retorno){

	printf("Llamo a retornar");
}

void imprimir(t_valor_variable valor_mostrar){

	stHeaderIPC* unHeaderPrimitiva;
	t_paquete paquete;
	int offset = 0;

	unHeaderPrimitiva = nuevoHeaderIPC(IMPRIMIR);

	enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva);

	crear_paquete(&paquete, IMPRIMIR);

	serializar_campo(&paquete, &offset, &valor_mostrar, sizeof(valor_mostrar));

	serializar_header(&paquete);

	if (enviar_paquete(configuracionInicial.sockNucleo, &paquete)) {
		log_error("No se pudo enviar el paquete para primitiva IMPRIMIR");
	}

	free_paquete(&paquete);

	liberarHeaderIPC(unHeaderPrimitiva);

}

void imprimirTexto(char* texto){

	stHeaderIPC* unHeaderPrimitiva;
	stPosicion posicionVariable;

	unHeaderPrimitiva = nuevoHeaderIPC(IMPRIMIRTEXTO);

	enviarMensajeIPC(configuracionInicial.sockNucleo,unHeaderPrimitiva,texto);

	if(!recibirHeaderIPC(configuracionInicial.sockNucleo,&unHeaderPrimitiva)){
		printf("Error: Fallo la impresion del texto:  %s.\n", texto);

	}

	if (unHeaderPrimitiva->tipo == OK)
		printf("Se imprime texto: %s \n", texto);

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
		.AnSISOP_llamarConRetorno		= llamarFuncion,
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
	stHeaderIPC *stHeaderIPC;

	if(!recibirHeaderIPC(socket,stHeaderIPC)){
		printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
		fflush(stdout);
	}

	printf("HandShake mensaje recibido %d",stHeaderIPC->tipo);

	if (stHeaderIPC->tipo == QUIENSOS)
	{
		stHeaderIPC = nuevoHeaderIPC(tipoHeader);
		if(!enviarHeaderIPC(socket,stHeaderIPC)){
			printf("No se pudo enviar el MensajeIPC\n");
			liberarHeaderIPC(stHeaderIPC);
			return (-1);
		}
	}

	if(!recibirHeaderIPC(socket,stHeaderIPC)){
			printf("SOCKET_ERROR - No se recibe un mensaje correcto\n");
			fflush(stdout);
			liberarHeaderIPC(stHeaderIPC);
			return (-1);
	}

	printf("HandShake: mensaje recibido %d",stHeaderIPC->tipo);
	fflush(stdout);

	if(stHeaderIPC->tipo == OK)
	{
		printf("Conexión establecida con id: %d...\n",tipoHeader);
		fflush(stdout);
		liberarHeaderIPC(stHeaderIPC);
		return socket;
	}

	liberarHeaderIPC(stHeaderIPC);
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

	printf("Conectando con %d...\n",puerto);
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
	uint32_t i, j, k, max_stack, max_args, max_vars;
	stIndiceStack *stack;
	stPosicion *argumento;

	recibir_paquete (configuracionInicial.sockNucleo, &paquete);

	type = obtener_paquete_type(&paquete);

	//if (cargarPCB(unMensaje.contenido) != -1)
	if (type == EXECANSISOP)
	{
		unPCB = (stPCB*)malloc(sizeof(stPCB));
		deserializar_pcb(unPCB , &paquete);

		max_stack = list_size(unPCB->stack);

		for (i = 0; i < max_stack; ++i) {
			stack = list_get(unPCB->stack, i);
			max_args = list_size(stack->argumentos);
			max_vars = list_size(stack->variables);
			printf("Posicion: %d\n",stack->pos);
			printf("Argumentos\n");
			printf("----------------------------\n");
			for (j = 0; j < max_args; ++j) {
				printf("Argumento [%d] --> ",j);
				argumento  = list_get(stack->argumentos, j);
				printf("Pagina:[%d] - ",argumento->pagina);
				printf("Offset:[%d] - ",argumento->offset);
				printf("Size:[%d]\n",argumento->size);
			}
			printf("----------------------------\n");
			printf("Variables\n");
			printf("----------------------------\n");
			for (k = 0; k < max_args; ++k) {
				printf("Variable [%d] --> ",k);
				unaVar  = list_get(stack->variables, k);
				printf("Id:[%d] - ",unaVar->id);
				printf("Pagina:[%d] - ",unaVar->posicion_memoria.pagina);
				printf("Offset:[%d] - ",unaVar->posicion_memoria.offset);
				printf("Size:[%d]\n",unaVar->posicion_memoria.size);
			}
			printf("----------------------------\n");

			printf("Posicion de stack retorno: %d\n",stack->retPosicion);
			printf("Posicion de memoria de variable de retorno:\n");
			printf("Pagina:[%d] - ",stack->retVar.pagina);
			printf("Offset:[%d] - ",stack->retVar.offset);
			printf("Size:[%d]\n",stack->retVar.size);
			fflush(stdout);
		}







		//log_info("PCB de ANSIPROG cargado. /n");
		free_paquete(&paquete);
		return 0;
	}else
		return (-1);

}

/*
 =========================================================================================
 Name        : calcularPaginaFisica()
 Author      : Ezequiel Martinez
 Inputs      : Recibe la dirección logica
 Outputs     : .
 Description : Funcion para obtener
 =========================================================================================
 */
int calcularPaginaFisica (int paginaLogica){

	int paginaFisica = unPCB->paginaInicial;
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
void getInstruccion (int startRequest, int sizeRequest,char** instruccion){

	char* instruccionTemp="\0";

	stPosicion posicionInstruccion;
	stMensajeIPC *unMensaje;

	int paginaToUMC;
	int startToUMC = startRequest;
	int sizeToUMC;
	int pagina;

	int cantidadPaginas = (sizeRequest / tamanioPaginaUCM) + 1;

	for (pagina=1;pagina<cantidadPaginas;pagina++)
	{
		sizeToUMC = tamanioPaginaUCM - startToUMC;

		if (sizeToUMC > sizeRequest)
			sizeToUMC = pagina*tamanioPaginaUCM - sizeRequest;

		paginaToUMC = calcularPaginaFisica(pagina);

		posicionInstruccion.pagina = paginaToUMC;
		posicionInstruccion.offset = startRequest;
		posicionInstruccion.size = sizeToUMC;

		//enviarHeaderIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(READ_BTYES_PAGE));

		enviarMensajeIPC(configuracionInicial.sockUmc,nuevoHeaderIPC(READ_BTYES_PAGE),&posicionInstruccion);

		recibirMensajeIPC(configuracionInicial.sockUmc, unMensaje );

		//enviar_paquete (UMC, posicionInstruccion);
		//recibir_paquete (UCM, unPaquete);
		instruccionTemp = (char*)unMensaje->contenido; //unPaquete.contenido;

		string_append (*instruccion,instruccionTemp);

		startToUMC = startToUMC + sizeToUMC;

	}

	free(instruccionTemp);

}

/*
 =========================================================================================
 Name        : ejecutarInstruccion()
 Author      : Ezequiel Martinez
 Inputs      : N/A
 Outputs     : Retorna -1 en caso de haber algun error.
 Description : Ejecuta una instrucción del PCB.
 =========================================================================================
 */
int ejecutarInstruccion(void){

	int programCounter = unPCB->pc;
	char* instruccion = NULL;

	getInstruccion(unPCB->metadata_program->instrucciones_serializado[programCounter].start,
					unPCB->metadata_program->instrucciones_serializado[programCounter].offset,
					*instruccion);

	if (instruccion != NULL){
		analizadorLinea(strdup(instruccion), &AnSISOP_functions, &kernel_functions);
	}else{
		printf("Error: fallo la ejecución de instrucción.\n");
		return (-1);
	}

	free(instruccion);
	return 0;

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
	int unSocket;
	int quantum=0;
	int quantumSleep=0;
	char* temp_file = "cpu.log";

	 //Primero instancio el log
	 t_log* logger = log_create(temp_file, "CPU",-1, LOG_LEVEL_INFO);

	log_info("Iniciando el proceo CPU..."); /* prints CPU Application */


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
		//log_info("OK - Nucleo conectado.");
		fflush(stdout);
		//loguear(OK_LOG,"Nucleo conectado","Nucleo"); TODO Agregar funcion de logueo.

	}	//Fin de conexion al Nucleo//


	/***** Lanzo conexión con el UMC ********/

	//log_info("Conectando al UMC...");

	configuracionInicial.sockUmc = cpuConectarse(configuracionInicial.ipUmc, configuracionInicial.puertoUmc, "UMC");

	if (configuracionInicial.sockUmc > 0){

		FD_SET(configuracionInicial.sockUmc,&(fds_master));
		configuracionInicial.socketMax = configuracionInicial.sockUmc;
		SocketAnterior = configuracionInicial.socketMax;
		//log_info("OK - UMC conectada.");
		fflush(stdout);

	}
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

						//loguear(INFO_LOG,"Se perdio conexion con Nucleo","Nucleo");//TODO Funciones de logueo

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

//							log_info("Respondiendo solicitud ANSIPROG...");

							unHeaderIPC = nuevoHeaderIPC(OK);
							enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

							if (cargarPCB() != -1)
							{
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


						case SIGUSR1:

							log_info("Respondiendo solicitud SIGUSR1...");

							unHeaderIPC = nuevoHeaderIPC(SIGUSR1CPU);
							/* Notifico al nucleo mi desconexion*/
							enviarHeaderIPC(configuracionInicial.sockNucleo,unHeaderIPC);

							configuracionInicial.salir = 1;
							break;

					}
				}

			}

		}
	}

	liberarHeaderIPC(unHeaderIPC);
	cerrarSockets(&configuracionInicial);
	log_info("CPU: Fin del programa");
	log_destroy(logger);
	return EXIT_SUCCESS;
}
