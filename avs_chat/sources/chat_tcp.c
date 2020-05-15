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
#include "../headers/files_processor.h"

#define IP_ADDRESS "127.0.0.1"

#define MAXIMUM_LISTEN 50
#define MAX_UDALOSTI 20
#define VELKOST_BUFFRA 100
#define SPRAVA_S_MENOM 200

#define MINI_BUFFER 30
#define SUPER_MINI_BUFFER 2

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
			// todo> nastavenie mena pre accept
			//strncpy(accept_info.meno, meno, DLZKA_MENA);
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
	int connect_status = connect(socket_klient, adresa, dlzka_adresy);
	osetri_chybu_nekriticka("Nepodaril sa connect - nekriticka chyba.", connect_status, -1);

	if (connect_status != -1) {
		ACCEPT_INFO connect_info;
		// !!! prehodit pri praci so zadanou mnozinou
		memcpy(&connect_info.ip_adresa, adresa, dlzka_adresy);
		strncpy(connect_info.meno, meno, DLZKA_MENA);
		connect_info.socket_id = socket_klient;
		connect_info.posledny_stav = 0;
		connect_info.cas_nastavenia = 0;

		addDLL(list_connect, connect_info);
	} else {
		uzatvor_socket_chat(socket_klient);
	}
}


/**
 *
 */
void chat_jeden_zapis(int socket_kam, char *sprava, char *meno) {
	// todo: nekontrolujeme pretecenie a podobne
	char buffer[SPRAVA_S_MENOM];
	sprintf(buffer, "%s%c %s", meno, ODDELOVAC, sprava);

	int pocet_znakov_zapis = send(socket_kam, buffer, strlen(buffer), 0);
	osetri_chybu_nekriticka("Nepodaril sa zapis - nekriticka chyba.", pocet_znakov_zapis, -1);
}

/**
 *
 */
void chat_akcia_zapis(int socket_kam, char *typ_akcie, char *parametre) {
	char buffer[SPRAVA_S_MENOM];
	sprintf(buffer, "%s%c%s", typ_akcie, ODDELOVAC_AKCIA, parametre);

	int pocet_znakov_zapis = send(socket_kam, buffer, strlen(buffer), 0);
	printf("%s\n", buffer);
	osetri_chybu_nekriticka("Nepodaril sa zapis akcie - nekriticka chyba.", pocet_znakov_zapis, -1);
}

/**
 *
 */
