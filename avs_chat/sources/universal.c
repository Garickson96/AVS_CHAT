#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

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
void osetri_chybu_malloc(char *popis_chyby, void *smernik_malloc) {
	if (smernik_malloc == NULL) {
		perror(popis_chyby);
		exit(EXIT_FAILURE);
	}
}

/**
 *
 */
void osetri_chybu_nekriticka(char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota) {
	if (hodnota_porovnaj == chybova_hodnota) {
		perror(popis_chyby);
	}
}

/**
 *
 */
bool osetri_chybu_suboru(char *popis_chyby, FILE *smernik_subor) {
	if (smernik_subor == NULL) {
		perror(popis_chyby);
		return true;
	}

	return false;
}

/**
 *
 */
void *vytvor_nastav_malloc(int velkost, char *sprava) {
	void *vytvorene_data = malloc(velkost);
	osetri_chybu_malloc(sprava, vytvorene_data);
	memset(vytvorene_data, 0, velkost);

	return vytvorene_data;
}

/**
 *
 */
void dealokuj_malloc(void **malloc_priestor) {
	if (*malloc_priestor != NULL) {
		free(*malloc_priestor);
		*malloc_priestor = NULL;
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
