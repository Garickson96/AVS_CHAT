#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <sys/socket.h>
#include <time.h>

typedef struct accept_info {
	char meno[50];
	struct sockaddr ip_adresa;
	int socket_id;

	int posledny_stav;
	time_t cas_nastavenia;
} ACCEPT_INFO;

typedef struct doublylinkedlist_item {
    ACCEPT_INFO data;
    struct doublylinkedlist_item *next;
    struct doublylinkedlist_item *previous;
} DOUBLYLINKEDLIST_ITEM;

typedef struct doublylinkedlist {
    DOUBLYLINKEDLIST_ITEM *first;
    DOUBLYLINKEDLIST_ITEM *last;
    int size;
} DOUBLYLINKEDLIST;

void initDLL(DOUBLYLINKEDLIST *list);
void initDLLItem(DOUBLYLINKEDLIST_ITEM *item, ACCEPT_INFO data);

void disposeDLL(DOUBLYLINKEDLIST *list); 

void addDLL(DOUBLYLINKEDLIST *list, ACCEPT_INFO data);

bool tryInsertDLL(DOUBLYLINKEDLIST *list, ACCEPT_INFO data, int pos);
bool trySetDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO data);
bool tryGetDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO *data);

bool tryRemoveDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO *data);

#endif /* LINKEDLIST_H */