int chat_jeden_citaj(int socket_odkoho, char *buffer, int velkost_buffra) {
	int pocet_znakov_citaj = recv(socket_odkoho, buffer, velkost_buffra, 0);
	osetri_chybu_nekriticka("Nepodaril sa citanie - nekriticka chyba.", pocet_znakov_citaj, -1);

	return pocet_znakov_citaj;
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
	// int socket_id = data->socket_id;
	int maximum_stavov = data->maximum_stavov;
	DOUBLYLINKEDLIST *list_connect = data->list_connect;
	bool *indikator_pokracuj = data->indikator_pokracuj;
	char *moje_meno = data->moje_meno;
	int *moj_stav = data->moj_stav;

	// epoll event pre socket_id
	/*
	struct epoll_event udalosti;
	udalosti.events = EPOLLOUT | EPOLLET;
	udalosti.data.fd = socket_id;
	int epoll_status = epoll_ctl(epoll_descriptor, EPOLL_CTL_ADD, socket_id, &udalosti);
	osetri_chybu("Nepodarilo sa priradit socket do epoll.", epoll_status, -1, true, socket_id);
	*/

	// vykonavanie udalosti, ktore boli zistene cez epoll
	struct epoll_event zoznam_udalosti[MAX_UDALOSTI];
	while (*indikator_pokracuj) {
		// -1 timeout
		int pocet_deskriptorov = 0;
		do {
			pocet_deskriptorov = epoll_wait(epoll_descriptor, zoznam_udalosti, MAX_UDALOSTI, -1);
		} while (pocet_deskriptorov < 0 && errno == EINTR);

		for (int i = 0; i < pocet_deskriptorov; i++) {
			if (zoznam_udalosti[i].events & EPOLLIN) {
				char buffer[VELKOST_BUFFRA];
				memset(buffer, 0, sizeof(buffer));

				chat_jeden_citaj(zoznam_udalosti[i].data.fd, buffer, VELKOST_BUFFRA);
				printf("%s\n", buffer);

				if (strchr(buffer, ODDELOVAC) == NULL) {
					char prikaz[MINI_BUFFER];
					char parametre[MINI_BUFFER];
					sscanf(buffer, "%[^'~']~%s", prikaz, parametre);

					if (strcmp(prikaz, "ZISKAJ_STAV") == 0) {
						// dodat tu - ziskaj_stav
						// vygeneruje spravu - zmen_stav = meno a stav
						debug_sprava("Informujem o svojom stave ineho pouzivatela...");
						char buffer_odoslat[VELKOST_BUFFRA];
						sprintf(buffer_odoslat, "%s#%d", moje_meno, *moj_stav);

						DOUBLYLINKEDLIST_ITEM *aktualny = list_connect->first;
						while (aktualny != NULL) {
							if (strcmp(parametre, aktualny->data.meno) == 0) {
								break;
							}

							aktualny = aktualny->next;
						}

						chat_akcia_zapis(aktualny->data.socket_id, "ZMEN_STAV", buffer_odoslat);

					} else if (strcmp(prikaz, "ZMEN_STAV") == 0) {
						// dodat tu - zmen_stav
						// nereagujem na to spravou
						printf("%s\n", buffer);
						debug_sprava("Zapisujem informaciu o zmene stavu ineho pouzivatela...");
						char meno[MINI_BUFFER];
						int stav_skonvertovany = -1;
						sscanf(parametre, "%[^'#']#%d", meno, &stav_skonvertovany);

						if (stav_skonvertovany >= 0 && stav_skonvertovany < maximum_stavov) {
							DOUBLYLINKEDLIST_ITEM *aktualny = list_connect->first;
							bool je_ok = false;
							while (aktualny != NULL) {
								if (strcmp(aktualny->data.meno, meno) == 0) {
									aktualny->data.posledny_stav = stav_skonvertovany;
									aktualny->data.cas_nastavenia = time(NULL);
									je_ok = true;
									break;
								}

								aktualny = aktualny->next;
							}

							if (je_ok) {
								printf("Zmena informacie bola uspesne vykonana.\n");
							} else {
								printf("Pozadovany pouzivatel pre zmenu informacie o stave sa nenasiel.\n");
							}
						} else {
							printf("Neplatny prikaz pre zmen stav!\n");
						}


					} else if (strcmp(prikaz, "SUBOR") == 0) {
						// dodat akcie, ak nam pride sprava SUBOR
						// mal by som vytvorit connect
						debug_sprava("Prisla mi sprava, ze niekto chce poslat subor...");

						char meno[MINI_BUFFER];
						char nazov_suboru[VELKOST_BUFFRA];
						size_t velkost_suboru;
						sscanf(parametre, "%[^'#']#%[^'#']#%ld", nazov_suboru, meno, &velkost_suboru);

						// todo> dodat tu moznost vyberu / problem s citanim
						printf("Idete prijat subor %s od %s s velkostou %ld B.\n", nazov_suboru, meno, velkost_suboru);

						// vyhladanie IP adresy v zozname connect
						// todo> rozhodit do funkcie
						DOUBLYLINKEDLIST_ITEM *aktualny = list_connect->first;
						while (aktualny != NULL) {
							if (strcmp(meno, aktualny->data.meno) == 0) {
								break;
							}

							aktualny = aktualny->next;
						}

						// todo> prerobit na thready
						struct sockaddr ip_adresa;
						memcpy(&ip_adresa, &(aktualny->data.ip_adresa), sizeof(ip_adresa));
						((struct sockaddr_in *)&ip_adresa)->sin_port = htons(9001);

						priprav_socket_prijimanie_subor(ip_adresa, sizeof(ip_adresa), nazov_suboru);

					} else if (strcmp(prikaz, "SUBOR_ACK") == 0) {
						// dodat akcie, ak nam pride sprava SUBOR_ACK
						// mal by som zacat posielat subor

					} else {
						printf("Prisla nerozlustitelna sprava!\n");
						printf("%s\n", buffer);
					}
				} else {
					printf("Sprava od %s\n", buffer);
				}
			} else {
				printf("Neznamy event v slucke.\n");
			}
		}
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
int spracuj_chat(bool *indikator_pokracuj, int *moj_stav, char *moje_meno, DOUBLYLINKEDLIST *list_connect, int maximum_stavov, int cislo_portu_server,
		pthread_t *thread_accept, pthread_t *thread_spracovanie,
		struct data_accept *data_pre_accept, struct data_read_write *data_pre_spracovanie) {
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
	data_pre_spracovanie->maximum_stavov = maximum_stavov;
	data_pre_spracovanie->list_connect = list_connect;
	data_pre_spracovanie->moje_meno = moje_meno;
	data_pre_spracovanie->moj_stav = moj_stav;

	pthread_create(thread_accept, NULL, &chat_accept, data_pre_accept);
	pthread_create(thread_spracovanie, NULL, &chat_spracovanie_sprav, data_pre_spracovanie);

	return socket_id;
}
