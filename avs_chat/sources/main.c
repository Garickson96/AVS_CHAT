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

#define PORT 6000
#define IP_ADDRESS "127.0.0.1"

/**
 * Pri true - kriticka chyba, uzatvor program
 */
void osetri_chybu(char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota, bool zavri_spojenie, int socket_descr) {
	if (hodnota_porovnaj == chybova_hodnota) {
		perror(popis_chyby);
		if (zavri_spojenie) {
			close(socket_descr);
		}

		exit(EXIT_FAILURE);
	}
}

/**
 *
 */
int nastav_socket(void) {
	int socket_descr = socket(AF_INET, SOCK_STREAM, 0);
	osetri_chybu("Nepodarilo sa vytvorit socket.", socket_descr, -1, false, 0);

	return socket_descr;
}

/**
 * mozno osetrit nespravne hodnoty?
 */
void nastav_bind(int socket_id, int port) {
	// nezabudat na prehadzovanie
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	// operacia bind
	int bind_status = bind(socket_id, (struct sockaddr *)&addr, sizeof(addr));
	osetri_chybu("Nepodarilo sa nastavit bind.", bind_status, -1, true, socket_id);
}

/**
 *
 */
void nastav_accept() {}

/**
 *
 */
void zapis() {}

/**
 *
 */
void citaj() {}

/**
 *
 */
void uzatvor_socket() {}


int main(int argc, char **argv) {

	int socket_descr = nastav_socket();
	nastav_bind(socket_descr, PORT);

	printf("SERVER UDP PO BIND\n");
	// echo server
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
		int pocet_odoslanych = sendto(socket_descr, buffer, sizeof(buffer), 0, (struct sockaddr *)&struct_vstup, addr_len);
		if (pocet_odoslanych < 0) {
			perror("chyba send");
			close(socket_descr);
			return EXIT_FAILURE;
		}
		printf("ODOSLANA SPRAVA\n");
	}

	close(socket_descr);
	printf("KONIEC\n");

	return EXIT_SUCCESS;
}
