#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <pthread.h>

#include "../headers/universal.h"
#include "../headers/chat_tcp.h"

#define IP_ADDRESS "127.0.0.1"

#define MAXIMUM_LISTEN 50
#define MAX_UDALOSTI 10
#define VELKOST_BUFFRA 100

/**
 *
 */
int nastav_chat_socket(void) {
	debug_sprava("Vytvaram socket TCP pre chat.");

	int socket_descr = socket(AF_INET, SOCK_STREAM, 0);
	osetri_chybu("Nepodarilo sa vytvorit socket pre TCP chat.", socket_descr, -1, false, 0);

	int reuseaddr_flag = 1;
	int status_setsockopt = setsockopt(socket_descr, SOL_SOCKET, SOCK_STREAM, &reuseaddr_flag, sizeof(reuseaddr_flag));
	osetri_chybu("Nepodarilo sa nastavit reuse pre TCP chat.", status_setsockopt, -1, true, socket_descr);

	return socket_descr;
}

/**
 *
 */
void nastav_chat_bind(int socket_id, int port) {
	debug_sprava("Nastavujem bind pre TCP chat.");

	struct sockaddr_in adresa_bind;
	memset(&adresa_bind, 0, sizeof(struct sockaddr_in));

	adresa_bind.sin_family = AF_INET;
	adresa_bind.sin_port = htons(port);
	adresa_bind.sin_addr.s_addr = INADDR_ANY;

	int bind_status = bind(socket_id, (struct sockaddr *)&adresa_bind, sizeof(adresa_bind));
	osetri_chybu("Nepodarilo sa nastavit bind pre TCP chat.", bind_status, -1, true, socket_id);
}

/**
 *
 */
void nastav_chat_listen(int socket_id, int pocet_listen) {
	debug_sprava("Nastavujem listen pre socket.");

	int listen_status = listen(socket_id, pocet_listen);
	osetri_chybu("Nepodarilo sa nastavit listen pre TCP chat.", listen_status, -1, true, socket_id);
}

/**
 *
 */
void *chat_accept(void *data_accept) {
	struct data_accept *data = (struct data_accept *)data_accept;
	int epoll_descriptor = data->epoll_descriptor;
	int socket_id = data->socket_id;
	bool *indikator_pokracuj = data->indikator_pokracuj;
	DOUBLYLINKEDLIST *list_accept = data->list_accept;

	while (*indikator_pokracuj) {
		struct sockaddr od_koho;
		socklen_t velkost_od_koho = sizeof(od_koho);
		memset(&od_koho, 0, sizeof(od_koho));

		int accept_socket = accept(socket_id, &od_koho, &velkost_od_koho);
		osetri_chybu_nekriticka("Nepodaril sa accept - nekriticka chyba.", accept_socket, -1);

		if (accept_socket != -1) {
			ACCEPT_INFO accept_info;
			memcpy(&accept_info.ip_adresa, &od_koho, velkost_od_koho);
			accept_info.socket_id = accept_socket;

			addDLL(list_accept, accept_info);

			struct epoll_event udalosti;
			udalosti.events = EPOLLIN | EPOLLET;
			udalosti.data.fd = accept_socket;
			int epoll_status = epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, accept_socket, &udalosti);
			osetri_chybu("Nepodarilo sa priradit socket do epoll.", epoll_status, -1, true, socket_id);
		}
	}

	return NULL;
}

/**
 *
 */
void pripoj_sa(struct sockaddr *adresa, socklen_t dlzka_adresy, DOUBLYLINKEDLIST *list_connect, char *meno) {
	debug_sprava("Vytvaram socket pre klienta...");
	int socket_klient = nastav_chat_socket();

	debug_sprava("Pripajam sa ako klient...");
	int connect_status = connect(socket_klient, (struct sockaddr *)adresa, (socklen_t)sizeof(*adresa));
	osetri_chybu_nekriticka("Nepodaril sa connect - nekriticka chyba.", connect_status, -1);

	if (connect_status != -1) {
		ACCEPT_INFO connect_info;
		// !!! prehodit pri praci so zadanou mnozinou
		memcpy(&connect_info.ip_adresa, &adresa, dlzka_adresy);
		strncpy(connect_info.meno, meno, DLZKA_MENA);

		connect_info.socket_id = socket_klient;
		addDLL(list_connect, connect_info);
	} else {
		uzatvor_socket_chat(socket_klient);
	}
}


/**
 *
 */
void chat_jeden_zapis(int socket_kam, char *sprava, int dlzka_spravy) {
	int pocet_znakov_zapis = send(socket_kam, sprava, dlzka_spravy, 0);
	osetri_chybu_nekriticka("Nepodaril sa zapis - nekriticka chyba.", pocet_znakov_zapis, -1);
}

