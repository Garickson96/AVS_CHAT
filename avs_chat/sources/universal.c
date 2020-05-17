#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include "../headers/universal.h"

extern int debug_avs_chat;

/**
 * Pri nastati takejto chyby sa porovnaju hodnoty v hodnota_porovnaj a chybova_hodnota - ak nastane zhoda, tak sa informuje pouzivatel o chybe.
 * Tu su zaradene kriticke chyby, ktore maju za nasledok nasilne ukoncenie programu. V priprade potreby sa uzavrie sokcket spojenie, ktore je uvedene ako parameter.
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
 * V pripade, ze sa operacia malloc nepodari, tak vrati NULL. V nasom pripade je to kriticka chyba programu - vypise sa hlaska.
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
 * Pri nekritickej chybe sa porovnavaju dve ciselne hodnoty. Ak doslo k zhode, tak sa vypise chybova hlaska, bez nasilneho ukoncenia programu.
 */
void osetri_chybu_nekriticka(const char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota) {
	if (hodnota_porovnaj == chybova_hodnota) {
		printf(CHYBA_NEKRITICKA_FIALOVA);
		perror(popis_chyby);
		printf(KONIEC_FORMATOVANIA);
	}
}

/**
 * V pripade chyby sa smernik na FILE nastavuje na NULL. My sme tuto funkciu spravili viac univerzalnejsiu (pre zvukove subory z SFML), preto tam je void *.
 * Vracia bool - true = chyba nastala, false = chyba nenastala.
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
 * Vytvori pamatovy priestor v halde o velkosti zadanych bajtov. Vynuluje sa pozadovany priestor. V pripade chyby sa vypise hlaska. V pripade, ze vytvorenie prebehlo OK, tak
 * sa vrati smernik na pamatovy priestor.
 */
void *vytvor_nastav_malloc(unsigned int velkost, const char *sprava) {
	void *vytvorene_data = malloc(velkost);
	osetri_chybu_malloc(sprava, vytvorene_data);
	memset(vytvorene_data, 0, velkost);

	return vytvorene_data;
}

/**
 * Ak zadany smernik na smernik na priestor nie je NULL, tak sa uvolni a premenna, na ktoru sa ukazuje sa nastavi na NULL.
 */
void dealokuj_malloc(void **malloc_priestor) {
	if (*malloc_priestor != NULL) {
		free(*malloc_priestor);
		*malloc_priestor = NULL;
	}
}

/**
 * Vypise tucnym pismom nadpis a prida ciaru pod zadany text.
 */
void vypis_nadpis(const char *text) {
	printf("%s%s%s\n", TUCNE, text, KONIEC_FORMATOVANIA);
	printf("=========================================================\n\n");
}

/**
 * Vypise hlasku uspechu so zelenym pismom. Text je ziskany z parametra.
 */
void vypis_uspech(const char *text) {
	printf("%s%s%s\n", OZNAM_TUCNY_ZELENY, text, KONIEC_FORMATOVANIA);
}

/**
 * Vypise chybovu hlasku s cervenym pismom.
 */
void vypis_chybu(const char *text) {
	printf("%s[CHYBA]: %s%s\n", CHYBA_TUCNA_CERVENA, text, KONIEC_FORMATOVANIA);
}

/**
 * Vypise vseobecnu informaciu pre pouzivatela modrym pismom. Text je ziskany z parametra.
 */
void vypis_informaciu(const char *text) {
	printf("%s%s%s\n", OZNAM_TUCNY_MODRY, text, KONIEC_FORMATOVANIA);
}

/**
 * Vypise tucnym pismom popisok pred zadavanym textom od pouzivatela. Neodriadkuvava sa.
 */
void vypis_popisok(const char *text) {
	printf("%s%s%s", TUCNE, text, KONIEC_FORMATOVANIA);
	fflush(stdout);
}

/**
 * V pripade DEBUG levelu 1 alebo 2 sa vypise debug sprava oranzovym pismom. Text je ziskany z parametra.
 */
void debug_sprava(const char *sprava) {
	if (debug_avs_chat == 1 || debug_avs_chat == 2) {
		printf("%s[DEBUG]: %s%s\n", DEBUG_ORANZOVA, sprava, KONIEC_FORMATOVANIA);
	}
}

/**
 * Len v priprade DEBUG levelu 2 sa vypise debug sprava oranzovym pismom. Text je ziskany z parametra.
 */
void debug_sprava_rozsirena(const char *sprava) {
	if (debug_avs_chat == 2) {
		printf("%s[DEBUG - EXT]: %s%s\n", DEBUG_ORANZOVA, sprava, KONIEC_FORMATOVANIA);
	}
}

/**
 * Len v pripade DEBUG levelu 2 sa vypise debug sprava s IP adresou prijemcu spravy. Pouziva sa pri DISCOVERY spravach.
 */
void debug_ip_sprava(struct sockaddr_in *ip, const char *sprava) {
	if (debug_avs_chat == 2) {
		printf("%s[DEBUG - EXT]: %s:%d -> %s%s\n", DEBUG_ORANZOVA, inet_ntoa(ip->sin_addr), ntohs(ip->sin_port), sprava, KONIEC_FORMATOVANIA);
	}
}
