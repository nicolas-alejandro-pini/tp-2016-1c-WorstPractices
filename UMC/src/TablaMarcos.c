/*
 * TablaMarcos.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "TablaMarcos.h"
#include "TLB.h"

/* puntero a la tabla de Marcos */
static t_list_mutex *TablaMarcos;

stRegistroTP *buscarRegistroEnTabla(uint16_t pid, uint16_t paginaBuscada){

	stNodoListaTP *nodo;
	stRegistroTP *registro;

	nodo = buscarPID(pid);
	registro = nodo->tabla+(sizeof(stRegistroTP)*paginaBuscada);

	return registro;
}

int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t *frame){

	stRegistroTP *registro = NULL;

	registro = buscarRegistroEnTabla(pid, paginaBuscada);
	if(registro){

		if(registro->bitPresencia==0)
			return 0;
		registro->bit2ndChance=1;
		*frame = registro->marco;
		return registro->marco;
	}
	return 0;
}

/*
	 * Clock o second chance
	 * Si el frame tiene el bit de acceso en 1, lo actualiza a 0 y avanza el puntero.
	 * Cuando encuentra un frame en 0, reemplaza la pagiina que contiene por la que produjo el fallo de pagina.
	 */

stRegistroTP *EjecutarClock(stNodoListaTP *tablaPaginas, uint16_t paginaEntrante, uint16_t *paginaSaliente){
	stRegistroTP *victima = NULL;
	stRegistroTP *regTP = NULL;
	uint16_t i;// = tablaPaginas->punteroClock;
	int puntero_siguiente = -1;

	// Me posiciono donde apunta el puntero
	for(i=tablaPaginas->punteroClock;!victima && i<=tablaPaginas->size;i++){

		if(i == (tablaPaginas->size))
			i=0;	// todas las paginas empiezan en 0

		regTP = obtenerRegistroTabladePaginas(tablaPaginas, i);

		// Si es valido el registro
		if(regTP->bitPresencia==1){
			// Si bit2ndChance es 1 lo pongo en 0 y avanzo.
			if(regTP->bit2ndChance==1){
				regTP->bit2ndChance=0;
			// Si bit2ndChance es 0, reemplazo
			}else{
				victima=regTP;
				*paginaSaliente = i; // Lo uso por si tiene bit modificado
			}
		}

	}
	// Posiciono el puntero en la pagina siguiente con presencia
	// con la pagina que se busca
	i=paginaEntrante;
	while(-1 == puntero_siguiente){

		if(i < (tablaPaginas->size-1))
			i++;
		else
			i=0;  // todas las paginas empiezan en 0

		regTP = obtenerRegistroTabladePaginas(tablaPaginas, i);
		if(regTP->bitPresencia==1)
			puntero_siguiente=i;
	}
	tablaPaginas->punteroClock=puntero_siguiente;

	return victima;
}

