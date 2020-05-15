#ifndef HEADERS_FILES_PROCESSOR_H_
#define HEADERS_FILES_PROCESSOR_H_

void *priprav_socket_odosielanie_subor(void *data);
void priprav_socket_prijimanie_subor(struct sockaddr adresa, socklen_t dlzka_adresy, char *nazov_suboru);

void nahraj_subor(int socket_id, char *nazov_suboru);
void preber_subor(int socket_id, char *nazov_suboru);

void zobraz_subory_adresar(char *cesta);
unsigned long zisti_velkost(char *cesta);

#endif /* HEADERS_FILES_PROCESSOR_H_ */
