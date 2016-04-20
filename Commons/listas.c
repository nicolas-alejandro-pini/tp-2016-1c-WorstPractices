/*----------------------------------------------------------------------------*/
/*                --------------------------------------------                */
/*                 BIBLIOTECA PARA MANEJO DE LISTAS EN ANSI C                 */
/*                --------------------------------------------                */
/*----------------------------------------------------------------------------*/

/*
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
*/

/*----------------------------------------------------------------------------*/
/*                     Definiciones y Declaraciones                           */
/*----------------------------------------------------------------------------*/

typedef struct _nodo * ptrANodo;
typedef ptrANodo lista;

typedef struct _nodo{
    void     *ptrADato;
    ptrANodo  sgte;
} stNodo;


int agregarAtras(void* unPtrADato, void* otroPtrADato);
int agregarAdelante(void* unPtrADato, void* otroPtrADato);
int quitarDeAdelante(void* unPtrADato, void* otroPtrADato);
int quitarDeAtras(void* unPtrADato, void *laLista);
int igual(void* unPtrADato, void* otroPtrADato);
void crearNodoConectados( stConectados *newNode, int socket, char* dispositivo);


/*----------------------------------------------------------------------------*/
/*                         Funciones Privadas                                 */
/*----------------------------------------------------------------------------*/

ptrANodo crearNodo (void* unPtrADato)
/* Devuelve un puntero a stNodo en el que carga el puntero a dato 'unPtrADato' y setea el campo 'sgte' en NULL.*/
{
    ptrANodo unPtrANodo = (ptrANodo) malloc(sizeof(stNodo));
    unPtrANodo->ptrADato = unPtrADato;
    unPtrANodo->sgte = NULL;
    return(unPtrANodo);
}


/*----------------------------------------------------------------------------*/
/*                         Funciones Basicas                                  */
/*----------------------------------------------------------------------------*/

int size(const lista unaLista)
/* Devuelve la cantidad de datos que tiene 'unaLista'.*/
{
    unsigned cantidad = 0;
    ptrANodo unPtrAux = unaLista;
    while(unPtrAux != NULL)
    {
        unPtrAux = unPtrAux->sgte;
        cantidad ++;
    }
    return(cantidad);
}

/*----------------------------------------------------------------------------*/

int isEmpty(const lista unaLista)
/* Devuelve 1 si 'unaLista' está vacia y 0 sí no lo está.*/
{
    return(unaLista == NULL);
}

/*----------------------------------------------------------------------------*/

void *ultimoDato(lista unaLista)
/* Devuelve un puntero al último dato de 'unaLista'. */
{
    ptrANodo ptrAnt, ptrAux = unaLista;

    if(isEmpty(unaLista))
    {
        return(NULL);
    }
    while (ptrAux)
    {
        ptrAnt = ptrAux;
        ptrAux = ptrAux->sgte;
    }
    return(ptrAnt->ptrADato);
}

/*----------------------------------------------------------------------------*/

void *primerDato(lista unaLista)
{
  ptrANodo ptrAux = unaLista;

  if(isEmpty(unaLista))
    {
        return(NULL);
    }
  
 return (ptrAux->ptrADato);
}

/*----------------------------------------------------------------------------*/
void* siguienteDato(lista unaLista)
{
	ptrANodo ptrAux = unaLista;

	ptrAux = ptrAux->sgte;

	return ((ptrAux->ptrADato));
}


/*----------------------------------------------------------------------------*/
int agregarALista(lista *unPtrALista, void* unPtrADato, int(*funCriterio) (void*,void*))
/*Agrega el dato referenciado por 'unPtrADato' en la lista referenciada por 'unPtrALista' segun el criterio establecido en 'funCriterio'.
  Devuelve 1 sí el dato se agrega con éxito y 0 si no.
  La función 'funCriterio' debe devolver 1 sí el dato referenciado por el primer parametro que recibe debe ser agregado antes que el que recibe como segundo y 0 sí no.
  Templates de 'funCriterio':
                             > agregarAdelante() - Causa que el dato se agregue adelante.
                             > agregarAtras()    - Causa que el dato se agregue al final.
*/

{
ptrANodo ptrAnt = NULL, ptrAux = *unPtrALista, unPtrANodo = crearNodo(unPtrADato);

    if(ptrAux == NULL)
    {
        *unPtrALista = unPtrANodo;
        return(1);
    }

    while(ptrAux)
    {
        if(funCriterio(unPtrADato,ptrAux->ptrADato))
        {
            unPtrANodo->sgte = ptrAux;
            if(ptrAnt == NULL)
            {
                *unPtrALista = unPtrANodo;
            }else
            {
                ptrAnt->sgte = unPtrANodo;
            }
            return(1);
        }
        ptrAnt = ptrAux;
        ptrAux = ptrAux->sgte;
    }
    if(ptrAux == NULL)
    {
        ptrAnt->sgte = unPtrANodo;
        return(1);
    }
    return(0);
}

