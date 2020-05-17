#ifndef STRUCTS_THREADS_H_
#define STRUCTS_THREADS_H_

#include <stdbool.h>
#include <SFML/Audio.h>

#include "../headers/doubly_linked_list.h"

#define NAZOV_SUBORU 100

/**
 * Data pre vlakno s discovery na prijem sprav. Obsahuje socket ID pre UDP prevadzku, indikator spustenia programu a zoznam pripojenych pouzivatelov cez connect (na zaklade toho
 * vytvaram spojenia na inych pouzivatelov). Connect spojenie sa pouziva v smere VON zo zariadenia.
 */
struct data_discovery {
	int socket_id;

	bool *indikator_pokracuj;
	DOUBLYLINKEDLIST *list_connect;
};

/**
 * Data pre vlakno s discovery na odosielanie sprav. Obsahuje socket ID pre UDP prevadzku, dalej cisla portov pre TCP server, ktory sa vklada do odosielanej spravy. UDP port
 * je pre spravne informovanie pouzivatela v debug spravach. Dalej tam je indikator pouzivatela, meno pouzivatela (odosiela sa v sprave) a zoznam rozhrani na pocitaci, pre
 * ktore sa bude posielat broadcast.
 */
struct data_discovery_zistovanie {
	int discovery_socket;
	int port_tcp_server;
	int port_discovery;

	bool *indikator_pokracuj;
	char *meno;
	struct ifaddrs *prva_polozka_zoznamu;
};

/**
 * V accept potrebujem deskriptor pre epoll - aby som zacal pocuvat pre zadaneho pouzivatela, dalej socket ID pre TCP prevadzku. Ostatne parametre maju podobny
 * vyznam ako v predoslych strukturach.
 */
struct data_accept {
	int epoll_descriptor;
	int socket_id;

	bool *indikator_pokracuj;
	DOUBLYLINKEDLIST *list_accept;
};

/**
 * Pre vlakno na citanie a zapis sprav potrebujeme deskriptor pre epoll (aby sme vedeli rozlisit, kto to posielal), socket ID pre TCP komunikaciu, buffre pre hudbu, zoznamy pre zapisovanie
 * informacii o prihlasovanych uzivateloch a informacie o globalnych nastaveniach programu - moje meno, moj stav, indikatory pre stav programu a posielania suborov.
 */
struct data_read_write {
	int epoll_descriptor;
	int socket_id;
	int maximum_stavov;

	bool *indikator_pokracuj;
	bool *indikator_subory;

	DOUBLYLINKEDLIST *list_connect;
	DOUBLYLINKEDLIST *list_accept;

	sfSoundBuffer *hudba_buffer_sprava;
	sfSoundBuffer *hudba_buffer_odhlasenie;

	char *moje_meno;
	int *moj_stav;
};

/**
 * Pre odoslanie suboru (informacie pre vlakno) potrebujeme ID TCP socketu, ktory je urceny na odosielanie suborov, cislo TCP portu 9001. Zvysne dva parametre sa pouzivaju
 * pre posielanie informacii o danom subore.
 */
struct data_odoslanie_suboru {
	int cislo_portu_odosielanie;
	int socket_id;

	char *moje_meno;
	char nazov_suboru[NAZOV_SUBORU];
};

#endif /* STRUCTS_THREADS_H_ */
