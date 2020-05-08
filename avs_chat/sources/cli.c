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

#define BUFFER_VELKOST 50
#define TEXT_VELKOST 160
#define BUFFER_DATUM 30
#define KONCOVA_SPRAVA ":KONIEC"

char vyber_volby(void) {
	char volba;
	printf("Zadajte vasu volbu: ");
	scanf("%c", &volba);

	return volba;
}

void zoznam_menu(void) {

}

char *daj_ip_string(const struct sockaddr *adresa, char *buffer) {
	sprintf(buffer, "%s:%d", inet_ntoa(((struct sockaddr_in *)adresa)->sin_addr), ntohs(((struct sockaddr_in *)adresa)->sin_port));
	return buffer;
}

void zobraz_list_connect(DOUBLYLINKEDLIST *list, char *moje_meno, char **pomenovania_statusov, int n) {
	DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
	printf("CISLO\tMENO\tSOCKET ID\tIP ADRESA\nSTATUS\nCAS AKTUALIZACIE");
	int i = 0;
	char buffer[BUFFER_VELKOST];
	char buffer_datum[BUFFER_DATUM];

	while (aktualny != NULL) {
		if (strcmp(moje_meno, aktualny->data.meno) != 0) {
			strftime(buffer_datum, BUFFER_DATUM, "%d.%m.%Y %H:%M:%S", localtime(&(aktualny->data.cas_nastavenia)));

			printf("%d\t%s\t%d\t%s\t%s\t%s\n", i, aktualny->data.meno, aktualny->data.socket_id,
					daj_ip_string(&(aktualny->data.ip_adresa), buffer),
					pomenovania_statusov[aktualny->data.posledny_stav],
					buffer_datum);
		}

		aktualny = aktualny->next;
		i++;
	}
}

void zobraz_list_accept(DOUBLYLINKEDLIST *list) {
	DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
	printf("CISLO\tMENO\tSOCKET ID\tIP ADRESA\nSTATUS\nCAS AKTUALIZACIE");
	int i = 0;
	char buffer[BUFFER_VELKOST];

	while (aktualny != NULL) {
		printf("%d\t%d\t%s\n", i, aktualny->data.socket_id, daj_ip_string(&(aktualny->data.ip_adresa), buffer));
		aktualny = aktualny->next;
		i++;
	}
}

void posli_spravu(DOUBLYLINKEDLIST *list, char *moje_meno, char **pomenovania_statusov, int n) {
	int cislo_adresata = -1;
	char buffer[TEXT_VELKOST];

	printf("Maximalna velkost spravy je obmedzena na 160 znakov.\nZakazane znaky: >\nPosielanie sprav ukoncte spravou :KONIEC.\n");
	printf("Zadajte komu chcete odoslat spravu: \n");

	zobraz_list_connect(list, moje_meno, pomenovania_statusov, n);

	printf("\nVyberte cislo adresata: \n");
	scanf("%d", &cislo_adresata);

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

	if (aktualny == NULL) {
		printf("Pozadovany adresat neexistuje!\n");
		return;
	}

	do {
		memset(buffer, 0, sizeof(buffer));
		printf("Zadajte spravu: ");
		fgets(buffer, TEXT_VELKOST, stdin);
		buffer[strlen(buffer) - 1] = '\0';

		if (strchr(buffer, ODDELOVAC) != NULL) {
			printf("Sprava obsahuje nepovoleny znak. Nebola odoslana!\n");
			break;
		}

		if (strcmp(buffer, ":KONIEC") != 0) {
			chat_jeden_zapis(aktualny->data.socket_id, buffer, moje_meno);
		}
	} while (strcmp(buffer, ":KONIEC") != 0);
}

void vypis_dostupne_statusy(char **pomenovania_statusov, int n) {
	printf("Vypis moznych statusov: \n");
	for (int i = 0; i < n; i++) {
		printf("%d - %s\n", i, pomenovania_statusov[i]);
	}
}

void zobraz_moj_status(char **pomenovania_statusov, int n, int *aktualny_stav) {
	if (*aktualny_stav >= 0 && *aktualny_stav < n) {
		printf("Vas aktualny stav je: %s.\n", pomenovania_statusov[*aktualny_stav]);
	} else {
		printf("Vas aktualny stav je: neznamy.\n");
	}
}

void zmen_moj_status(char **pomenovania_statusov, int n, int *aktualny_stav, DOUBLYLINKEDLIST *list, char *moje_meno) {
	vypis_dostupne_statusy(pomenovania_statusov, n);

	int test = -1;
	printf("Ktory status chcete nastavit? ");
	scanf("%d", &test);

	if (test >= 0 && test < n) {
		*aktualny_stav = test;
		printf("Vas stav bol uspesne nastaveny.\n");

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
		printf("Vas stav nebol uspesne nastaveny.\n");
	}

	zobraz_moj_status(pomenovania_statusov, n, aktualny_stav);
}

void aktualizuj_statusy(DOUBLYLINKEDLIST *list, char *moje_meno) {
	DOUBLYLINKEDLIST_ITEM *aktualny = list->first;
	while (aktualny != NULL) {
		if (strcmp(moje_meno, aktualny->data.meno) != 0) {
			debug_sprava("Posielam ziadost o aktualny stav.");
			chat_akcia_zapis(aktualny->data.socket_id, "ZISKAJ_STAV", "");
		}

		aktualny = aktualny->next;
	}

	printf("Ziadosti boli rozposlane, dajte si zobrazit zoznam pouzivatelov.\n");
}

void nespravna_hodnota(void) {
	printf("Zadali ste nespravnu hodnotu v menu.\n");
}

void ukoncenie_programu(bool *indikator_pokracuj, int chat_socket, int discovery_socket, pthread_t **pole_threadov, int pocet) {
	*indikator_pokracuj = false;
	uzatvor_socket_chat(chat_socket);
	uzatvor_socket_discovery(discovery_socket);

	for (int i = 0; i < pocet; i++) {
		pthread_cancel(*(pole_threadov[i]));
	}
}

