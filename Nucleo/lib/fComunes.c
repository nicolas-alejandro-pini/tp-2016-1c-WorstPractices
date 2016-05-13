/*----------------------------------------------------------------------------*/
/*                --------------------------------------------                */
/*                 BIBLIOTECA PARA USO DE FUNCIONES COMUNES                   */
/*                --------------------------------------------                */
/*                                                                            */
/*  Nombre: fComunes.c                                                         */
/*  Versión: 1.0                                                              */
/*  Fecha: 18 de Abril de 2016					              				  */
/*  Description: Implementación de la biblioteca funciones comunes            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*#include "../lib/librerias.h"*/


/*----------------------------------------------------------------------------------------------*/
/*
 * Entrada : Tipo de log,OK_LOG ERROR_LOG WARNING_LOG INFO_LOG DEBUG_LOG. 
 * 			 sdata Informacion adicional a almacenar..
 *           proceso el nombre del proceso que genero el log, es para
 * 			 identificar el archivo de log.
 *
 * Salida  : Devuelve 0 si se grabo bien el suceso sino retorna -1. 
/*----------------------------------------------------------------------------------------------*/
/*
int loguear(int itipo, const char *sdata,const char *proceso) 
{
   FILE *flog;
   char szALoging[LARGOLOG];
   char szFecha[30];
   char szNombreLog[35];
   char sdesc[6];
   time_t timer;

   memset(szALoging, '\0', LARGOLOG);
   memset(szFecha, '\0', 30);
   memset(szNombreLog, '\0', 35);
   memset(sdesc, '\0', 6);

   timer = time(NULL);
   
   strcpy(szFecha, asctime(localtime(&timer)));
   
   szFecha[strlen(szFecha)-1] = '\0';
   
   sprintf(szNombreLog, "%s%d.log", proceso,getpid());


   switch (itipo)
{
	case OK_LOG:
	sprintf(sdesc, "%s", "OK   ");
	break;
	case ERROR_LOG:
	sprintf(sdesc, "%s", "ERROR   ");
	break;
	case WARNING_LOG:
	sprintf(sdesc, "%s", "WARNING   ");
	break;
	case INFO_LOG:
	sprintf(sdesc, "%s", "INFO   ");
	break;
	case DEBUG_LOG:
	sprintf(sdesc, "%s", "DEBUG   ");
	break;

}
  
   sprintf(szALoging, "%s %s[%d]:%s:%s\n", szFecha, proceso, getpid(), sdesc, sdata);
   
   if((flog = fopen(szNombreLog, "a")) == NULL) {
      printf("ERROR EN EL ARCHIVO LOG!!!\n");
      perror("Error description:");
      return(-1);
   }

   fputs(szALoging, flog);
   fclose(flog);
   return(0);
}
*/
/*----------------------------------------------------------------------------------------------*/
/*Automatizacion de la función fork(). Devuelve 0 si es el hijo y 1 si es el padre.*/
/*
int partir(){
	int resultado;
	if((resultado = fork()) == -1)
		return -1;
	return(resultado);
}
*/

