#include <stdio.h>
#include <stdlib.h>

#include "../headers/chat_tcp.h"
#include "../headers/discovery_udp.h"

/**
 *
 */
int main(int argc, char **argv) {
	printf("P2P chat - semestralna praca\n\n");

	spracuj_discovery();
	spracuj_chat();

	printf("KONIEC\n");

	return EXIT_SUCCESS;
}
