#ifndef STRUCTS_THREADS_H_
#define STRUCTS_THREADS_H_

#include <stdbool.h>

#define DLZKA_MENA 50

struct data_discovery {
	int socket_id;

	bool *indikator_pokracuj;
	DOUBLYLINKEDLIST *list_connect;
};

struct data_discovery_zistovanie {
	int discovery_socket;
	int port_tcp_server;
	int port_discovery_receive;

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
};

#endif /* STRUCTS_THREADS_H_ */
