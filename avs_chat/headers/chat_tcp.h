#ifndef CHAT_TCP_H_
#define CHAT_TCP_H_

#include <sys/types.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>

#include "../headers/structs_threads.h"

int nastav_chat_socket(void);
void nastav_chat_bind(int socket_id, int port);
void nastav_chat_listen(int socket_id, int pocet_listen);

void chat_jeden_zapis(int socket_kam, const char *sprava, const char *meno);
int chat_jeden_citaj(int socket_odkoho, char *buffer, int velkost_buffra);

void chat_akcia_zapis(int socket_kam, const char *typ_akcie, const char *parametre);

int spracuj_chat(bool *indikator_pokracuj, bool *indikator_subory, int *moj_stav, char *moje_meno, DOUBLYLINKEDLIST *list_connect,
		DOUBLYLINKEDLIST *list_accept, int maximum_stavov, int cislo_portu_server, pthread_t *thread_accept, pthread_t *thread_spracovanie,
		struct data_accept *data_pre_accept, struct data_read_write *data_pre_spracovanie, sfSoundBuffer *hudba_buffer_sprava, sfSoundBuffer *hudba_buffer_odhlasenie);

void pripoj_sa(struct sockaddr *adresa, socklen_t dlzka_adresy, DOUBLYLINKEDLIST *list_connect, const char *meno);

void uzatvor_socket_chat(int socket_id);

#endif /* CHAT_TCP_H_ */