stRegistroTP *EjecutarClockModificado(stNodoListaTP *tablaPaginas, uint16_t paginaNueva, uint16_t *paginaSaliente){

	stRegistroTP *regTP = NULL;
	stRegistroTP *victima = NULL;
	int i, j, size;//posicionInicial, posicionActual=-1;
	t_queue *cola;
	uint16_t *paginaCola;
	/*
	 * Clock Modificado o Enhanced second chance
	 * Si el frame tiene el bit de acceso en 1, lo actualiza a 0 y avanza el puntero.
	 * Cuando encuentra un frame en 0, reemplaza la pagina que contiene por la que produjo el fallo de pagina.
	 *
	 * Las pagina modificadas no pueden reemplazarse hasta que se escriban en memoria secundaria (Disco).
	 * El clock mejorado genera 4 clases de frames:
	 * 		1) No accedida, No Modificada (bit2ndChance=0;bitModificado=0)
	 * 		2) Accedida, No modificada (bit2ndChance=1;bitModificado=0)
	 * 		3) No accedida, Modificada (bit2ndChance=0;bitModificado=1)
	 * 		4) Accedida, Modificada (bit2ndChance=1;bitModificado=1)
	 * 	Paso 1: Recorro la lista de frames buscando uno con los bits bit2ndChance=0; bitModificado=0 (No cambiar los bits de uso)
	 * 	Paso 2: Si en el paso anterior no encuentro ningun frame, recorro la lista de frames buscando los bits bit2ndChance=0; bitModificado=1.
	 * 	En el recorrido pongo el bit bit2ndChance en 0 a medida que voy pasando.
	 *	Paso 3: Si el paso anterior no encontro ningun frame, habre regresado a la posicion de comienzo y todos los frames tienen el bit2ndChance=0.
	 *	Proseguir con el paso 1 y de ser necesario el paso 2.
	 */
	// Creo una cola con las paginas presentes
	cola = queue_create();
	j=tablaPaginas->punteroClock;

	for(i=0; i<tablaPaginas->size; i++, j++){
		if(j == (tablaPaginas->size))
			j=0;
		regTP = obtenerRegistroTabladePaginas(tablaPaginas, j);
		if(regTP->bitPresencia==1){
			paginaCola = calloc(1,sizeof(uint16_t));
			*paginaCola = j;
			queue_push(cola,paginaCola);
		}
	}

	do{
		// recorro tabla Paso 1

		for(i=0;victima==NULL && (size=queue_size(cola))!=i;i++){

			paginaCola = (uint16_t*)queue_pop(cola);
			regTP= obtenerRegistroTabladePaginas(tablaPaginas,*paginaCola);

			// Si bit2ndChance=0; bitModificado=0 o avanzo.
			if(regTP->bit2ndChance==0 && regTP->bitModificado==0){
				victima = regTP;
				*paginaSaliente=*paginaCola;
			} // no modifico ningun bit en el primer recorrido
			queue_push(cola, paginaCola);
			// avanzo
		}
		// recorro tabla Paso 2
		for(i=0;!victima && (size=queue_size(cola))!=i;i++){

			paginaCola = (uint16_t*)queue_pop(cola);
			regTP = obtenerRegistroTabladePaginas(tablaPaginas, *paginaCola);

			// Si bit2ndChance=0; bitModificado=1 o avanzo.
			if(regTP->bit2ndChance==0 && regTP->bitModificado==1){
				victima=regTP;
				*paginaSaliente=*paginaCola;  // Lo utilizo para guardar en swap la pagina modificada
			}else if(regTP->bit2ndChance==1){
				// Si bit2ndChance es 1 lo pongo en 0 y avanzo.
				regTP->bit2ndChance=0;
			}
			queue_push(cola, paginaCola);
			// paso 3 es volver a empezar a recorrer desde el paso 1 con los bits modificados
		}
	}while(!victima);

	// Posiciono el puntero en la pagina siguiente con presencia
	// con la pagina que se busca
	paginaCola = (uint16_t*)queue_pop(cola);
	tablaPaginas->punteroClock = *paginaCola;

	queue_clean(cola);

	return victima;
}


int agregarFrameATablaMarcos(uint16_t frameNuevo, stNodoListaTP *tablaPaginas, uint16_t pagina){

	stRegistroTP *registro = NULL;
	registro = obtenerRegistroTabladePaginas(tablaPaginas, pagina);

	if(registro==NULL)
		return EXIT_FAILURE;

	registro->marco = frameNuevo;
	registro->bitPresencia = 1;
	registro->bitModificado = 0;
	registro->bit2ndChance = 1;   // Empieza inicializado en 1 (ver ppt)

	// Muevo el puntero de marcos de esta tabladePaginas
	if(pagina==(tablaPaginas->size-1))
		pagina=0;
	else
		pagina++;

	tablaPaginas->punteroClock = pagina;

	return EXIT_SUCCESS;
}



