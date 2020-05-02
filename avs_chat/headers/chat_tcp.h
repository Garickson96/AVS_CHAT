#ifndef CHAT_TCP_H_
#define CHAT_TCP_H_

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>

#include "../headers/doubly_linked_list.h"
#include "../headers/structs_threads.h"

int nastav_chat_socket(void);
void nastav_chat_bind(int socket_id, int port);
void nastav_chat_listen(int socket_id, int pocet_listen);

int spracuj_chat(bool *indikator_pokracuj, int cislo_portu_server, pthread_t *thread_accept, pthread_t *thread_spracovanie,
		struct data_accept *data_pre_accept, struct data_read_write *data_pre_spracovanie);

void pripoj_sa(struct sockaddr *adresa, socklen_t dlzka_adresy, DOUBLYLINKEDLIST *list_connect, char *meno);

void uzatvor_socket_chat(int socket_id);

#endif /* CHAT_TCP_H_ */
