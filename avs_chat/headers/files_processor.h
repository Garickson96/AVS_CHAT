#ifndef HEADERS_FILES_PROCESSOR_H_
#define HEADERS_FILES_PROCESSOR_H_

#include <stdbool.h>

#define ADRESAR_SEPARATOR '/'

#define FARBA_ADRESAR "\e[1;94m"
#define FARBA_SUBOR "\e[1;36m"
#define FARBA_LINK "\e[1;95m"

void *priprav_socket_odosielanie_subor(void *data);
void priprav_socket_prijimanie_subor(struct sockaddr adresa, socklen_t dlzka_adresy, const char *nazov_suboru, bool *indikator_subory);

void nahraj_subor(int socket_id, const char *nazov_suboru);
void preber_subor(int socket_id, const char *nazov_suboru);

bool skontroluj_adresar(const char *cesta);
bool skontroluj_subor(const char *cesta);

void zobraz_subory_adresar(const char *cesta);
unsigned long zisti_velkost(const char *cesta);

void vrat_absolutnu_cestu(const char *nazov_suboru, char *plna_cesta_subor, int dlzka);
void vrat_relativnu_cestu(const char *nazov_suboru, char *vystup, int dlzka);

#endif /* HEADERS_FILES_PROCESSOR_H_ */