/* Devuelve null si no existe la tabla de paginas del PID o es un proceso nuevo y no hay marcos libres */
int reemplazarValorTabla(uint16_t *frameNuevo, stNodoListaTP *tablaPaginas, uint16_t pagina){

	stRegistroTP *victima = NULL;
	stRegistroTP *registroPresencia = NULL;
	stRegistroTLB registro;
	uint16_t paginaSaliente = -1;

	// El registro de la pagina solicitada se va a guardar en frameNuevo
	// Busco victima en la tabla de paginas del proceso.
	if(string_equals_ignore_case(losParametros.algoritmo,"CLOCK-M")){
		victima = EjecutarClockModificado(tablaPaginas, pagina, &paginaSaliente);
		log_info("Pid[%d] CLOCK-M: Pagina Victima[%d] Pagina Nueva[%d]", tablaPaginas->pid, paginaSaliente, pagina);
	}
	else{
		victima = EjecutarClock(tablaPaginas, pagina, &paginaSaliente);
		log_info("Pid[%d] CLOCK: Pagina Victima[%d] Pagina Nueva[%d]", tablaPaginas->pid, paginaSaliente, pagina);
	}

	if(victima==NULL)
		return EXIT_FAILURE;

	if(victima->bitModificado==1){
		log_info("Pid[%d]: graba pagina[%d] en Swap. Libero marco[%d]", tablaPaginas->pid, paginaSaliente, victima->marco);
		if(grabarEnSwap(tablaPaginas->pid, victima->marco, paginaSaliente)){
			log_error("Error al guardar pagina [%d] del Pid[%d] en swap", paginaSaliente, tablaPaginas->pid);
			return EXIT_FAILURE;
		}
		else
			log_info("grabada OK.");
	}

	// Seteo los bits de la pagina del pageFault como entrante
	registroPresencia = obtenerRegistroTabladePaginas(tablaPaginas, pagina);
	registroPresencia->bit2ndChance = 1;
	registroPresencia->bitModificado = 0;
	registroPresencia->bitPresencia = 1;
	registroPresencia->marco = victima->marco;  // guardo la direccion del marco a usar

	// Quito victima de la TLB
	registro.marco = victima->marco;
	registro.pagina = paginaSaliente;
	registro.pid = tablaPaginas->pid;
	if(estaActivadaTLB()==OK){
		imprimirTLB();
		quitarValorTLB(registro);
	}
	log_info("Pid[%d] Pagina[%d] Marco[%d] borrada de la TLB", registro.pid, registro.pagina, registro.marco);

	// Seteo bits de la victima en 0
	victima->bit2ndChance = 0;
	victima->bitModificado = 0;
	victima->bitPresencia = 0;
	victima->marco = 0;  // ya lo guarde, ahora no tiene marco asignado



	// Devuelvo el marco de la victima
	*frameNuevo = registroPresencia->marco;

	return EXIT_SUCCESS;
}
void setSecondChance(uint16_t pid, uint16_t pagina){
	stNodoListaTP *nodo;
	stRegistroTP* reg;

	nodo = buscarPID(pid);
	if (nodo==NULL){
		log_error("no existe proceso %d para setear secondChance", pid);
		return;
	}
	reg = obtenerRegistroTabladePaginas(nodo, pagina);
	if (reg==NULL){
		log_error("no existe registro de pagina %d de pid %d para setear secondChance", pagina, pid);
		return;
	}
	reg->bit2ndChance=1;

	return;
}
void creatListaDeTablas(){
	//sleep(losParametros.delay);
	TablaMarcos = list_mutex_create();
}

int crearTabla(uint16_t processId, uint16_t longitud_tabla){

	stNodoListaTP *nodo = NULL;
	void *tabla = NULL;
	int i;
	int posicionEnTablaMarcos;

	tabla = calloc(longitud_tabla,sizeof(stRegistroTP));

	//recorro la tabla para inicializarla
	for(i=0;i<longitud_tabla;i++){
		((stRegistroTP *)(tabla+(sizeof(stRegistroTP)*i)))->bit2ndChance=0;
		((stRegistroTP *)(tabla+(sizeof(stRegistroTP)*i)))->bitModificado=0;
		((stRegistroTP *)(tabla+(sizeof(stRegistroTP)*i)))->bitPresencia=0;
	}

	nodo = calloc(1,sizeof(stNodoListaTP));

	nodo->pid=processId;
	nodo->size=longitud_tabla;
	nodo->tabla=tabla;
	nodo->punteroClock = 0; // Apunta al ultimo elemento de la tabla

	// agrego retardo
	usleep(losParametros.delay*1000);
	//enlazo en la lista
	posicionEnTablaMarcos = list_mutex_add(TablaMarcos, nodo);

	return posicionEnTablaMarcos;
}

