#include <signal.h>
#include <unistd.h>

#include "commons/log.h"
#include "CPU.h"

t_configCPU *config;

/**
 * Funcion de interpretacion de seniales
 */
void signal_handler(int sigNumber)
{
  if (sigNumber == SIGUSR1 || sigNumber ==SIGINT){
    log_info("Recibida signal de finalizacion.");
    config->salir = 1;
  }
}

/**
 * Arranca la rutina de interpretacion de seniales de la CPU
 */
void init_signal_handler(t_configCPU *configuracion_inicial){

	config = configuracion_inicial;

	if(signal(SIGUSR1, signal_handler) == SIG_ERR)
		log_error("No pude trapear la signal SIGUSR1");
	if(signal(SIGINT, signal_handler) == SIG_ERR)
		log_error("No pude trapear la signal SIGINT");
}
