#ifndef HEADERS_CLI_H_
#define HEADERS_CLI_H_

#include "../headers/doubly_linked_list.h"

char vyber_volby(void);

void zobraz_list_connect(DOUBLYLINKEDLIST *list, char *moje_meno, char **pomenovania_statusov, int n);
void zobraz_list_accept(DOUBLYLINKEDLIST *list);

void posli_spravu(DOUBLYLINKEDLIST *list, char *moje_meno, char **pomenovania_statusov, int n);

void vypis_dostupne_statusy(char **pomenovania_statusov, int n);
void zobraz_moj_status(char **pomenovania_statusov, int n, int *aktualny_stav);
void zmen_moj_status(char **pomenovania_statusov, int n, int *aktualny_stav, DOUBLYLINKEDLIST *list, char *moje_meno);
void aktualizuj_statusy(DOUBLYLINKEDLIST *list, char *moje_meno);

void zoznam_menu(void);
void nespravna_hodnota(void);

void ukoncenie_programu(bool *indikator_pokracuj, int chat_socket, int discovery_socket, pthread_t **pole_threadov, int pocet);

#endif /* HEADERS_CLI_H_ */
