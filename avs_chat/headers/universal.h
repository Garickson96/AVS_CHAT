#ifndef HEADERS_UNIVERSAL_H_
#define HEADERS_UNIVERSAL_H_

#include <stdbool.h>

void osetri_chybu(char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota, bool zavri_spojenie, int socket_descr);

void debug_sprava(char *sprava);
void debug_ip_sprava(struct sockaddr_in *ip, char *sprava);

#endif /* HEADERS_UNIVERSAL_H_ */