/**
 *
 */
void chat_jeden_citaj(int socket_odkoho, char *buffer, int velkost_buffra) {
	int pocet_znakov_citaj = recv(socket_odkoho, buffer, velkost_buffra, 0);
	osetri_chybu_nekriticka("Nepodaril sa citanie - nekriticka chyba.", pocet_znakov_citaj, -1);
}

/**
 *
 */
int vytvor_epoll(int socket_id) {
	// http://man7.org/linux/man-pages/man7/epoll.7.html
	int epoll_descriptor = epoll_create(1);
	osetri_chybu("Nepodarilo sa vytvorit epoll.", epoll_descriptor, -1, true, socket_id);

	return epoll_descriptor;
}


/**
 *
 */
void *chat_spracovanie_sprav(void *data_spracovanie) {
	struct data_read_write *data = (struct data_read_write *)data_spracovanie;
	int epoll_descriptor = data->epoll_descriptor;
	int socket_id = data->socket_id;
	bool *indikator_pokracuj = data->indikator_pokracuj;

	// epoll event pre socket_id
	struct epoll_event udalosti;
	udalosti.events = EPOLLOUT | EPOLLET;
	udalosti.data.fd = socket_id;
	int epoll_status = epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, socket_id, &udalosti);
	osetri_chybu("Nepodarilo sa priradit socket do epoll.", epoll_status, -1, true, socket_id);

	/*
	// todo: problem
	// epoll event pre sockety v connect_list
	DOUBLYLINKEDLIST_ITEM *polozka = connect_list->first;
	while (polozka != NULL) {
		udalosti.events = EPOLLOUT | EPOLLET;
		udalosti.data.fd = polozka->data.socket_id;
		int epoll_status = epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, polozka->data.socket_id, &udalosti);
		osetri_chybu("Nepodarilo sa priradit socket klient do epoll.", epoll_status, -1, true, socket_id);

		polozka = polozka->next;
	}
	*/

	// vykonavanie udalosti, ktore boli zistene cez epoll
	struct epoll_event zoznam_udalosti[MAX_UDALOSTI];
	while (*indikator_pokracuj) {
		// -1 timeout
		int pocet_deskriptorov = 0;
		do {
			pocet_deskriptorov = epoll_wait(epoll_descriptor, zoznam_udalosti, MAX_UDALOSTI, -1);
		} while (pocet_deskriptorov < 0 && errno == EINTR);

		// osetri_chybu("Nepodarilo sa cakat v epoll.", pocet_deskriptorov, -1, true, socket_id);

		int x = 0;
		for (int i = 0; i < pocet_deskriptorov; i++) {
			x = zoznam_udalosti[i].data.fd;
			if (zoznam_udalosti[i].events & EPOLLIN) {
				char buffer[VELKOST_BUFFRA];
				memset(buffer, 0, sizeof(buffer));
				chat_jeden_citaj(zoznam_udalosti[i].data.fd, buffer, VELKOST_BUFFRA);

				printf("Dosla nam spravicka> %s\n", buffer);
			} else if (zoznam_udalosti[i].events & EPOLLOUT) {
				// chat_jeden_zapis(int socket_kam, char *sprava, int dlzka_spravy)
				printf("epollout\n");
			} else {
				printf("Neznamy event v slucke.\n");
			}
		}

		chat_jeden_zapis(x, "nazdar", 7);
	}

	return NULL;
}

/**
 *
 */
void uzatvor_socket_chat(int socket_id) {
	debug_sprava("Uzatvaram socket pre TCP chat.");
	close(socket_id);
}

/**
 *
 */
int spracuj_chat(bool *indikator_pokracuj, int cislo_portu_server, pthread_t *thread_accept, pthread_t *thread_spracovanie, struct data_accept *data_pre_accept, struct data_read_write *data_pre_spracovanie) {
	debug_sprava("Spustam chat ako kvazi server");

	int socket_id = nastav_chat_socket();
	nastav_chat_bind(socket_id, cislo_portu_server);
	nastav_chat_listen(socket_id, MAXIMUM_LISTEN);

	int epoll_descriptor = vytvor_epoll(socket_id);

	data_pre_accept->epoll_descriptor = epoll_descriptor;
	data_pre_accept->socket_id = socket_id;
	data_pre_accept->indikator_pokracuj = indikator_pokracuj;

	data_pre_spracovanie->epoll_descriptor = epoll_descriptor;
	data_pre_spracovanie->socket_id = socket_id;
	data_pre_spracovanie->indikator_pokracuj = indikator_pokracuj;

	pthread_create(thread_accept, NULL, &chat_accept, data_pre_accept);
	pthread_create(thread_spracovanie, NULL, &chat_spracovanie_sprav, data_pre_spracovanie);

	return socket_id;
}