/*----------------------------------------------------------------------------*/

int quitarDeLista(lista *unPtrALista, void* unParametro, int(*funCriterio) (void*,void*))
  /*Quita de la lista referenciada por 'unPtrALista' el primer dato que cumpla con el criterio establecido en 'funCriterio' al recibir como segundo parametro 'unParametro'.
  Devuelve 1 sí se quitó con éxito un dato y 0 si no.
  La función 'funCriterio'debe devolver 1 sí el dato referenciado por el primer parametro que recibe debe ser eliminado, usando el segundo parametro ('unParametro) como referencia y 0 sí no.
  Templates de 'funCriterio':
                             > quitarDeAdelante() - Causa que se elimine el dato de adelante.
                             > quitarDeAtras()    - Causa que se elimine el dato de atras.
                             > igual()            - Causa que se elimine el primer dato igual al contenido del puntero que recibe como segundo parámetro.
*/
{	
    ptrANodo ptrAnt = NULL, ptrAux = *unPtrALista;

    while(ptrAux)
    {
        if(funCriterio(ptrAux->ptrADato,unParametro))
        {
            if(ptrAnt == NULL)
            {
                *unPtrALista = (*unPtrALista)->sgte;
            }else
            {
                ptrAnt->sgte = ptrAux->sgte;
            }
            free(ptrAux);
            return(1);
        }
        ptrAnt = ptrAux;
        ptrAux = ptrAux->sgte;
    }
    return(0);
}
int quitarDeListaSF(lista *unPtrALista, void* unParametro, int(*funCriterio) (void*,void*))
  /*Quita de la lista referenciada por 'unPtrALista' el primer dato que cumpla con el criterio establecido en 'funCriterio' al recibir como segundo parametro 'unParametro'.
  Devuelve 1 sí se quitó con éxito un dato y 0 si no.
  La función 'funCriterio'debe devolver 1 sí el dato referenciado por el primer parametro que recibe debe ser eliminado, usando el segundo parametro ('unParametro) como referencia y 0 sí no.
  Templates de 'funCriterio':
                             > quitarDeAdelante() - Causa que se elimine el dato de adelante.
                             > quitarDeAtras()    - Causa que se elimine el dato de atras.
                             > igual()            - Causa que se elimine el primer dato igual al contenido del puntero que recibe como segundo parámetro.
*/
{
    ptrANodo ptrAnt = NULL, ptrAux = *unPtrALista;

    while(ptrAux)
    {
        if(funCriterio(ptrAux->ptrADato,unParametro))
        {
            if(ptrAnt == NULL)
            {
                *unPtrALista = (*unPtrALista)->sgte;
            }else
            {
                ptrAnt->sgte = ptrAux->sgte;
            }
            /*free(ptrAux);*/
            return(1);
        }
        ptrAnt = ptrAux;
        ptrAux = ptrAux->sgte;
    }
    return(0);
}
/*----------------------------------------------------------------------------*/

int vaciarLista(lista *unPtrALista)
/* Vacia la lista referenciada por 'unaLista'.
   Devuelve 1 sí se vació la lista con éxito y 0 si no.
*/
{
    while(*unPtrALista)
    {
        if(quitarDeLista(unPtrALista,"",quitarDeAdelante)==0)
        {
            return(0);
        }
    }
    return(1);
}

/*----------------------------------------------------------------------------*/

void *buscarEnListaFull(const lista unaLista, void* unParametro, int(*funCriterio) (void*, void*))
/* Busca en la lista referenciada por 'unaLista' un dato que cumpla con el criterio establecido en 'funCriterio'.
   Devuelve un puntero la lista con los resultados; sí no se encuentra nada, devuelve NULL.
   Templates de 'funCriterio':
           > igual() - Causa que se busque el primer dato igual al contenido del puntero que recibe como segundo parámetro.
*/
{
 /*  ptrANodo unPtrAux = unaLista;
    lista listaAux = NULL;

    while(unPtrAux)
    {
        if(funCriterio(unPtrAux->ptrADato,unParametro))
        {
	    agregarALista(&listaAux,unPtrAux->ptrADato,agregarAdelante);
        }
        unPtrAux = unPtrAux->sgte;
    }

    return(listaAux);
}*/

ptrANodo unPtrAux = unaLista;

    lista * listaAux = NULL;
 
    listaAux = (lista *) malloc (sizeof(lista));
    *listaAux = NULL;  

    while(unPtrAux)
   {
        if(funCriterio(unPtrAux->ptrADato,unParametro))
        {
            /*return(unPtrAux->ptrADato);*/
              agregarALista(listaAux,unPtrAux->ptrADato,agregarAdelante);
        }
        unPtrAux = unPtrAux->sgte;

    }

    return(*listaAux);

}

