#ifndef DISCOVERY_SERVER_H_
#define DISCOVERY_SERVER_H_

#include "../headers/structs_threads.h"

int vytvor_socket_discovery(void);
void nastav_broadcast_discovery(int socket_id);
void nastav_discovery_bind(int socket_id, int port);

void *posli_info_existujem(void *data);
void *prijmi_info_existujem(void *data);

bool existuje_pouzivatel(DOUBLYLINKEDLIST *list_connect, char *meno);
void uzatvor_socket_discovery(int socket_id);

int spracuj_discovery(bool *indikator_pokracuj, int port_tcp_server, int port_discovery,
		pthread_t *thread_discovery, pthread_t *thread_discovery_posli,
		struct data_discovery *data_pre_spracovanie, struct data_discovery_zistovanie *data_discovery_posli,
		int socket_id_tcp, char *meno, DOUBLYLINKEDLIST *list_connect);

#endif /* DISCOVERY_SERVER_H_ */
