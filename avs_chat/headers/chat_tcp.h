#ifndef CHAT_TCP_H_
#define CHAT_TCP_H_

int nastav_chat_socket(void);
void nastav_chat_bind(int socket_id, int port);

void nastav_accept_chat();
void chat_zapis();
void chat_citaj();

void spracuj_chat();

void uzatvor_socket_chat(int socket_id);

#endif /* CHAT_TCP_H_ */