/*----------------------------------------------------------------------------*/

void *buscarEnLista(const lista unaLista, void* unParametro, int(*funCriterio) (void*, void*))
/* Busca en la lista referenciada por 'unaLista' un dato que cumpla con el criterio establecido en 'funCriterio'.
   Devuelve un puntero a el dato encontrado; sí no se encuentra nada, devuelve NULL.
   Templates de 'funCriterio':
           > igual() - Causa que se busque el primer dato igual al contenido del puntero que recibe como segundo parámetro.
*/
{
    ptrANodo unPtrAux = unaLista;
    while(unPtrAux)
    {
        if(funCriterio(unPtrAux->ptrADato,unParametro))
        {
            return(unPtrAux->ptrADato);
        }
        unPtrAux = unPtrAux->sgte;
    }
    return(NULL);
}

/*----------------------------------------------------------------------------*/

void *hacerATodos(lista *unaLista, void* p1, void* p2, void* p3, int(*fun) (void*, void*, void*, void*))
/* Aplica una función a todos los elementos de una lista, recibiendo como primer parametro el elemento.*/
{
    ptrANodo unPtrAux = *unaLista;
    while(unPtrAux)
    {
        fun(unPtrAux->ptrADato,p1,p2,p3);
        unPtrAux = unPtrAux->sgte;
    }
    return(NULL);
}

/*---------------------------------------------------------------------------------*/
int ordenarLista(lista *unPtrALista, int (*funCriterio) (void*,void*))
/*Ordena la lista referenciada por 'unaLista' según el criterio establecido en 'funCriterio'.
  Devuelve 1 sí no hubo errores y 0 sí si.
*/
{
    lista unaListaNueva = NULL;
    ptrANodo unPtrAux = (*unPtrALista);
    while(unPtrAux)
    {
        if(!(agregarALista(&unaListaNueva,unPtrAux->ptrADato,funCriterio)))
        {
            return(0);
        }
        unPtrAux = unPtrAux->sgte;
    }
    vaciarLista(unPtrALista);
    *unPtrALista = unaListaNueva;
    return(1);
}

/*----------------------------------------------------------------------------*/








/*----------------------------------------------------------------------------*/

void imprimirLista(const lista unaLista, char *(*unaFuncion) (void*))
/* Imprime por pantalla la lista referenciada por 'unaLista', usando la plantilla generada por 'unaFuncion'.
   La funcion 'unaFuncion' debe recibir como parametro un puntero a void, castearlo al tipo de dato correspondiente y devolver un string que describa el contenido del mismo, como se desea que se muestre.
*/
{
    ptrANodo unPtrAux = unaLista;
    if(isEmpty(unaLista))
    {
        printf("<<<LA LISTA ESTA VACIA>>>\n");
    }
    while(unPtrAux)
    {
        printf("%s",unaFuncion(unPtrAux->ptrADato));
        unPtrAux = unPtrAux->sgte;
    }
}

/*----------------------------------------------------------------------------*/
/*                         Funciones Parametro                                */
/*----------------------------------------------------------------------------*/

/* Estas funciones se incluyen solo para ser usadas como parametro para otras funciones básicas. */

int agregarAtras(void* unPtrADato, void* otroPtrADato)
/* Puede usarse como criterio en:
                                > agregarALista() - Causa que el dato se agregue al final.
*/
{
    return(0);
}

/*----------------------------------------------------------------------------*/

int agregarAdelante(void* unPtrADato, void* otroPtrADato)
/* Puede usarse como criterio en:
                                > agregarALista() - Causa que el dato se agregue adelante.
*/
{
    return(1);
}

/*----------------------------------------------------------------------------*/

int quitarDeAdelante(void* unPtrADato, void* otroPtrADato)
/* Puede usarse como criterio en:
                                > quitarDeLista() - Causa que se elimine el dato de adelante.
*/
{
    return(1);
}

/*----------------------------------------------------------------------------*/

int quitarDeAtras(void* unPtrADato, void *laLista)
/* Puede usarse como criterio en:
                                > quitarDeLista() - Causa que se elimine el dato de atras.
*/
{
    if(igual(unPtrADato,ultimoDato((lista)laLista)))
    {
        return(1);
    }
    return(0);
}

/*----------------------------------------------------------------------------*/

int igual(void* unPtrADato, void* otroPtrADato)
/* Puede usarse como criterio en:
         > quitarDeLista() - Causa que se elimine el primer dato igual al contenido del puntero que recibe como segundo parámetro.

         > buscarEnLista() - Causa que se busque el primer dato igual al contenido del puntero que recibe como segundo parámetro.
*/
{
    return(*((char*)unPtrADato) == *((char*) otroPtrADato));
}
