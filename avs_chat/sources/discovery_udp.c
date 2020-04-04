#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <ifaddrs.h>
#include <unistd.h>

#include "../headers/universal.h"
#include "../headers/discovery_udp.h"

#define PORT_DISCOVERY_SEND 10001
#define PORT_DISCOVERY_RECEIVE 10002

#define BUFFER_UDP_SIZE 50

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
	adresa_bind.sin_port = htons(PORT_DISCOVERY_SEND);
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
void posli_info_existujem(int socket_id) {
	// http://man7.org/linux/man-pages/man3/getifaddrs.3.html
	struct ifaddrs *prva_polozka_zoznamu;
    int stav_adresy = getifaddrs(&prva_polozka_zoznamu);
    osetri_chybu("Nepodarilo sa zistit broadcastove adresy portov.", stav_adresy, -1, false, 0);

    struct ifaddrs *aktualna_polozka = prva_polozka_zoznamu;
	struct sockaddr_in ciel_spravy;
	memset(&ciel_spravy, 0, sizeof(ciel_spravy));

	ciel_spravy.sin_family = AF_INET;
	ciel_spravy.sin_port = htons(PORT_DISCOVERY_RECEIVE);

    while (aktualna_polozka != NULL) {
    	if (aktualna_polozka->ifa_broadaddr != NULL && aktualna_polozka->ifa_broadaddr->sa_family == AF_INET) {
    		debug_sprava("Posielam spravu, ze existujem.");
    		memcpy(&ciel_spravy.sin_addr, &(((struct sockaddr_in *)(aktualna_polozka->ifa_broadaddr))->sin_addr), sizeof(struct in_addr));

    		int pocet_odoslanych = sendto(socket_id, "DISCOVERY", 9, 0, (struct sockaddr *)&ciel_spravy, sizeof(ciel_spravy));
    		osetri_chybu("Nepodarilo sa odoslat spravu, ze existujem - UDP DISCOVERY.", pocet_odoslanych, -1, true, socket_id);
    	}

    	aktualna_polozka = aktualna_polozka->ifa_next;
    }

    freeifaddrs(prva_polozka_zoznamu);

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
void prijmi_info_existujem(int socket_id) {
	char buffer[BUFFER_UDP_SIZE];
	struct sockaddr od_koho_informacie;
	socklen_t size_od_koho_informacie = sizeof(od_koho_informacie);

	memset(buffer, 0, sizeof(buffer));
	debug_sprava("Prijimam spravy od ostatnych na zistenie pritomnosti.");

	for (;;) {
		memset(&od_koho_informacie, 0, sizeof(od_koho_informacie));

		int pocet_prijatych = recvfrom(socket_id, buffer, sizeof(buffer), 0, &od_koho_informacie, &size_od_koho_informacie);
		osetri_chybu("Nepodarilo sa prijat spravu o inom, ze existuje - UDP DISCOVERY.", pocet_prijatych, -1, true, socket_id);

		// TODO: dodat tu akcie, co sa maju spravit pri prijati
		debug_ip_sprava((struct sockaddr_in *)&od_koho_informacie, buffer);
		memset(buffer, 0, sizeof(buffer));
	}
}

/**
 *
 */
void spracuj_discovery(void) {
	debug_sprava("Spustil som slucku pre UDP discovery.");

	int discovery_socket = vytvor_socket_discovery();
	nastav_broadcast_discovery(discovery_socket);
	nastav_discovery_bind(discovery_socket, PORT_DISCOVERY_SEND);

	posli_info_existujem(discovery_socket);
	prijmi_info_existujem(discovery_socket);

	uzatvor_socket_discovery(discovery_socket);
}
