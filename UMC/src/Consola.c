/*
 * Consola.c
 *
 *  Created on: 8/7/2016
 *      Author: utnso
 */

#include "Consola.h"

void mostrarHelp(){

	printf("Comandos disponibles a ejecutar: (Se puede elegir)\n");
	printf("\t1. retardo <tiempo>\n");
	printf("\t2. dump tabla [<pid>]\n");
	printf("\t3. dump memoria [<pid>]\n");
	printf("\t4. flush tlb\n");
	printf("\t5. flush memoria [<pid>]\n");
	printf("\n>>>");

}

void readConsole(char* buffer, char* comando, char* parametros)
{
	int i;
	int j = 0;
	int k = 0;
	int into = 1;
	int param = 0;

	memset(comando,'\0',20);

	memset(parametros,'\0',155);

	for(i=0;i<MAX_BUFFER;i++)
	{
		if(param==1)
		{
			parametros[k] = buffer[i];
			k++;
		}

		if(buffer[i]!=' ' && param !=1){
			comando[j]=buffer[i];
			j++;
		}

		if(buffer[i]==' ')
		{
			param = 1;

		}

	}
}

void consolaUMC(){
	char buffer[MAX_BUFFER];
	char comando[20];
	char parametro[155];
	int retardo;

	fgets(buffer,200,stdin);

	readConsole(buffer, comando, parametro);

	/*
	 * Este comando permitirá modificar la cantidad de milisegundos que debe
	 * esperar el proceso UMC antes de responder una solicitud. Este parámetro será
	 * de ayuda para evaluar el funcionamiento del sistema.
	 */
	if(!strcmp(comando,"retardo"))
	{

		retardo = atoi(parametro);
		log_info("se cambia el retardo. Ahora es %d", retardo);
		losParametros.delay=retardo;

		fflush(stdin);
		fflush(stdout);
/*
 * Este comando generará un reporte en pantalla y en un archivo en disco del
 * estado actual de:
 * 		○ Estructuras de memoria: Tablas de páginas de todos los procesos o de un
 * 		proceso en particular.
 * 		○ Contenido de memoria: Datos almacenados en la memoria de todos los
 * 		procesos o de un proceso en particular.
 */
	}else if(!strcmp(comando,"dump tabla"))
	{

		log_info("se hace el dump de la table");
		if (parametro)
			;
		// TODO dump tabla

		fflush(stdin);
		fflush(stdout);

	}else if(!strcmp(comando,"dump memoria"))
	{
		log_info("se hace el dump de la memoria");
		// TODO dump memoria
		fflush(stdin);
		fflush(stdout);
/*
 * ○ tlb: Este comando deberá limpiar completamente el contenido de la TLB.
 * ○ memory: Este comando marcará todas las páginas del proceso como modificadas
 */
	}else if(!strcmp(comando,"flush tlb"))
	{
		log_info("se hace el flush de la TLB");
		// TODO flush tlb
		fflush(stdin);
		fflush(stdout);
	}else if(!strcmp(comando,"flush memoria"))
	{
		log_info("se hace el flush de la memoria");
		// TODO flush memoria
		fflush(stdin);
		fflush(stdout);

	}else{
		printf("Comando no interpretado\n");
		fflush(stdin);
		fflush(stdout);
	}

	mostrarHelp();
	return;
}
