/*----------------------------------------------------------------------------*/
/*                --------------------------------------------                */
/*                 BIBLIOTECA PARA MANEJO DE LISTAS EN ANSI C                 */
/*                --------------------------------------------                */
/*                                                                            */
/*  Nombre: listas.c                                                          */
/*  Version: 1.0                                                              */
/*  Fecha: 25 de abril de 2016                                                */
/*  Description: Implementacion de la biblioteca listas                       */
/*                                                                            */
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
/* Devuelve 1 si 'unaLista' est� vacia y 0 s� no lo est�.*/
{
    return(unaLista == NULL);
}

/*----------------------------------------------------------------------------*/

void *ultimoDato(lista unaLista)
/* Devuelve un puntero al �ltimo dato de 'unaLista'. */
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
  Devuelve 1 s� el dato se agrega con �xito y 0 si no.
  La funci�n 'funCriterio' debe devolver 1 s� el dato referenciado por el primer parametro que recibe debe ser agregado antes que el que recibe como segundo y 0 s� no.
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
  Devuelve 1 s� se quit� con �xito un dato y 0 si no.
  La funci�n 'funCriterio'debe devolver 1 s� el dato referenciado por el primer parametro que recibe debe ser eliminado, usando el segundo parametro ('unParametro) como referencia y 0 s� no.
  Templates de 'funCriterio':
                             > quitarDeAdelante() - Causa que se elimine el dato de adelante.
                             > quitarDeAtras()    - Causa que se elimine el dato de atras.
                             > igual()            - Causa que se elimine el primer dato igual al contenido del puntero que recibe como segundo par�metro.
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

/*----------------------------------------------------------------------------*/

int vaciarLista(lista *unPtrALista)
/* Vacia la lista referenciada por 'unaLista'.
   Devuelve 1 s� se vaci� la lista con �xito y 0 si no.
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

void *buscarEnLista(const lista unaLista, void* unParametro, int(*funCriterio) (void*, void*))
/* Busca en la lista referenciada por 'unaLista' un dato que cumpla con el criterio establecido en 'funCriterio'.
   Devuelve un puntero a el dato encontrado; s� no se encuentra nada, devuelve NULL.
   Templates de 'funCriterio':
           > igual() - Causa que se busque el primer dato igual al contenido del puntero que recibe como segundo par�metro.
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

/*---------------------------------------------------------------------------------*/
int ordenarLista(lista *unPtrALista, int (*funCriterio) (void*,void*))
/*Ordena la lista referenciada por 'unaLista' seg�n el criterio establecido en 'funCriterio'.
  Devuelve 1 s� no hubo errores y 0 s� si.
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

/* Estas funciones se incluyen solo para ser usadas como parametro para otras funciones b�sicas. */

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
         > quitarDeLista() - Causa que se elimine el primer dato igual al contenido del puntero que recibe como segundo par�metro.

         > buscarEnLista() - Causa que se busque el primer dato igual al contenido del puntero que recibe como segundo par�metro.
*/
{
    return(*((char*)unPtrADato) == *((char*) otroPtrADato));
}
