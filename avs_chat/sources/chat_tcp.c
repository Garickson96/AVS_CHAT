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
#include "../headers/hudba_chat.h"

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
	debug_sprava("Vytvaram socket TCP.");

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

	while (*data->indikator_pokracuj) {
		struct sockaddr od_koho;
		socklen_t velkost_od_koho = sizeof(od_koho);
		memset(&od_koho, 0, sizeof(od_koho));

		int accept_socket = accept(data->socket_id, &od_koho, &velkost_od_koho);
		osetri_chybu_nekriticka("Nepodaril sa accept - nekriticka chyba.", accept_socket, -1);

		if (accept_socket != -1) {
			ACCEPT_INFO accept_info;
			memcpy(&accept_info.ip_adresa, &od_koho, velkost_od_koho);
			// todo> nastavenie mena pre accept
			//strncpy(accept_info.meno, meno, DLZKA_MENA);
			accept_info.socket_id = accept_socket;

			addDLL(data->list_accept, accept_info);

			struct epoll_event udalosti;
			udalosti.events = EPOLLIN | EPOLLET;
			udalosti.data.fd = accept_socket;
			int epoll_status = epoll_ctl(data->epoll_descriptor, EPOLL_CTL_ADD, accept_socket, &udalosti);
			osetri_chybu("Nepodarilo sa priradit socket do epoll.", epoll_status, -1, true, data->socket_id);
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
	debug_sprava_rozsirena(buffer);
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
void akcia_ziskaj_stav(char *moje_meno, int *moj_stav, DOUBLYLINKEDLIST *list_connect, char *parametre) {
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
}

/**
 *
 */
void akcia_zmen_stav(char *parametre, DOUBLYLINKEDLIST *list_connect, int maximum_stavov) {
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
			vypis_uspech("Zmena informacie bola uspesne vykonana.");
		} else {
			vypis_chybu("Pozadovany pouzivatel pre zmenu informacie o stave sa nenasiel.");
		}
	} else {
		vypis_chybu("Neplatny prikaz pre zmen stav!");
	}
}

/**
 *
 */
void akcia_subor(sfSoundBuffer *hudba_buffer_sprava, char *parametre, DOUBLYLINKEDLIST *list_connect, bool *indikator_subory) {
	// dodat akcie, ak nam pride sprava SUBOR
							// mal by som vytvorit connect
							debug_sprava("Prisla mi sprava, ze niekto chce poslat subor...");
							prehraj_zvuk_sprava(hudba_buffer_sprava);

							char meno[MINI_BUFFER];
							char nazov_suboru[VELKOST_BUFFRA];
							size_t velkost_suboru;
							sscanf(parametre, "%[^'#']#%[^'#']#%ld", nazov_suboru, meno, &velkost_suboru);

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

							struct sockaddr ip_adresa;
							memcpy(&ip_adresa, &(aktualny->data.ip_adresa), sizeof(ip_adresa));
							((struct sockaddr_in *)&ip_adresa)->sin_port = htons(9001);

							priprav_socket_prijimanie_subor(ip_adresa, sizeof(ip_adresa), nazov_suboru, indikator_subory);

}

void akcia_odhlasovanie_pouzivatela(sfSoundBuffer *hudba_buffer_odhlasenie, char *parametre, DOUBLYLINKEDLIST *list_connect, DOUBLYLINKEDLIST *list_accept, int fd) {
	// odhlasujem pouzivatela
	debug_sprava("Odhlasujem pouzivatela... Zacinam vyhladavat informacie v zoznamoch.");
	prehraj_zvuk_odhlasenie(hudba_buffer_odhlasenie);

	DOUBLYLINKEDLIST_ITEM *aktualny_accept = list_accept->first;
	int pozicia_accept = 0;
	while (aktualny_accept != NULL) {
		if (fd == aktualny_accept->data.socket_id) {
			break;
		}

		aktualny_accept = aktualny_accept->next;
		pozicia_accept++;
	}

	// vymazanie z obidvoch zoznamov
	// najprv spravit vymazanie z druheho zoznamu
	DOUBLYLINKEDLIST_ITEM *aktualny_connect = list_connect->first;
	int hodnota_accept = ((struct sockaddr_in *)&aktualny_accept->data.ip_adresa)->sin_addr.s_addr;
	int pozicia_connect = 0;

	while (aktualny_connect != NULL) {
		int hodnota_connect = ((struct sockaddr_in *)&aktualny_connect->data.ip_adresa)->sin_addr.s_addr;
		if (hodnota_connect == hodnota_accept) {
			break;
		}

		aktualny_connect = aktualny_connect->next;
		pozicia_connect++;
	}

	// to ze sa to najde v zozname je zarucene
	ACCEPT_INFO vystup;
	bool status_accept = tryRemoveDLL(list_accept, pozicia_accept, &vystup);
	bool status_connect = tryRemoveDLL(list_connect, pozicia_connect, &vystup);

	if (status_connect && status_accept) {
		char buffer[SPRAVA_S_MENOM];
		memset(buffer, '\0', SPRAVA_S_MENOM);

		sprintf(buffer, "Pouzivatel s menom %s bol vymazany zo zonamu - odhlasenie.", vystup.meno);
		debug_sprava(buffer);
	} else {
		debug_sprava("Pouzivatel nebol vymazany zo zoznamov. Nastala neznama chyba.");
	}
}


