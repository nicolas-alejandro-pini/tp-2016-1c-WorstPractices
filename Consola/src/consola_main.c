#include <stdio.h>
#include <stdlib.h>
#include "consola.h"
#include <sys/wait.h>
#include <signal.h>
#define SERVER_TEST_PORT 50001
#define SERVER_BACKLOG 10

void sigchld_handler(int s)
{
	while(wait(NULL) > 0);
}

int server_test(void)
{
	int sockfd, newfd;
	struct sockaddr_in myAddr;
	struct sockaddr_in theirAddr;
	int sin_size;
	struct sigaction sa;
	int yes=1;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket server");
		return -1;
	}
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return -1;
	}
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(SERVER_TEST_PORT);
	myAddr.sin_addr.s_addr = INADDR_ANY; // ip local
	memset(&(myAddr.sin_zero), '\0', 8);
	if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return -1;
	}
	if (listen(sockfd, SERVER_BACKLOG) == -1) {
	     perror("listen");
	     return -1;
	}
	sa.sa_handler = sigchld_handler; // Eliminar procesos muertos
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		return -1;
	}
	while(1) {  // main accept() loop
		sin_size = sizeof(struct sockaddr_in);
	    if ((newfd = accept(sockfd, (struct sockaddr *)&theirAddr, &sin_size)) == -1){
	    	perror("accept");
	    	continue;
	    }
	    printf("server: got connection from %s\n", inet_ntoa(theirAddr.sin_addr));
	    if (!fork())
	    {
	    	close(sockfd);
	        if (send(newfd, "Hello, world!\n", 14, 0) == -1)
	            perror("send");
	            close(newfd);
	            return -1;
	    	}
	        close(newfd);
	    }
	return 0;
}

int main(int argc, char *argv[]) {
	t_console tConsola;
    char *log = "consola.log";
	t_log *logger = log_create(log,"CONSOLA", -1, LOG_LEVEL_INFO);
	log_info("Inicio consola...");

	if(load_program(&tConsola, argc, argv))
	{
		log_error("Error al cargar cargar el programa.");
	    log_destroy(logger);
	    exit(EXIT_FAILURE);
	}
	else{
		log_info("\nPrograma: \n[%s]\n", tConsola.pProgram);
	}

	if(create_console(&tConsola))
	{
		log_error("Error al crear la consola.");
	    log_destroy(logger);
	    destroy_console(&tConsola);
	    exit(EXIT_FAILURE);
	}
	else
	{
		log_info("Puerto [%d]", config_get_int_value(tConsola.tConfigFile, PARAM_PORT));
		log_info("IP [%d]", config_get_int_value(tConsola.tConfigFile, PARAM_IP));
	}

	if(connect_console(&tConsola))
	{
		log_error("Error al conectar consola.");
	    log_destroy(logger);
	    destroy_console(&tConsola);
	    exit(EXIT_FAILURE);
	}

	if(handshake_console(&tConsola))
	{
		log_error("Error al realizar handshake con el nucleo.");
	    log_destroy(logger);
	    destroy_console(&tConsola);
	    exit(EXIT_FAILURE);
	}
	else{
		log_info("\Handshake OK  IP[]", tConsola.pProgram);
	}

	if(send_program(&tConsola))
	{
		log_error("Error al enviar programa al nucleo.");
	    log_destroy(logger);
	    destroy_console(&tConsola);
	    exit(EXIT_FAILURE);
	}

	if(recv_print(&tConsola))
	{
		log_error("Error al realizar impresion o finalizar programa.");
	    log_destroy(logger);
	    destroy_console(&tConsola);
	    exit(EXIT_FAILURE);
	}

	destroy_console(&tConsola);
	log_info("Fin consola...OK");
	return EXIT_SUCCESS;
}
