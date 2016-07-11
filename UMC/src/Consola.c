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

void readConsole(char* buffer, char* comando, char parametros[CANTIDAD_PARAMETROS][MAX_PARAMETROS])
{
	int i;
	int j = 0;
	int k = 0;
	int param = 0;

	// inicializo comando
	for(j=0;j<MAX_COMANDO;j++)
		comando[j]='\0';

	//inicializo parametros
	for(i=0;i<CANTIDAD_PARAMETROS;i++)
		for(j=0;j<MAX_PARAMETROS;j++)
			parametros[i][j]='\0';
	j=0;

	for(i=0;i<MAX_BUFFER;i++)
	{
		if(param==1&& param<CANTIDAD_PARAMETROS)
		{
			parametros[param-1][k] = buffer[i];
			k++;
		}

		if(buffer[i]!=' ' && param !=1){
			comando[j]=buffer[i];
			j++;
		}

		if(buffer[i]==' ')
		{
			param++;
			k=0;

		}

	}
}

void consolaUMC(){
	char buffer[MAX_BUFFER];
	char comando[MAX_COMANDO];
	char parametros[CANTIDAD_PARAMETROS][MAX_PARAMETROS];
	int retardo, pid;

	fgets(buffer,200,stdin);

	readConsole(buffer, comando, parametros);

	/*
	 * Este comando permitirá modificar la cantidad de milisegundos que debe
	 * esperar el proceso UMC antes de responder una solicitud. Este parámetro será
	 * de ayuda para evaluar el funcionamiento del sistema.
	 */
	if(!strcmp(comando,"retardo"))
	{

		retardo = atoi(parametros[0]);
		log_info("se cambia el retardo. Ahora es %d", retardo);
		losParametros.delay=retardo;

/*
 * Este comando generará un reporte en pantalla y en un archivo en disco del
 * estado actual de:
 * 		○ Estructuras de memoria: Tablas de páginas de todos los procesos o de un
 * 		proceso en particular.
 * 		○ Contenido de memoria: Datos almacenados en la memoria de todos los
 * 		procesos o de un proceso en particular.
 */
	}else if(!strcmp(comando,"dump"))
	{
		if(!strcmp(parametros[0],"tabla"))
		{
			if (parametros[1][0]=='\0'){
				log_info("se hace el dump de la tabla de todos los procesos");
				mostrarTabla();
			}
			if (parametros[1][0]!='\0'){
				log_info("se hace el dump de la tabla de un proceso");
				pid = atoi(parametros[1]);
				mostrarTablaPid(pid);

			}
		}else if(!strcmp(parametros[0],"memoria")){

			if (parametros[1][0]=='\0'){
				log_info("se hace el dump de la memoria de todos los procesos");
				// TODO dump memoria todos los procesos


			}
			if (parametros[1][0]!='\0')
				log_info("se hace el dump de la memoria de un proceso");
				// TODO dump memoria de un proceso

		}

/*
 * ○ tlb: Este comando deberá limpiar completamente el contenido de la TLB.
 * ○ memory: Este comando marcará todas las páginas del proceso como modificadas
 */
	}else if(!strcmp(comando,"flush"))
	{
		if(!strcmp(parametros[0],"tlb"))
		{
			log_info("se hace el flush de la TLB");
			// TODO flush tlb
		}else if(!strcmp(parametros[0],"memoria"))
		{
			if (parametros[1][0]=='\0')
				log_info("se hace el flush de la memoria de todos los procesos");
			// TODO flush memoria todos los procesos
			if (parametros[1][0]!='\0')
				log_info("se hace el flush de la memoria de un proceso");
			// TODO flush memoria de un proceso

		}
	}else{
		printf("Comando no interpretado\n");

	}

	fflush(stdin);
	fflush(stdout);

	mostrarHelp();
	return;
}
