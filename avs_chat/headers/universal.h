#ifndef HEADERS_UNIVERSAL_H_
#define HEADERS_UNIVERSAL_H_

#include <stdbool.h>

void osetri_chybu(char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota, bool zavri_spojenie, int socket_descr);
void osetri_chybu_nekriticka(char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota);
void osetri_chybu_malloc(char *popis_chyby, void *smernik_malloc);

void *vytvor_nastav_malloc(int velkost, char *sprava);
void dealokuj_malloc(void **malloc_priestor);

void debug_sprava(char *sprava);
void debug_ip_sprava(struct sockaddr_in *ip, char *sprava);

#endif /* HEADERS_UNIVERSAL_H_ */
