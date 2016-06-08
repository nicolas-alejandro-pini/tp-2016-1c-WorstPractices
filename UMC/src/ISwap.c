/*
 * ISwap.c
 *
 *  Created on: 6/6/2016
 *      Author: utnso
 */

#include "ISwap.h"

int inicializarSwap(stPageIni *st){


	/*
	 * PASAR pid, cantidad de pagina, y codigo prg
	 *
	 * devuelve OK o ERROR
	 */


	return 0;
}

//define INICIAR_PROGRAMA	141ul /* pid (ui), cantidad paginas (ui), (ul), codigo prg (char*) */
//define DESTRUIR_PROGRAMA	142ul /* pid */
//define LEER_PAGINA			143ul /* pid, numero de pagina */
//define ESCRIBIR_PAGINA		144ul /* pid, numero de pagina, contenido de pagina */

/*
 * enviarPagina()<- NULL
 * recibir pagina()<-contenido de la pagina
 *
 * destruirPrograma()<-NULL
 */


int enviarPagina(){
	int offset = 0;
		t_paquete paquete;
		t_UMCConfig UMCConfig;

		// cargo estructura
		UMCConfig.paginasXProceso = frameByProc;
		UMCConfig.tamanioPagina = frameSize;

		crear_paquete(&paquete, CONFIG_UMC);
		serializar_campo(&paquete, &offset, &UMCConfig, sizeof(UMCConfig));
		// Otra forma
		//serializar_campo(&paquete, &offset, &(UMCConfig.paginasXProceso), sizeof(UMCConfig.paginasXProceso));
		//serializar_campo(&paquete, &offset, &(UMCConfig.tamanioPagina), sizeof(UMCConfig.tamanioPagina));
		serializar_header(&paquete);

		enviar_paquete(sockSwap, &paquete);
		free_paquete(&paquete);

		return EXIT_SUCCESS;
}
