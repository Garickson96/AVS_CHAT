#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>

#include "../headers/chat_tcp.h"
#include "../headers/discovery_udp.h"
#include "../headers/structs_threads.h"
#include "../headers/universal.h"

#define MENO_BUFFER 30

/**
 *
 */
// poradie argumentov <TCP server> <DISCOVERY SERVER> <DISCOVERY KLIENT>
// predvolene 10100 10001 10002
int main(int argc, char **argv) {
	bool indikator_pokracuj = true;
	pthread_t thread_accept, thread_spracovanie, thread_discovery, thread_discovery_posli;

	if (argc != 4) {
		printf("Vyzaduju sa cisla portov ako argumenty programu.\n");
		printf("Takto: <TCP server> <DISCOVERY SERVER> <DISCOVERY KLIENT>\n");
		return EXIT_FAILURE;
	}

	// porty a ich cisla
	int cisla_portov[3];
	for (int i = 0; i < sizeof(cisla_portov)/sizeof(int); i++) {
		cisla_portov[i] = atoi(argv[i + 1]);
		if (cisla_portov[i] < 1 || cisla_portov[i] > 65535) {
			printf("Skontrolujte si parametre programu - chyba v cisle %d (od 1 po 65535).\n", i + 1);
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

	/**
	 *
	 */
	int chat_socket = spracuj_chat(&indikator_pokracuj, cisla_portov[0], &thread_accept, &thread_spracovanie,
			data_accept, data_read_write);

	int discovery_socket = spracuj_discovery(&indikator_pokracuj, cisla_portov[0], cisla_portov[1], cisla_portov[2],
			&thread_discovery, &thread_discovery_posli, data_discovery, data_discovery_zistovanie, chat_socket, meno, &list_connect);

	/**
	 *
	 */
	while (indikator_pokracuj) {
		char znak;
		scanf("%c", &znak);
		printf("Zadali ste znak> %c\n", znak);

		if (znak == 'e') {
			indikator_pokracuj = false;

			uzatvor_socket_chat(chat_socket);
			uzatvor_socket_discovery(discovery_socket);

			// todo> preverit
			pthread_cancel(thread_discovery_posli);
			pthread_cancel(thread_discovery);
			pthread_cancel(thread_accept);
			pthread_cancel(thread_spracovanie);
		}
	}

	/**
	 *
	 */
	pthread_join(thread_discovery_posli, NULL);
	pthread_join(thread_discovery, NULL);
	pthread_join(thread_spracovanie, NULL);
	pthread_join(thread_accept, NULL);

	disposeDLL(&list_connect);
	disposeDLL(data_accept->list_accept);

	dealokuj_malloc((void **)&data_read_write);
	dealokuj_malloc((void **)&data_accept);
	dealokuj_malloc((void **)&data_discovery_zistovanie);
	dealokuj_malloc((void **)&data_discovery);

	printf("KONIEC\n");
	return EXIT_SUCCESS;
}
