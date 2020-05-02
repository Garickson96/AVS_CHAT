#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <unistd.h>

#include <pthread.h>

#include "../headers/universal.h"
#include "../headers/chat_tcp.h"
#include "../headers/discovery_udp.h"
#include "../headers/structs_threads.h"

#define BUFFER_UDP_MINI_SIZE 20
#define BUFFER_UDP_SIZE 50

// 10 sekund
#define CAS_SPANKU 10000000
#define FORMAT_SPRAVY "DISCOVERY"

/**
 *
 */
int vytvor_socket_discovery(void) {
	debug_sprava("Vytvaram socket UDP pre discovery.");

	int socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	osetri_chybu("Nepodarilo sa vytvorit socket pre UDP DISCOVERY.", socket_id, -1, false, 0);

	return socket_id;
}

/**
 *
 */
void nastav_broadcast_discovery(int socket_id) {
	debug_sprava("Nastavujem broadcast pre discovery.");

	int broadcast = 1;
	int priznak_status = setsockopt(socket_id, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

	osetri_chybu("Nepodarilo sa nastavit broadcast pre UDP DISCOVERY.", priznak_status, -1, true, socket_id);
}

/**
 * Aby som nemusel prijímať sockety od všetkých programov.
 */
void nastav_discovery_bind(int socket_id, int port) {
	debug_sprava("Nastavujem bind pre socket UDP discovery.");

	struct sockaddr_in adresa_bind;
	memset(&adresa_bind, 0, sizeof(struct sockaddr_in));

	adresa_bind.sin_family = AF_INET;
	adresa_bind.sin_port = htons(port);
	adresa_bind.sin_addr.s_addr = INADDR_ANY;

	int bind_status = bind(socket_id, (struct sockaddr *)&adresa_bind, sizeof(adresa_bind));
	osetri_chybu("Nepodarilo sa nastavit bind pre UDP DISCOVERY.", bind_status, -1, true, socket_id);
}

/**
 *
 */
void uzatvor_socket_discovery(int socket_id) {
	debug_sprava("Uzatvaram socket pre UDP discovery.");
	close(socket_id);
}

/**
 *
 */
void *posli_info_existujem(void *data) {
	struct data_discovery_zistovanie *data_zis = (struct data_discovery_zistovanie *)data;

	bool *indikator_pokracuj = data_zis->indikator_pokracuj;
	char *meno = data_zis->meno;
	int socket_id = data_zis->discovery_socket;
	int port_tcp_server = data_zis->port_tcp_server;
	int port_discovery_receive = data_zis->port_discovery_receive;

	// http://man7.org/linux/man-pages/man3/getifaddrs.3.html
	struct ifaddrs *prva_polozka_zoznamu;
    int stav_adresy = getifaddrs(&prva_polozka_zoznamu);
    osetri_chybu("Nepodarilo sa zistit broadcastove adresy portov.", stav_adresy, -1, false, 0);

    struct ifaddrs *aktualna_polozka = prva_polozka_zoznamu;
	struct sockaddr_in ciel_spravy;
	memset(&ciel_spravy, 0, sizeof(ciel_spravy));

	ciel_spravy.sin_family = AF_INET;
	ciel_spravy.sin_port = htons(port_discovery_receive);

	char buffer[BUFFER_UDP_MINI_SIZE];
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%s %d %s", FORMAT_SPRAVY, port_tcp_server, meno);

	while (*indikator_pokracuj) {
		while (aktualna_polozka != NULL) {
			if (aktualna_polozka->ifa_broadaddr != NULL && aktualna_polozka->ifa_broadaddr->sa_family == AF_INET) {
				debug_sprava("Posielam spravu, ze existujem.");
				memcpy(&ciel_spravy.sin_addr, &(((struct sockaddr_in *)(aktualna_polozka->ifa_broadaddr))->sin_addr), sizeof(struct in_addr));

				int pocet_odoslanych = sendto(socket_id, buffer, strlen(buffer), 0, (struct sockaddr *)&ciel_spravy, sizeof(ciel_spravy));
				osetri_chybu("Nepodarilo sa odoslat spravu, ze existujem - UDP DISCOVERY.", pocet_odoslanych, -1, true, socket_id);
			}

			aktualna_polozka = aktualna_polozka->ifa_next;
		}

		usleep(CAS_SPANKU);
		aktualna_polozka = prva_polozka_zoznamu;
	}

    freeifaddrs(prva_polozka_zoznamu);
    return NULL;

	// toto posle na 255.255.255.255, podla smerovacej tabulky to spracuje cez default route
	/*
	struct sockaddr_in ciel_spravy;
	memset(&ciel_spravy, 0, sizeof(ciel_spravy));

	ciel_spravy.sin_family = AF_INET;
	ciel_spravy.sin_port = htons(PORT_DISCOVERY);
	ciel_spravy.sin_addr.s_addr = INADDR_BROADCAST;

	int pocet_odoslanych = sendto(socket_id, "DISCOVERY", 9, 0, (struct sockaddr *)&ciel_spravy, sizeof(ciel_spravy));
	osetri_chybu("Nepodarilo sa odoslat spravu, ze existujem - UDP DISCOVERY.", pocet_odoslanych, -1, true, socket_id);
	*/
}

/**
 *
 */
void *prijmi_info_existujem(void *data) {
	struct data_discovery *data_dis = (struct data_discovery *)data;
	int socket_id = data_dis->socket_id;
	bool *indikator_pokracuj = data_dis->indikator_pokracuj;
	DOUBLYLINKEDLIST *list_connect = data_dis->list_connect;

	char buffer[BUFFER_UDP_SIZE];
	char temp_buffer[BUFFER_UDP_MINI_SIZE];
	char buffer_meno[BUFFER_UDP_MINI_SIZE];

	struct sockaddr od_koho_informacie;
	socklen_t size_od_koho_informacie = sizeof(od_koho_informacie);

	memset(buffer, 0, sizeof(buffer));
	debug_sprava("Prijimam spravy od ostatnych na zistenie pritomnosti.");

	while (*indikator_pokracuj) {
		memset(&od_koho_informacie, 0, sizeof(od_koho_informacie));

		int pocet_prijatych = recvfrom(socket_id, buffer, sizeof(buffer), 0, &od_koho_informacie, &size_od_koho_informacie);
		osetri_chybu("Nepodarilo sa prijat spravu o inom, ze existuje - UDP DISCOVERY.", pocet_prijatych, -1, true, socket_id);

		if (strstr(buffer, FORMAT_SPRAVY) != NULL) {
			struct sockaddr_in *od_koho = (struct sockaddr_in *)&od_koho_informacie;
			short cislo_portu = 0;

			sscanf(buffer, "%s %hd %s", temp_buffer, &cislo_portu, buffer_meno);
			od_koho->sin_port = htons(cislo_portu);

			if (!existuje_pouzivatel(list_connect, buffer_meno)) {
				debug_sprava("Pripajam noveho pouzivatela...");
				pripoj_sa(&od_koho_informacie, size_od_koho_informacie, list_connect, buffer_meno);
			} else {
				// debug_sprava("Pouzivatel so zadanym menom je uz pripojeny.");
			}
		}

		debug_ip_sprava((struct sockaddr_in *)&od_koho_informacie, buffer);
		memset(buffer, 0, sizeof(buffer));
	}

	return NULL;
}

/**
 *
 */
bool existuje_pouzivatel(DOUBLYLINKEDLIST *list_connect, char *meno) {
	DOUBLYLINKEDLIST_ITEM *aktualny = list_connect->first;
	while (aktualny != NULL) {
		if (strcmp(aktualny->data.meno, meno) == 0) {
			return true;
		}

		aktualny = aktualny->next;
	}

	return false;
}

/**
 *
 */
int spracuj_discovery(bool *indikator_pokracuj, int port_tcp_server, int port_discovery_send, int port_discovery_receive,
		pthread_t *thread_discovery, pthread_t *thread_discovery_posli,
		struct data_discovery *data_pre_spracovanie, struct data_discovery_zistovanie *data_discovery_posli,
		int socket_id_tcp, char *meno, DOUBLYLINKEDLIST *list_connect) {

	debug_sprava("Spustil som slucku pre UDP discovery.");

	int discovery_socket = vytvor_socket_discovery();
	nastav_broadcast_discovery(discovery_socket);
	nastav_discovery_bind(discovery_socket, port_discovery_send);

	data_pre_spracovanie->indikator_pokracuj = indikator_pokracuj;
	data_pre_spracovanie->socket_id = discovery_socket;
	data_pre_spracovanie->list_connect = list_connect;
	pthread_create(thread_discovery, NULL, &prijmi_info_existujem, data_pre_spracovanie);

	data_discovery_posli->indikator_pokracuj = indikator_pokracuj;
	data_discovery_posli->discovery_socket = discovery_socket;
	data_discovery_posli->port_tcp_server = port_tcp_server;
	data_discovery_posli->port_discovery_receive = port_discovery_receive;
	// netreba deep copy
	data_discovery_posli->meno = meno;

	pthread_create(thread_discovery_posli, NULL, &posli_info_existujem, data_discovery_posli);

	return discovery_socket;
}
