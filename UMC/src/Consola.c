/*
 * Consola.c
 *
 *  Created on: 8/7/2016
 *      Author: utnso
 */

#include "Consola.h"

void mostrarHelp(){

	printf("\nComandos disponibles a ejecutar: (Elegir el nuemro correspondiente)\n");
	printf("\t1. retardo <tiempo>\n");
	printf("\t2. dump tabla\n");
	printf("\t3. dump tabla <pid>\n");
	printf("\t4. dump memoria\n");
	printf("\t5. dump memoria <pid>\n");
	printf("\t6. flush tlb\n");
	printf("\t7. flush memoria <pid>\n");
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
void inicializar(char* buffer, int longitud){
	// inicializo comando
	int j;
	for(j=0;j<longitud;j++)
		buffer[j]='\0';
}

void consolaUMC(){
	char buffer[MAX_BUFFER];
	char comando[MAX_COMANDO];
	char parametros[CANTIDAD_PARAMETROS][MAX_PARAMETROS];
	int retardo, pid;

	fgets(buffer,MAX_BUFFER,stdin);

	inicializar(comando,MAX_COMANDO);
	inicializar(parametros[0],MAX_PARAMETROS);
	inicializar(parametros[1],MAX_PARAMETROS);
	//readConsole(buffer, comando, parametros);

	/*
	 * Este comando permitirá modificar la cantidad de milisegundos que debe
	 * esperar el proceso UMC antes de responder una solicitud. Este parámetro será
	 * de ayuda para evaluar el funcionamiento del sistema.
	 */
	//2. retardo <tiempo>
	if(!strcmp(comando,"1"))
	{
		printf("Especificar retardo en segundos:");
		fgets(parametros[0], MAX_PARAMETROS, stdin);

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
		// 2. dump tabla
	}else if(!strcmp(comando,"2"))
	{
		log_info("se hace el dump de la tabla de todos los procesos");
		mostrarTabla();

		// 2. dump tabla <pid>
	}else if (!strcmp(comando,"3")){

		log_info("se hace el dump de la tabla de un proceso");
		printf("Especificar proceso:");
		fgets(parametros[0], MAX_PARAMETROS, stdin);

		pid = atoi(parametros[0]);
		mostrarTablaPid(pid);

		// 4. dump memoria
	}else if(!strcmp(parametros[0],"4")){

		log_info("se hace el dump de la memoria de todos los procesos");
		listarMemoria();

		// 5. dump memoria <pid>
	}else if (!strcmp(parametros[0],"5")){

		log_info("se hace el dump de la memoria de un proceso");

		printf("Especificar proceso:");
		fgets(parametros[0], MAX_PARAMETROS, stdin);

		pid = atoi(parametros[0]);
		listarMemoriaPid(pid);

/*
 * ○ tlb: Este comando deberá limpiar completamente el contenido de la TLB.
 * ○ memory: Este comando marcará todas las páginas del proceso como modificadas
 */
		// 6. flush tlb
	}else if(!strcmp(comando,"6"))
	{
		log_info("se hace el flush de la TLB");
		flushTLB();

		// 7. flush memoria <pid>
	}else if (!strcmp(comando,"7")){

		log_info("se hace el flush de la memoria de un proceso");
		printf("Especificar proceso:");
		fgets(parametros[0], MAX_PARAMETROS, stdin);

		pid = atoi(parametros[0]);
		marcarMemoriaModificada(pid);

	}else{
		printf("Comando no interpretado\n");

	}

	fflush(stdin);
	fflush(stdout);

	mostrarHelp();
	return;
}