/**
 *
 */
void *chat_spracovanie_sprav(void *data_spracovanie) {
	struct data_read_write *data = (struct data_read_write *)data_spracovanie;
	struct epoll_event zoznam_udalosti[MAX_UDALOSTI];

	// vykonavanie udalosti, ktore boli zistene cez epoll
	while (*data->indikator_pokracuj) {
		// cakanie na udalost
		int pocet_deskriptorov = 0;
		do {
			pocet_deskriptorov = epoll_wait(data->epoll_descriptor, zoznam_udalosti, MAX_UDALOSTI, -1);
		} while (pocet_deskriptorov < 0 && errno == EINTR && *(data->indikator_pokracuj) == true);

		// vykonanie udalosti
		for (int i = 0; i < pocet_deskriptorov; i++) {
			if (zoznam_udalosti[i].events & EPOLLIN) {
				char buffer[VELKOST_BUFFRA];
				memset(buffer, 0, sizeof(buffer));

				chat_jeden_citaj(zoznam_udalosti[i].data.fd, buffer, VELKOST_BUFFRA);
				debug_sprava_rozsirena(buffer);

				if (strchr(buffer, ODDELOVAC) == NULL) {
					char prikaz[MINI_BUFFER];
					char parametre[MINI_BUFFER];
					sscanf(buffer, "%[^'~']~%s", prikaz, parametre);

					if (strcmp(prikaz, "ZISKAJ_STAV") == 0) {
						akcia_ziskaj_stav(data->moje_meno, data->moj_stav, data->list_connect, parametre);
					} else if (strcmp(prikaz, "ZMEN_STAV") == 0) {
						akcia_zmen_stav(parametre, data->list_connect, data->maximum_stavov);
					} else if (strcmp(prikaz, "SUBOR") == 0) {
						akcia_subor(data->hudba_buffer_sprava, parametre, data->list_connect, data->indikator_subory);
					} else {
						if (strlen(buffer) == 0) {
							akcia_odhlasovanie_pouzivatela(data->hudba_buffer_odhlasenie, parametre, data->list_connect, data->list_accept, zoznam_udalosti[i].data.fd);
						} else {
							vypis_chybu("Prisla nerozlustitelna sprava!");
							debug_sprava(buffer);
						}
					}
				} else {
					prehraj_zvuk_sprava(data->hudba_buffer_sprava);
					printf("Sprava od %s\n", buffer);
				}
			} else {
				vypis_chybu("Neznamy event v slucke!");
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
int spracuj_chat(bool *indikator_pokracuj, bool *indikator_subory, int *moj_stav, char *moje_meno, DOUBLYLINKEDLIST *list_connect,
		DOUBLYLINKEDLIST *list_accept, int maximum_stavov, int cislo_portu_server, pthread_t *thread_accept, pthread_t *thread_spracovanie,
		struct data_accept *data_pre_accept, struct data_read_write *data_pre_spracovanie, sfSoundBuffer *hudba_buffer_sprava, sfSoundBuffer *hudba_buffer_odhlasenie) {
	debug_sprava("Spustam chat ako kvazi server");

	int socket_id = nastav_chat_socket();
	nastav_chat_bind(socket_id, cislo_portu_server);
	nastav_chat_listen(socket_id, MAXIMUM_LISTEN);

	int epoll_descriptor = vytvor_epoll(socket_id);

	//
	data_pre_accept->epoll_descriptor = epoll_descriptor;
	data_pre_accept->socket_id = socket_id;
	data_pre_accept->indikator_pokracuj = indikator_pokracuj;

	//
	data_pre_spracovanie->epoll_descriptor = epoll_descriptor;
	data_pre_spracovanie->socket_id = socket_id;
	data_pre_spracovanie->indikator_pokracuj = indikator_pokracuj;
	data_pre_spracovanie->indikator_subory = indikator_subory;
	data_pre_spracovanie->maximum_stavov = maximum_stavov;

	data_pre_spracovanie->list_connect = list_connect;
	data_pre_spracovanie->list_accept = list_accept;

	data_pre_spracovanie->hudba_buffer_sprava = hudba_buffer_sprava;
	data_pre_spracovanie->hudba_buffer_odhlasenie = hudba_buffer_odhlasenie;

	data_pre_spracovanie->moje_meno = moje_meno;
	data_pre_spracovanie->moj_stav = moj_stav;

	//
	pthread_create(thread_accept, NULL, &chat_accept, data_pre_accept);
	pthread_create(thread_spracovanie, NULL, &chat_spracovanie_sprav, data_pre_spracovanie);

	return socket_id;
}
