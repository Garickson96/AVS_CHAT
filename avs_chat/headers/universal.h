#ifndef HEADERS_UNIVERSAL_H_
#define HEADERS_UNIVERSAL_H_

#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ODDELOVAC '>'
#define ODDELOVAC_AKCIA '~'
#define ODDELOVAC_PARAMETER '#'

// https://gist.github.com/RabaDabaDoba/145049536f815903c79944599c6f952a

#define CHYBA_TUCNA_CERVENA "\e[1;31m"
#define CHYBA_NEKRITICKA_FIALOVA "\e[0;35m"
#define OZNAM_TUCNY_ZELENY "\e[1;32m"
#define OZNAM_TUCNY_MODRY "\e[1;94m"
#define DEBUG_ORANZOVA "\e[38;5;220m"

#define TUCNE "\e[1m"
#define KONIEC_FORMATOVANIA "\e[0m"

void osetri_chybu(const char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota, bool zavri_spojenie, int socket_descr);
void osetri_chybu_nekriticka(const char *popis_chyby, int hodnota_porovnaj, int chybova_hodnota);
void osetri_chybu_malloc(const char *popis_chyby, void *smernik_malloc);

bool osetri_chybu_suboru(const char *popis_chyby, void *smernik_subor);

void *vytvor_nastav_malloc(int velkost, const char *sprava);
void dealokuj_malloc(void **malloc_priestor);

void vypis_nadpis(const char *text);
void vypis_uspech(const char *text);
void vypis_chybu(const char *text);
void vypis_informaciu(const char *text);
void vypis_popisok(const char *text);

void debug_sprava(const char *sprava);
void debug_ip_sprava(struct sockaddr_in *ip, const char *sprava);
void debug_sprava_rozsirena(const char *sprava);

#endif /* HEADERS_UNIVERSAL_H_ */
