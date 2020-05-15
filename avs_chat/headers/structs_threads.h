#ifndef STRUCTS_THREADS_H_
#define STRUCTS_THREADS_H_

#include <stdbool.h>

#include "../headers/doubly_linked_list.h"

#define DLZKA_MENA 50

struct data_discovery {
	int socket_id;

	bool *indikator_pokracuj;
	DOUBLYLINKEDLIST *list_connect;
};

struct data_discovery_zistovanie {
	int discovery_socket;
	int port_tcp_server;
	int port_discovery;

	bool *indikator_pokracuj;
	char *meno;
};

struct data_accept {
	int epoll_descriptor;
	int socket_id;

	bool *indikator_pokracuj;
	DOUBLYLINKEDLIST *list_accept;
};

struct data_read_write {
	int epoll_descriptor;
	int socket_id;
	bool *indikator_pokracuj;

	int maximum_stavov;
	DOUBLYLINKEDLIST *list_connect;

	char *moje_meno;
	int *moj_stav;
};

struct data_odoslanie_suboru {
	int cislo_portu_odosielanie;
	int socket_id;

	char *moje_meno;
	char nazov_suboru[100];
};

#endif /* STRUCTS_THREADS_H_ */
