#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../headers/cli.h"
#include "../headers/discovery_udp.h"
#include "../headers/chat_tcp.h"
#include "../headers/universal.h"
#include "../headers/files_processor.h"

#define CISLO_PORTU_ODOSIELANIE 9001
#define BUFFER_CESTA 255

#define BUFFER_VELKOST 50
#define TEXT_VELKOST 160
#define BUFFER_DATUM 30
#define KONCOVA_SPRAVA ":KONIEC"

extern int debug_avs_chat;

/**
 *
 */
char vyber_volby(void) {
	char volba = '\0';
	vypis_popisok("Zadajte vasu volbu: ");
	scanf(" %c", &volba);
	return volba;
}

/**
 *
 */
void uprav_hodnotu_debug(void) {
	printf("Povolene hodnoty pre debug: \n");
	printf("0 - bez debug sprav\n");
	printf("1 - zakladne debug spravy\n");
	printf("2 - vsetky debug spravy\n\n");
	vypis_popisok("Vasa volba: ");

	int debug_test = 0;
	scanf("%d", &debug_test);

	if (debug_test >= 0 && debug_test <= 2) {
		debug_avs_chat = debug_test;
		vypis_uspech("Hodnota pre debug bola uspesne zmenena.");
	} else {
		vypis_chybu("Zadali ste nepovolenu hodnotu pre zobrazovanie debug sprav. Aktualny stav nebol zmeneny.");
	}
}

/**
 *
 */
void zoznam_menu(void) {
	printf("\n");
	vypis_nadpis("MENU PROGRAMU: ");

	printf("Volba a:\tvypise list accept\n");
	printf("Volba b:\tvypise list connect\n");
	printf("Volba c:\tposlanie spravy\n");
	printf("Volba d:\tzobraz svoj status\n");
	printf("Volba e:\tzmen svoj status\n");
	printf("Volba f:\taktualizuje statusy\n");
	printf("Volba g:\tzobrazenie obsahu zadaneho adresara\n");
	printf("Volba h:\tposielanie suborov\n");
	printf("Volba i:\tnastavenie prijimania/neprijimania suborov\n\n");

	printf("Volba x:\tukonc program\n");
	printf("Volba z:\tpovol zobrazovanie debug sprav\n");
	printf("Volba ?:\tzobrazenie menu\n\n");
}

/**
 *
 */
char *daj_ip_string(const struct sockaddr *adresa, char *buffer) {
	sprintf(buffer, "%s:%d", inet_ntoa(((struct sockaddr_in *)adresa)->sin_addr), ntohs(((struct sockaddr_in *)adresa)->sin_port));
	return buffer;
}

/**
 *
 */
void zobraz_list_connect(DOUBLYLINKEDLIST *list, char *moje_meno, char **pomenovania_statusov, int n) {
	DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
	printf("%s%-6s%-20s%-15s%-25s%-15s%-20s%s\n", TUCNE, "CISLO", "MENO", "SOCKET ID", "IP ADRESA", "STATUS", "CAS AKTUALIZACIE", KONIEC_FORMATOVANIA);
	int i = 0;
	char buffer[BUFFER_VELKOST];
	char buffer_datum[BUFFER_DATUM];

	while (aktualny != NULL) {
		if (strcmp(moje_meno, aktualny->data.meno) != 0) {
			strftime(buffer_datum, BUFFER_DATUM, "%d.%m.%Y %H:%M:%S", localtime(&(aktualny->data.cas_nastavenia)));

			printf("%-6d%-20s%-15d%-25s%-15s%-20s\n", i, aktualny->data.meno, aktualny->data.socket_id,
					daj_ip_string(&(aktualny->data.ip_adresa), buffer),
					pomenovania_statusov[aktualny->data.posledny_stav],
					buffer_datum);
		}

		aktualny = aktualny->next;
		i++;
	}
}

/**
 *
 */
