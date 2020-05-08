#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>

#include "../headers/chat_tcp.h"
#include "../headers/discovery_udp.h"
#include "../headers/structs_threads.h"
#include "../headers/universal.h"
#include "../headers/cli.h"

#define MENO_BUFFER 30
#define POLOZKY_THREAD 4

#define POCET_STATUSOV 5

/**
 *
 */
// poradie argumentov <TCP server> <DISCOVERY SERVER>
// predvolene 10100 10001
int main(int argc, char **argv) {
	bool indikator_pokracuj = true;
	pthread_t thread_accept, thread_spracovanie, thread_discovery, thread_discovery_posli;
	pthread_t *pole_thread[POLOZKY_THREAD] = { &thread_accept, &thread_spracovanie, &thread_discovery, &thread_discovery_posli };

	char *pomenovania_statusov[] = { "dostupny", "zaneprazdneny", "nerusit", "nedostupny", "oficialne neznamy" };
	int aktualny_stav = 0;

	if (argc != 3) {
		printf("Vyzaduju sa cisla portov ako argumenty programu.\n");
		printf("Takto: <TCP server> <DISCOVERY SERVER>\n");
		return EXIT_FAILURE;
	}

	// porty a ich cisla
	int cisla_portov[2];
	memset(cisla_portov, 0, sizeof(cisla_portov));
	for (int i = 1; i < argc; i++) {
		cisla_portov[i - 1] = atoi(argv[i]);
		if (cisla_portov[i - 1] < 1 || cisla_portov[i - 1] > 65535) {
			printf("Skontrolujte si parametre programu - chyba v cisle %d (od 1 po 65535).\n", i - 1);
			return EXIT_FAILURE;
		}
	}

	printf("P2P chat - semestralna praca\n\n");

	char meno[MENO_BUFFER];
	memset(meno, 0, sizeof(meno));
	printf("Zadajte meno (jedinecny identifikator): ");
	scanf("%s", meno);



	/**
	 *
	 */
	struct data_discovery *data_discovery = (struct data_discovery *)vytvor_nastav_malloc(sizeof(struct data_discovery), "Nepodarilo sa alokovat priestor pre data_discovery.");
	struct data_discovery_zistovanie *data_discovery_zistovanie = (struct data_discovery_zistovanie *)vytvor_nastav_malloc(sizeof(struct data_discovery_zistovanie), "Nepodarilo sa alokovat priestor pre data_discovery_zistovanie.");

	struct data_accept *data_accept = (struct data_accept *)vytvor_nastav_malloc(sizeof(struct data_accept), "Nepodarilo sa alokovat priestor pre data_accept.");
	struct data_read_write *data_read_write = (struct data_read_write *)vytvor_nastav_malloc(sizeof(struct data_read_write), "Nepodarilo sa alokovat priestor pre data_read_write.");

	DOUBLYLINKEDLIST list_accept, list_connect;
	initDLL(&list_accept);
	initDLL(&list_connect);

	data_accept->list_accept = &list_accept;

	// pridanie sameho seba
	struct accept_info info_ja;
	memset(&info_ja, 0, sizeof(info_ja));
	strncpy(info_ja.meno, meno, sizeof(info_ja.meno));
	addDLL(&list_connect, info_ja);

	/**
	 *
	 */

	int chat_socket = spracuj_chat(&indikator_pokracuj, &aktualny_stav, meno, &list_connect, POCET_STATUSOV, cisla_portov[0], &thread_accept, &thread_spracovanie,
			data_accept, data_read_write);

	int discovery_socket = spracuj_discovery(&indikator_pokracuj, cisla_portov[0], cisla_portov[1],
			&thread_discovery, &thread_discovery_posli, data_discovery, data_discovery_zistovanie, chat_socket, meno, &list_connect);

	/**
	 *
	 */
	while (indikator_pokracuj) {
		char volba = vyber_volby();

		switch (volba) {
			case 'a':
				zobraz_list_accept(&list_accept);
				break;
			case 'b':
				zobraz_list_connect(&list_connect, meno, pomenovania_statusov, POCET_STATUSOV);
				break;
			case 'c':
				posli_spravu(&list_connect, meno, pomenovania_statusov, POCET_STATUSOV);
				break;
			case 'd':
				ukoncenie_programu(&indikator_pokracuj, chat_socket, discovery_socket, pole_thread, POLOZKY_THREAD);
				break;
			case 'e':
				zobraz_moj_status(pomenovania_statusov, POCET_STATUSOV, &aktualny_stav);
				break;
			case 'f':
				zmen_moj_status(pomenovania_statusov, POCET_STATUSOV, &aktualny_stav, &list_connect, meno);
				break;
			case 'g':
				aktualizuj_statusy(&list_connect, meno);
				break;
			default:
				nespravna_hodnota();
				break;
		}
	}

	/**
	 *
	 */
	for (int i = 0; i < POLOZKY_THREAD; i++) {
		pthread_join(*(pole_thread[i]), NULL);
	}

	disposeDLL(&list_connect);
	disposeDLL(data_accept->list_accept);

	dealokuj_malloc((void **)&data_read_write);
	dealokuj_malloc((void **)&data_accept);
	dealokuj_malloc((void **)&data_discovery_zistovanie);
	dealokuj_malloc((void **)&data_discovery);

	printf("KONIEC\n");
	return EXIT_SUCCESS;
}
