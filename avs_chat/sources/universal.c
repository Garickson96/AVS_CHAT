#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../headers/universal.h"

#define DEBUG 1

/**
 *
 */
void osetri_chybu(char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota, bool zavri_spojenie, int socket_descr) {
	if (hodnota_porovnaj == chybova_hodnota) {
		perror(popis_chyby);
		if (zavri_spojenie) {
			close(socket_descr);
		}

		exit(EXIT_FAILURE);
	}
}

/**
 *
 */
void debug_sprava(char *sprava) {
	#ifdef DEBUG
		printf("[DEBUG]: %s\n", sprava);
	#endif
}

/**
 *
 */
void debug_ip_sprava(struct sockaddr_in *ip, char *sprava) {
	#ifdef DEBUG
		printf("[DEBUG]: %s:%d -> %s\n", inet_ntoa(ip->sin_addr), ntohs(ip->sin_port), sprava);
	#endif
}