void zobraz_list_accept(DOUBLYLINKEDLIST *list) {
	DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
	printf("%s%-6s%-15s%-25s%s\n", TUCNE, "CISLO", "SOCKET ID", "IP ADRESA", KONIEC_FORMATOVANIA);
	int i = 1;
	char buffer[BUFFER_VELKOST];

	while (aktualny != NULL) {
		printf("%-6d%-15d%-25s\n", i, aktualny->data.socket_id, daj_ip_string(&(aktualny->data.ip_adresa), buffer));
		aktualny = aktualny->next;
		i++;
	}
}

/**
 *
 */
void posli_spravu(DOUBLYLINKEDLIST *list, char *moje_meno, char **pomenovania_statusov, int n) {
	int cislo_adresata = -1;
	char buffer[TEXT_VELKOST];

	printf("Maximalna velkost spravy je obmedzena na 160 znakov.\nZakazane znaky: >\nPosielanie sprav ukoncte spravou :KONIEC.\n");
	vypis_popisok("Zadajte komu chcete odoslat spravu: ");
	printf("\n");

	zobraz_list_connect(list, moje_meno, pomenovania_statusov, n);

	printf("\n");
	vypis_popisok("Vyberte cislo adresata: ");
	scanf("%d", &cislo_adresata);

	// kontrola cisla adresata
	DOUBLYLINKEDLIST_ITEM *aktualny = overit_existenciu_pouzivatela(list, cislo_adresata);
	if (aktualny == NULL) {
		vypis_chybu("Pozadovany adresat neexistuje!\n");
		return;
	}

	// fflush(stdin);
	do {
		memset(buffer, 0, sizeof(buffer));
		vypis_popisok("Zadajte spravu: ");

		fgets(buffer, TEXT_VELKOST, stdin);
		buffer[strlen(buffer) - 1] = '\0';

		if (strlen(buffer) > 0) {
			if (strchr(buffer, ODDELOVAC) != NULL) {
				vypis_chybu("Sprava obsahuje nepovoleny znak. Nebola odoslana!\n");
				break;
			}

			if (strcmp(buffer, ":KONIEC") != 0) {
				chat_jeden_zapis(aktualny->data.socket_id, buffer, moje_meno);
			}
		}
	} while (strcmp(buffer, ":KONIEC") != 0);
}

/**
 *
 */
void vypis_dostupne_statusy(char **pomenovania_statusov, int n) {
	printf("Vypis moznych statusov: \n");
	for (int i = 0; i < n; i++) {
		printf("%d - %s\n", i, pomenovania_statusov[i]);
	}
}

/**
 *
 */
void zobraz_moj_status(char **pomenovania_statusov, int n, int *aktualny_stav) {
	if (*aktualny_stav >= 0 && *aktualny_stav < n) {
		printf("Vas aktualny stav je: %s.\n", pomenovania_statusov[*aktualny_stav]);
	} else {
		printf("Vas aktualny stav je: neznamy.\n");
	}
}

/**
 *
 */
void zmen_moj_status(char **pomenovania_statusov, int n, int *aktualny_stav, DOUBLYLINKEDLIST *list, char *moje_meno) {
	vypis_dostupne_statusy(pomenovania_statusov, n);

	int test = -1;
	vypis_popisok("Ktory status chcete nastavit? ");
	scanf("%d", &test);

	if (test >= 0 && test < n) {
		*aktualny_stav = test;
		vypis_uspech("Vas stav bol uspesne nastaveny.\n");

		char buffer[BUFFER_VELKOST];
		sprintf(buffer, "%s#%d", moje_meno, *aktualny_stav);

		DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
		while (aktualny != NULL) {
			if (strcmp(moje_meno, aktualny->data.meno) != 0) {
				chat_akcia_zapis(aktualny->data.socket_id, "ZMEN_STAV", buffer);
			}

			aktualny = aktualny->next;
		}
	} else {
		vypis_chybu("Vas stav nebol uspesne nastaveny.\n");
	}

	zobraz_moj_status(pomenovania_statusov, n, aktualny_stav);
}

/**
 *
 */
