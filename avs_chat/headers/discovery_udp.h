#ifndef DISCOVERY_SERVER_H_
#define DISCOVERY_SERVER_H_

int vytvor_socket_discovery(void);
void nastav_broadcast_discovery(int socket_id);
void nastav_discovery_bind(int socket_id, int port);

void posli_info_existujem(int socket_id);

void uzatvor_socket_discovery(int socket_id);
void spracuj_discovery(void);

#endif /* DISCOVERY_SERVER_H_ */
