/*
 * TablaMarcos.c
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#include "TablaMarcos.h"

stRegistroTP *buscarRegistroEnTabla(uint16_t pid, uint16_t paginaBuscada){

	stNodoListaTP *nodo;
	stRegistroTP *registro;

	nodo = buscarPID(pid);
	registro = nodo->tabla+(sizeof(stRegistroTP)*paginaBuscada);

	return registro;
}

int buscarEnTabla(uint16_t pid, uint16_t paginaBuscada, uint16_t **frame){

	stRegistroTP *registro;

	registro = buscarRegistroEnTabla(pid, paginaBuscada);
	if(registro->bitPresencia==0)
		return 0;
	registro->bit2ndChance=1;
	*frame = malloc(sizeof(uint16_t));
	**frame = registro->marco;
	return registro->marco;
}
// voy a elegir uno de los registros de la tabla y lo reemplazo con el parametro registro
stRegistroTP *EjecutarClock(stNodoListaTP *nodo, uint16_t pagina, stRegistroTP registro, uint8_t flag){
	stRegistroTP *regTP;
	int i;

	/*
	 * Clock o second chance
	 * Si el frame tiene el bit de acceso en 1, lo actualiza a 0 y avanza el puntero.
	 * Cuando encuentra un frame en 0, reemplaza la pagiina que contiene por la que produjo el fallo de pagina.
	 */

	// recorro tabla
	for(i=0;i<nodo->size;i++){
		regTP = nodo->tabla+(sizeof(stRegistroTP)*i);

		// Si es valido el registro
		if(regTP->bitPresencia==1){
			// Si bit2ndChance es 1 lo pongo en 0 y avanzo.
			if(regTP->bit2ndChance==1){
				regTP->bit2ndChance=0;
			// Si bit2ndChance es 0, reemplazo
			}else if(regTP->bit2ndChance==0){
				regTP->bit2ndChance=registro.bit2ndChance;
				regTP->bitModificado=registro.bitModificado;
				regTP->bitPresencia=registro.bitPresencia;
				// Si no tengo que reusar la memoria, asigno el nuevo marco a la pagina
				if (flag!=REEMPLAZAR_MARCO)
					regTP->marco=registro.marco;
				// termino de recorrer
				break;
			}
		}
		// avanzo
	}

	return regTP;
}
stRegistroTP *EjecutarClockModificado(stNodoListaTP *nodo, uint16_t pagina, stRegistroTP registro, uint8_t flag){
	stRegistroTP *regTP;
	int i, estado;

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

	do{
		// recorro tabla Paso 1
		for(i=0;i<nodo->size;i++){
			regTP = nodo->tabla+(sizeof(stRegistroTP)*i);

			// Si es valido el registro
			if(regTP->bitPresencia==1){
				// Si bit2ndChance=0; bitModificado=0 o avanzo.
				if(regTP->bit2ndChance==0 && regTP->bitModificado==0){
					regTP->bit2ndChance=registro.bit2ndChance;
					regTP->bitModificado=registro.bitModificado;
					regTP->bitPresencia=registro.bitPresencia;
					// Si no tengo que reusar la memoria, asigno el nuevo marco a la pagina
					if (flag!=REEMPLAZAR_MARCO)
						regTP->marco=registro.marco;
					// termino de recorrer
					estado=ENCONTRADO;
					return regTP;
				}
			}
			// avanzo
		}
		// recorro tabla Paso 2
		for(i=0;i<nodo->size && estado==NO_ENCONTRADO;i++){
			regTP = nodo->tabla+(sizeof(stRegistroTP)*i);

			// Si es valido el registro
			if(regTP->bitPresencia==1){
				// Si bit2ndChance=0; bitModificado=1 o avanzo.
				if(regTP->bit2ndChance==0 && regTP->bitModificado==1){
					regTP->bit2ndChance=registro.bit2ndChance;
					regTP->bitModificado=registro.bitModificado;
					regTP->bitPresencia=registro.bitPresencia;
					// Si no tengo que reusar la memoria, asigno el nuevo marco a la pagina
					if (flag!=REEMPLAZAR_MARCO)
						regTP->marco=registro.marco;
					// termino de recorrer
					estado=ENCONTRADO;
					break;
				}else if(regTP->bit2ndChance==1){
					// Si bit2ndChance es 1 lo pongo en 0 y avanzo.
					regTP->bit2ndChance=0;
				}
			}
			// avanzo

		}
	}while(estado==ENCONTRADO);

	return regTP;
}
stRegistroTP *reemplazarValorTabla(uint16_t pid, uint16_t pagina, stRegistroTP registro, uint8_t flagReemplazoAsignados){

	stNodoListaTP *nodo;
	stRegistroTP *retorno;
	int i, presencias;
	void *buf;

	nodo = buscarPID(pid);
// casos nueva pagina desde swap:


	for(i=0;i<nodo->size;i++){
		retorno = nodo->tabla+(sizeof(stRegistroTP)*i);
		if(retorno->bitPresencia==1)
			presencias++;
	}
// - si es un nuevo proceso, y no hay memoria-> rechazo el pedido.
	if(presencias==0 && REEMPLAZAR_MARCO)
		return NULL;
// - Hay espacio en memoria y es menor a la cantidad de pag por pid-> la ubico en un nuevo marco y nuevo registro de Tabla.
	if(presencias <= losParametros.frameByProc && !REEMPLAZAR_MARCO){
		for(i=0;i<nodo->size;i++){
			retorno = nodo->tabla+(sizeof(stRegistroTP)*i);
			if(retorno->bitPresencia==1){
				retorno->bit2ndChance=registro.bit2ndChance;
				retorno->bitModificado=registro.bitModificado;
				retorno->bitPresencia=registro.bitPresencia;
				retorno->marco=registro.marco;
				break;
			}
		}
// - Hay espacio en memoria y es mayor a la cantidad de pag por pid-> la ubico en un nuevo marco y reemplazo un registro de Tabla.
// - No hay espacio en memoria y es menor a la cantidad de pag por pid-> la ubico en un marco asignado y reemplazo un registro de Tabla.
// - No hay espacio en memoria y es mayor a la cantidad de pag por pid-> la ubico en un marco asignado y reemplazo un registro de Tabla.
	}else{
		if(REEMPLAZAR_MARCO || presencias > losParametros.frameByProc){
			if(string_equals_ignore_case(losParametros.algoritmo,"CLOCK"))
				retorno = EjecutarClock(nodo, pagina, registro, flagReemplazoAsignados);
			else if(string_equals_ignore_case(losParametros.algoritmo,"CLOCK_MODIFICADO"))
				retorno = EjecutarClockModificado(nodo, pagina, registro,flagReemplazoAsignados);
			else
				log_error("No hay un algoritmo correctamente cargado");
			// escribo en memoria secundaria si es necesario
			if(retorno->bitModificado==1)
				buf=malloc(losParametros.frameSize);
				memcpy(buf, memoriaPrincipal+retorno->marco, losParametros.frameSize);
				enviarPagina(pid, pagina, buf);

		}
	}

	return retorno;
}
int crearTabla(uint16_t processId, uint16_t cantidadPaginas){

	stNodoListaTP *nodo;
	stRegistroTP *tabla;
	int i;

	tabla = calloc(cantidadPaginas,sizeof(stRegistroTP));

	//recorro la tabla para inicializarla
	for(i=0;i<cantidadPaginas;i++){
		(tabla+(sizeof(stRegistroTP)*i))->bit2ndChance=0;
		(tabla+(sizeof(stRegistroTP)*i))->bitModificado=0;
		(tabla+(sizeof(stRegistroTP)*i))->bitPresencia=0;
	}

	nodo = calloc(1,sizeof(stNodoListaTP));

	nodo->size=cantidadPaginas;
	nodo->tabla=tabla;

	//creo tabla sino existe
	if(TablaMarcos==NULL)
		TablaMarcos = list_create();

	//enlazo en la lista
	list_add_in_index(TablaMarcos,processId, nodo);

	return 0;
}
stNodoListaTP *buscarPID(uint16_t pid){
	return (stNodoListaTP*)list_get(TablaMarcos,pid);
}
void liberarTablaPid(uint16_t pid){
	stNodoListaTP *nodo;
	stRegistroTP *registro;
	int i;

	nodo = buscarPID(pid);
	for(i=0;i<nodo->size;i++){
		registro = nodo->tabla+(sizeof(stRegistroTP)*i);
		liberarMarco(registro->marco);
	}
	free(registro);
	list_remove(TablaMarcos,pid);

	return;

}