stNodoListaTP *buscarPID(uint16_t pid){
	stNodoListaTP* nodoListaTP = NULL;

	void _comparo_con_pid(stNodoListaTP *list_nodo){
		if(list_nodo->pid == pid){
			nodoListaTP = list_nodo;
		}
	}
	usleep(losParametros.delay*1000);
	list_mutex_iterate(TablaMarcos, (void*)_comparo_con_pid);

	// NULL: si no lo encontro, sino puntero a nodo
	return nodoListaTP;
}

int obtenerPresenciasTabladePaginas(stNodoListaTP* nodo){
	int i=0, presencias=0;
	stRegistroTP *retorno = NULL;

	if(nodo==NULL)
		return 0;

	for(i=0;i<nodo->size;i++){
		retorno = nodo->tabla+(sizeof(stRegistroTP)*i);
		if(retorno->bitPresencia==1)
			presencias++;
	}

	return presencias;
}

stRegistroTP* obtenerRegistroTabladePaginas(stNodoListaTP* nodo, int pagina){
	stRegistroTP *registro = NULL;

	if(nodo==NULL)
		return registro;

	// es mayor igual porque empieza en la pagina 0
	if(pagina >= nodo->size)
		return registro;

	registro = nodo->tabla+(sizeof(stRegistroTP)*pagina);
	return registro;
}

int grabarEnSwap(uint16_t pid, uint16_t marco, uint16_t pagina){
	stPosicion posLogica;

	void *buf=NULL;
	buf=malloc(losParametros.frameSize);

	// Posicion a grabar
	posLogica.offset=0;
	posLogica.pagina = pagina;
	posLogica.size = losParametros.frameSize;

	if(leerMemoria(buf, marco, posLogica))
		return EXIT_FAILURE;

	if(enviarPagina(pid, pagina, buf))
		return EXIT_FAILURE;

	free(buf);

	return EXIT_SUCCESS;
}

int setBitModificado(uint16_t pid, uint16_t pagina){
	stNodoListaTP* nodoListaTP = NULL;
	stRegistroTP* registro = NULL;

	nodoListaTP = buscarPID(pid);

	if(!nodoListaTP)
		return EXIT_FAILURE;

	registro = obtenerRegistroTabladePaginas(nodoListaTP, pagina);

	if(registro->bitPresencia == 1){
		registro->bitModificado = 1;
	}
	else
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}



