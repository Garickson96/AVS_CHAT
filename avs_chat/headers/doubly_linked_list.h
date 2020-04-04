#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct doublylinkedlist_item {
    double data;
    struct doublylinkedlist_item *next;
    struct doublylinkedlist_item *previous;
} DOUBLYLINKEDLIST_ITEM;

typedef struct doublylinkedlist {
    DOUBLYLINKEDLIST_ITEM *first;
    DOUBLYLINKEDLIST_ITEM *last;
    int size;
} DOUBLYLINKEDLIST;

void initDLL(DOUBLYLINKEDLIST *list);
void initDLLItem(DOUBLYLINKEDLIST_ITEM *item, double data);

void disposeDLL(DOUBLYLINKEDLIST *list); 
void printDLL(const DOUBLYLINKEDLIST *list); 
void printDLLReverse(const DOUBLYLINKEDLIST *list);

void addDLL(DOUBLYLINKEDLIST *list, double data); 

bool tryInsertDLL(DOUBLYLINKEDLIST *list, double data, int pos);
bool trySetDLL(DOUBLYLINKEDLIST *list, int pos, double data); 
bool tryGetDLL(DOUBLYLINKEDLIST *list, int pos, double *data); 

bool tryRemoveDLL(DOUBLYLINKEDLIST *list, int pos, double *data);
bool tryCopyDLL(const DOUBLYLINKEDLIST *src, DOUBLYLINKEDLIST *dest); 

void readFromTxtDLL(DOUBLYLINKEDLIST *list, FILE *txtFile); 
void writeToTxtDLL(const DOUBLYLINKEDLIST *list, FILE *txtFile);

#endif /* LINKEDLIST_H */