void aktualizuj_statusy(DOUBLYLINKEDLIST *list, char *moje_meno) {
	DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
	while (aktualny != NULL) {
		if (strcmp(moje_meno, aktualny->data.meno) != 0) {
			debug_sprava("Posielam ziadost o aktualny stav.");
			chat_akcia_zapis(aktualny->data.socket_id, "ZISKAJ_STAV", moje_meno);
		}

		aktualny = aktualny->next;
	}

	vypis_uspech("Ziadosti boli rozposlane, dajte si zobrazit zoznam pouzivatelov.\n");
}

/**
 *
 */
DOUBLYLINKEDLIST_ITEM *overit_existenciu_pouzivatela(DOUBLYLINKEDLIST *list, int cislo_adresata) {
	// kontrola cisla adresata
	DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
	int i = 0;
	while (aktualny != NULL) {
		if (i == cislo_adresata) {
			break;
		}

		aktualny = aktualny->next;
		i++;
	}

	return aktualny;
}

/**
 *
 */
void odoslat_subor_cli(DOUBLYLINKEDLIST *list, char *moje_meno, char **pomenovania_statusov, int n, struct data_odoslanie_suboru *data) {
	vypis_popisok("Vyberte si adresata spravy: ");
	printf("\n");

	zobraz_list_connect(list, moje_meno, pomenovania_statusov, n);

	printf("\n");
	int cislo_adresata = -1;
	vypis_popisok("Vyberte cislo adresata: ");
	scanf("%d", &cislo_adresata);

	DOUBLYLINKEDLIST_ITEM *aktualny = overit_existenciu_pouzivatela(list, cislo_adresata);
	if (aktualny == NULL) {
		vypis_chybu("Pozadovany adresat neexistuje!\n");
		return;
	}

	char nazov_suboru[BUFFER_VELKOST];
	vypis_popisok("Zadajte nazov suboru: ");
	scanf("%s", nazov_suboru);

	if (skontroluj_subor(nazov_suboru)) {
		// pripravim svoju stranu
		data->cislo_portu_odosielanie = CISLO_PORTU_ODOSIELANIE;
		data->socket_id = aktualny->data.socket_id;
		data->moje_meno = moje_meno;
		memcpy(data->nazov_suboru, nazov_suboru, sizeof(data->nazov_suboru));

		pthread_t thread_subor;
		pthread_create(&thread_subor, NULL, &priprav_socket_odosielanie_subor, data);

		// notifikujem druhu stranu o tom, ze idem posielat
		debug_sprava("Posielam informaciu o tom, ze idem posielat subor.");

		// pthread_join(&thread_subor, NULL);
	} else {
		vypis_chybu("Pozadovany subor pravdepodobne neexistuje.");
	}
}

/**
 *
 */
void invertuj_indikator_subory(bool *indikator_subory) {
	*indikator_subory = !(*indikator_subory);
	if (*indikator_subory == true) {
		vypis_informaciu("Nove nastavenie umoznuje prijimat subory od inych pouzivatelov.");
	} else {
		vypis_informaciu("Nove nastavenie neumoznuje prijimat subory od inych pouzivatelov.");
	}
}

/**
 *
 */
void nespravna_hodnota(void) {
	vypis_chybu("Zadali ste nespravnu hodnotu v menu. Pre pomoc stlacte ?.");
}

/**
 *
 */
void zobraz_subory_adresar_cli(void) {
	char absolutna_cesta[BUFFER_CESTA];
	vypis_popisok("Zadajte absolutnu cestu k adresaru, pre ktory chcete vidiet subory: ");

	getchar();
	fgets(absolutna_cesta, BUFFER_CESTA, stdin);
	// odstranit newline
	*(absolutna_cesta + strlen(absolutna_cesta) - 1) = '\0';

	zobraz_subory_adresar(absolutna_cesta);
}

/**
 *
 */
void ukoncenie_programu(bool *indikator_pokracuj, int chat_socket, int discovery_socket, pthread_t **pole_threadov, int pocet) {
	*indikator_pokracuj = false;
	uzatvor_socket_chat(chat_socket);
	uzatvor_socket_discovery(discovery_socket);

	for (int i = 0; i < pocet; i++) {
		pthread_cancel(*(pole_threadov[i]));
	}
}

