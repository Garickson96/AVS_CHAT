#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <unistd.h>

#include "../headers/universal.h"
#include "../headers/chat_tcp.h"

#define PORT 10101
#define IP_ADDRESS "127.0.0.1"

/**
 *
 */
int nastav_chat_socket(void) {
	debug_sprava("Vytvaram socket TCP pre chat.");

	int socket_descr = socket(AF_INET, SOCK_STREAM, 0);
	osetri_chybu("Nepodarilo sa vytvorit socket pre TCP chat.", socket_descr, -1, false, 0);

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
void nastav_accept_chat() {}

/**
 *
 */
void chat_zapis() {}

/**
 *
 */
void chat_citaj() {}

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
void spracuj_chat() {
	debug_sprava("Spustil som slucku pre TCP chat.");

	int socket_id = nastav_chat_socket();
	nastav_chat_bind(socket_id, PORT);

	uzatvor_socket_chat(socket_id);

	/*
	char buffer[255];
	struct sockaddr_in struct_vstup;

	memset(buffer, 0, sizeof(buffer));
	memset(&struct_vstup, 0, sizeof(struct_vstup));

	for (;;) {
		// receive message
		// tu ziskam informacie od koho to mam a pre unicast si to potrebujem uchovat, aby som vedel, kam poslat odpoved
		socklen_t addr_len = sizeof(struct_vstup);
		int pocet_prijatych = recvfrom(socket_descr, buffer, sizeof(buffer), 0, (struct sockaddr *)&struct_vstup, &addr_len);
		if (pocet_prijatych < 0) {
			perror("chyba recvfrom");
			close(socket_descr);
			return EXIT_FAILURE;
		}

		// spracovanie zobrazenia spravy
		char *pole_ip = inet_ntoa(struct_vstup.sin_addr);
		printf("PRIJATA SPRAVA od %s: %s\n", pole_ip, buffer);

		// response
		// v sendto uz nejde smernik
		//int pocet_odoslanych = sendto(socket_descr, buffer, sizeof(buffer), 0, (struct sockaddr *)&struct_vstup, addr_len);
		/*if (pocet_odoslanych < 0) {
			perror("chyba send");
			close(socket_descr);
			return EXIT_FAILURE;
		}
		printf("ODOSLANA SPRAVA\n");
	}

	close(socket_descr);
	printf("KONIEC\n");
	 */
	//return EXIT_SUCCESS;
}
