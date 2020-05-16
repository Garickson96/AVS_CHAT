#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "../headers/universal.h"

extern int debug_avs_chat;

/**
 *
 */
void osetri_chybu(const char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota, bool zavri_spojenie, int socket_descr) {
	if (hodnota_porovnaj == chybova_hodnota) {
		printf(CHYBA_TUCNA_CERVENA);
		perror(popis_chyby);
		printf(KONIEC_FORMATOVANIA);

		if (zavri_spojenie) {
			close(socket_descr);
		}

		exit(EXIT_FAILURE);
	}
}

/**
 *
 */
void osetri_chybu_malloc(const char *popis_chyby, void *smernik_malloc) {
	if (smernik_malloc == NULL) {
		printf(CHYBA_TUCNA_CERVENA);
		perror(popis_chyby);
		printf(KONIEC_FORMATOVANIA);

		exit(EXIT_FAILURE);
	}
}

/**
 *
 */
void osetri_chybu_nekriticka(const char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota) {
	if (hodnota_porovnaj == chybova_hodnota) {
		printf(CHYBA_NEKRITICKA_FIALOVA);
		perror(popis_chyby);
		printf(KONIEC_FORMATOVANIA);
	}
}

/**
 *
 */
bool osetri_chybu_suboru(const char *popis_chyby, void *smernik_subor) {
	if (smernik_subor == NULL) {
		printf(CHYBA_TUCNA_CERVENA);
		perror(popis_chyby);
		printf(KONIEC_FORMATOVANIA);

		return true;
	}

	return false;
}

/**
 *
 */
void *vytvor_nastav_malloc(int velkost, const char *sprava) {
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
void vypis_nadpis(const char *text) {
	printf("%s%s%s\n", TUCNE, text, KONIEC_FORMATOVANIA);
	printf("=========================================================\n\n");
}

/**
 *
 */
void vypis_uspech(const char *text) {
	printf("%s%s%s\n", OZNAM_TUCNY_ZELENY, text, KONIEC_FORMATOVANIA);
}

/**
 *
 */
void vypis_chybu(const char *text) {
	printf("%s[CHYBA]: %s%s\n", CHYBA_TUCNA_CERVENA, text, KONIEC_FORMATOVANIA);
}

/**
 *
 */
void vypis_informaciu(const char *text) {
	printf("%s%s%s\n", OZNAM_TUCNY_MODRY, text, KONIEC_FORMATOVANIA);
}

/**
 *
 */
void vypis_popisok(const char *text) {
	printf("%s%s%s", TUCNE, text, KONIEC_FORMATOVANIA);
	fflush(stdout);
}

/**
 *
 */
void debug_sprava(const char *sprava) {
	if (debug_avs_chat == 1 || debug_avs_chat == 2) {
		printf("%s[DEBUG]: %s%s\n", DEBUG_ORANZOVA, sprava, KONIEC_FORMATOVANIA);
	}
}

/**
 *
 */
void debug_sprava_rozsirena(const char *sprava) {
	if (debug_avs_chat == 2) {
		printf("%s[DEBUG - EXT]: %s%s\n", DEBUG_ORANZOVA, sprava, KONIEC_FORMATOVANIA);
	}
}

/**
 *
 */
void debug_ip_sprava(struct sockaddr_in *ip, const char *sprava) {
	if (debug_avs_chat == 2) {
		printf("%s[DEBUG]: %s:%d -> %s%s\n", DEBUG_ORANZOVA, inet_ntoa(ip->sin_addr), ntohs(ip->sin_port), sprava, KONIEC_FORMATOVANIA);
	}
}