void _mostrarContenidoTP(stNodoListaTP *list_nodo){
	int i;
	//stRegistroTP *nodo;
	stRegistroTP* regTP;

	printf("--------------------------------------------------------------------------------\n");
	printf("pid: %d\n", list_nodo->pid);
	//nodo= ((stRegistroTP*)list_nodo->tabla);

	for(i=0;i<list_nodo->size;i++){
		regTP = obtenerRegistroTabladePaginas(list_nodo, i);
		printf("Pagina: %d ", i);
		printf("Marco: %d ", regTP->marco);
		printf("bit2ndChance: %d ", regTP->bit2ndChance);
		printf("bitModificado: %d ", regTP->bitModificado);
		printf("bitPresencia: %d ", regTP->bitPresencia);
		if(regTP->bitPresencia==1)
			printf("<<<<<--------------");
		printf("\n");
	}
	fflush(stdout);
}
void _mostrarContenidoMemoria(stNodoListaTP* nodoPid){
	stRegistroTP *regTP;
	stPosicion posicion;
	int i;
	uint16_t marco;
	void* buffer;

	if(nodoPid->size==0){
		printf("La memoria esta vacia para el proceso %d", nodoPid->pid);
		return;
	}
	printf("\npid: %d\n", nodoPid->pid);
	//nodo= ((stRegistroTP*)nodoPid->tabla);

	for(i=0;i<nodoPid->size;i++){
		regTP = obtenerRegistroTabladePaginas(nodoPid, i);

		if(regTP->bitPresencia==1){
			printf("\n--------------------------------------------------------------------------------\n");
			printf("Pagina %d:\n", i);
			printf("--------------------------------------------------------------------------------\n");
			posicion.offset=0;
			posicion.pagina=i;
			posicion.size=losParametros.frameSize;

			marco = regTP->marco;
			buffer = calloc(1, losParametros.frameSize+1);
			if(leerMemoria(buffer, marco, posicion)!=0){
				log_error("no se pudo leer memoria - marco: %d", marco);
			}
			printf("%s", (char*)buffer);
			free(buffer);
		}
	}

}
void marcarMemoriaModificada(uint16_t pid){
	int i;
	stRegistroTP *nodo;
	stNodoListaTP *list_nodo;

	list_nodo = buscarPID(pid);
	if(list_nodo == NULL){
		printf("No se encuentra el proceso %d para marcarlo como modificado", pid);
		return;
	}
	if(list_nodo->size==0){
		printf("No hay memoria disponible para este el proceso %d para marcarlo como modificado", pid);
		return;
	}
	printf("pid: %d\n", list_nodo->pid);
	//nodo= ((stRegistroTP*)list_nodo->tabla);
	for(i=0;i<list_nodo->size;i++){
		nodo = obtenerRegistroTabladePaginas(list_nodo, i);
		printf("Pagina: %d ", i);
		//printf("Marco: %d ", (nodo+(i*sizeof(stRegistroTP)))->marco);
		printf("Marco: %d ", nodo->marco);
		nodo->bitModificado=1;
		printf("Marcado como modificado");
		printf("\n");
	}
}
void listarMemoria(){

	if(list_mutex_is_empty(TablaMarcos)){
		printf("La memoria esta vacia");
		return;
	}
	list_mutex_iterate(TablaMarcos, (void*)_mostrarContenidoMemoria);

}

void listarMemoriaPid(uint16_t pid){
	stNodoListaTP* nodoPid;

	//sleep(losParametros.delay);

	nodoPid = buscarPID(pid);
	if(nodoPid==NULL){
		printf("El proceso %d no se encuentra en memoria\n", pid);
		return;
	}
	_mostrarContenidoMemoria(nodoPid);
}
void mostrarTabla(){

	if(list_mutex_is_empty(TablaMarcos))
		printf("La Tabla esta vacia\n");
	//sleep(losParametros.delay);
	list_mutex_iterate(TablaMarcos, (void*)_mostrarContenidoTP);
	return;
}
void mostrarTablaPid(uint16_t pid){
	stNodoListaTP* nodo;

	//sleep(losParametros.delay);
	nodo = buscarPID(pid);
	if (nodo==NULL){
		printf("No se encuentra el proceso en la Tabla de Pagina\n");
		return;
	}
	_mostrarContenidoTP(nodo);
}

void liberarTablaPid(uint16_t pid){
	stNodoListaTP *nodoListaTP = NULL;
	int i=0;
	int index = 0;

	void _comparo_con_pid_y_borro_tabla(stNodoListaTP *list_nodo){
		if(list_nodo->pid == pid){
			nodoListaTP = list_nodo;
			liberarMarcosXtabla(nodoListaTP);
			free(nodoListaTP->tabla);
			index = i;
		}
		i++;
	}
	list_mutex_iterate(TablaMarcos, (void*)_comparo_con_pid_y_borro_tabla);
	usleep(losParametros.delay*1000);
	list_mutex_remove(TablaMarcos,index);

	// mostrarTabla();
}

void liberarMarcosXtabla(stNodoListaTP* nodo){
	stRegistroTP* registro = NULL;
	int i;
	for(i=0; i< nodo->size; i++){
		registro = obtenerRegistroTabladePaginas(nodo, i);
		if(registro->bitPresencia == 1)
		{
			liberarMarco(registro->marco);
		}
	}
}
