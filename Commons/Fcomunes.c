/*------------------------------------------------------------------------------*/
/*                --------------------------------------------			*/
/*              		Funciones Comunes				*/
/*                --------------------------------------------			*/
/*------------------------------------------------------------------------------*/

/*
 Entrada : Se pasa el parametro a buscar en el archivo de configuracion y el 
 		   path completo de archivo de configuracion.
 
 Salida  : En caso de exito retorna el valor del valor pedido del archivo de 
  		   configuracion, sino aborta el programa.	
 */
int readconfig (const char *param,const char *file_name,char **r_value)
{
 FILE *fdconfig;
 char lparam[255];
 char lvalue[255];
 int flag=0;
 if ((fdconfig = fopen(file_name,"rt")) == NULL)
        {
	     perror("error al abrir archivo ");
	     exit(-1);
	    }
 while(1)
 	{
	 
 	 fscanf(fdconfig,"%s = %s",lparam,lvalue);
	 if ( feof(fdconfig) )
		break;

	if (strcmp(lparam,param) == 0) 
	   {
	    flag = 1;

	    (*r_value) = (char * ) malloc (strlen(lvalue) +1 ); 	
	    strcpy((*r_value),lvalue);

	    return strlen(lvalue);
  	   }
	}

  if (flag==0) 
 	{
	 printf("Parametro no cargado en el archivo de configuracion\n \"%s\"  \n",param);
	 exit(-2);	
	}
  return 0;
}

/*----------------------------------------------------------------------------------------------*/

/*
 * Entrada : Tipo de log,OK_LOG ERROR_LOG WARNING_LOG INFO_LOG DEBUG_LOG. 
 * 			sdata Informacion adicional a almacenar..
 *          proceso el nombre del proceso que genero el log, es para 
 * 			identificar el archivo de log.
 * Salida  : Devuelve 0 si se grabo bien el suceso sino retorna -1. 
 * */
int loguear(int itipo, const char *sdata,const char *proceso) 
{
   FILE *flog;
   char szALoging[8192];
   char szFecha[30];
   char szNombreLog[35];
   char sdesc[6];
   time_t timer; /*del asctime()*/

   memset(szALoging, '\0', 500);
   memset(szFecha, '\0', 30);
   memset(szNombreLog, '\0', 35);
   memset(sdesc, '\0', 6);

   timer = time(NULL);
   
   strcpy(szFecha, asctime(localtime(&timer)));
   
   szFecha[strlen(szFecha)-1] = '\0';
   
   sprintf(szNombreLog, "%s%d.log", proceso,getpid());

/* -----------------------------------Revisar---------------------------------- */

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

/*----------------------------------------------------------------------------------------------*/

int loguear2(int itipo, const char *sdata,const char *proceso) 
{
   FILE *flog2;
   char szALoging[8192];
   char szFecha[30];
   char szNombreLog[35];
   char sdesc[6];
   time_t timer; /*del asctime()*/

   memset(szALoging, '\0', 500);
   memset(szFecha, '\0', 30);
   memset(szNombreLog, '\0', 35);
   memset(sdesc, '\0', 6);

   timer = time(NULL);
   
   strcpy(szFecha, asctime(localtime(&timer)));
   
   szFecha[strlen(szFecha)-1] = '\0';
   
   sprintf(szNombreLog, "%s%d.log", proceso,getpid());

/* -----------------------------------Revisar---------------------------------- */

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
   
   if((flog2 = fopen(szNombreLog, "a")) == NULL) {
      printf("ERROR EN EL ARCHIVO LOG!!!\n");
      perror("Error description:");
      return(-1);
   }

   fputs(szALoging, flog2);
   fclose(flog2);
   return(0);
}

/*----------------------------------------------------------------------------------------------*/


int cambiarNombreProceso(char **argv,int argc,const char *nombre)
{
	int i,aux;

	if(*argv != NULL && argv != NULL && argc >= 0 && nombre != NULL)
	{
		i=0;
		while(argv[i] != NULL)
		{
			aux = strlen(argv[i]);
			memset(argv[i], 0, aux);
			i++;
		}
		strcpy(argv[0],nombre);
	}
	else
		return 0;
	return 1;
}

/*----------------------------------------------------------------------------------------------*/

void vaciarVector(char *unVector, int tamanio)
{
	memset(unVector,'\0',tamanio);
}

/*----------------------------------------------------------------------------------------------*/
int tieneElSocketEnNodoConectado(void* unPtr, void* unSocket)
/* Devuelve 1 si el socket es el que posee el nodo. */
{
	stConectados * unPtrANodoConectado = (stConectados *) unPtr;
	if(unPtrANodoConectado->socket == atoi(unSocket))
		return(1);
	return(0);
}

/*----------------------------------------------------------------------------------------------*/

int partir () {

	int resultado;
	if((resultado=fork())== -1){return -1;}
return resultado;
}
/*----------------------------------------------------------------------------------------------*/

void mostrarLista(const lista unaLista){
	
	lista listaAux= unaLista;
	stConectados* unPtr;
	
	if (size(listaAux) == 0) printf("No Existen Discos Conectados\n");fflush(stdout);	

	while(!isEmpty(listaAux)){
		
		unPtr = (stConectados*)primerDato(listaAux);
		quitarDeLista(&listaAux,NULL,quitarDeAdelante);
		
		printf("\nDisco Conectado en: %d, NombreDisco:%s\n",unPtr->socket,unPtr->dispositivo);
		fflush(stdout);
		
		}

	/*lista listaAux = NULL;
	listaAux = unaLista;
	stConectados* unPtr;
	
	while(listaAux!=NULL)
	{
		unPtr = (stConectados*)listaAux->ptrADato;
		printf("%s\n",unPtr->dispositivo);
		listaAux = listaAux->sgte;
	}*/
	
}
/*----------------------------------------------------------------------------------------------*/

