#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../headers/doubly_linked_list.h"

void initDLL(DOUBLYLINKEDLIST *list) {
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
}

void initDLLItem(DOUBLYLINKEDLIST_ITEM *item, ACCEPT_INFO data) {
    memcpy(&(item->data), &data, sizeof(data));
    item->next = NULL;
    item->previous = NULL;
}

void disposeDLL(DOUBLYLINKEDLIST *list) {
    DOUBLYLINKEDLIST_ITEM *item = list->first;
    
    // postupné prechádzanie prvkov s medzivymazávaním
    while (item != NULL) {
        list->first = list->first->next;
        
        free(item);
        item = list->first;
    }
    
    list->last = NULL;
    list->size = 0;
} 

static DOUBLYLINKEDLIST_ITEM *get_item(DOUBLYLINKEDLIST *list, int pos) {
    DOUBLYLINKEDLIST_ITEM *aktualna_polozka = list->first;   
    int i = 1;
    
    while (i <= pos) {
        aktualna_polozka = aktualna_polozka->next;
        i++;
    }
    
    return aktualna_polozka;
}

static DOUBLYLINKEDLIST_ITEM *get_item_reverse(DOUBLYLINKEDLIST *list, int pos) {
    DOUBLYLINKEDLIST_ITEM *aktualna_polozka = list->last;   
    int i = list->size - 1;
            
    while (i > pos) {
        aktualna_polozka = aktualna_polozka->previous;
        i--;
    }
    
    return aktualna_polozka;
}

void addDLL(DOUBLYLINKEDLIST *list, ACCEPT_INFO data) {
    // musí to byť na halde
    DOUBLYLINKEDLIST_ITEM *new_item = (DOUBLYLINKEDLIST_ITEM *)malloc(sizeof(DOUBLYLINKEDLIST_ITEM));
    // zišla by sa tu kontrola či to existuje
    initDLLItem(new_item, data);
    
    if (list->size == 0) {
        // pridávam prvok do prázdneho linkedlistu
        list->first = new_item;
        list->last = new_item;
    } else {
        new_item->previous = list->last;
        list->last->next = new_item;
        list->last = new_item;      
    }
    
    list->size++;
} 

bool tryInsertDLL(DOUBLYLINKEDLIST *list, ACCEPT_INFO data, int pos) {
    if (pos >= 0 && pos <= list->size) {
        DOUBLYLINKEDLIST_ITEM *new_item = (DOUBLYLINKEDLIST_ITEM *)malloc(sizeof(DOUBLYLINKEDLIST_ITEM));
        if (new_item == NULL) {
            return false;
        }
        initDLLItem(new_item, data);
        
        if (pos == list->size) {
            addDLL(list, data);
            return true;
        } else if (pos == 0) {
            DOUBLYLINKEDLIST_ITEM *teraz_druhy = list->first;
            list->first = new_item;
            new_item->next = teraz_druhy;
            // vkladám na začiatok - takže previous null
        } else {
            DOUBLYLINKEDLIST_ITEM *pred_poziciou;
            DOUBLYLINKEDLIST_ITEM *za_poziciou;
            
            if (pos < list->size / 2) {
                pred_poziciou = get_item(list, pos - 1);
                za_poziciou = pred_poziciou->next;
            } else {              
                za_poziciou = get_item_reverse(list, pos - 1);
                pred_poziciou = za_poziciou->previous;
            }
            
            pred_poziciou->next = new_item;
            new_item->next = za_poziciou;
            new_item->previous = pred_poziciou;
        }
        
        // nezabudnúť zvýšiť počet prvkov
        list->size++;
        return true;
    } else {
        return false;
    }
}

bool trySetDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO data) {
    if (pos >= 0 && pos < list->size) {
        DOUBLYLINKEDLIST_ITEM *aktualna_polozka;
        if (pos < list->size / 2) {
            aktualna_polozka = get_item(list, pos);
        } else {
            aktualna_polozka = get_item_reverse(list, pos);
        }
        
        aktualna_polozka->data = data;
        return true;
    } else {
        return false;
    }
}

bool tryGetDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO *data) {
    if (pos >= 0 && pos < list->size) {
        DOUBLYLINKEDLIST_ITEM *aktualna_polozka;
        if (pos < list->size / 2) {
            aktualna_polozka = get_item(list, pos);
        } else {
            aktualna_polozka = get_item_reverse(list, pos);
        }
        
        *data = aktualna_polozka->data;
        return true;
    } else {
        return false;
    }
} 

bool tryRemoveDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO *data) {
    if (pos >= 0 && pos < list->size) {   
        // veľmi špecifický prípad
        if (list->size == 1) {
            // tu budem mať prázdny zoznam
            *data = list->first->data;
            free(list->first);
            
            list->first = NULL;
            list->last = NULL;
            list->size--;
            
            return true;
        }
        
        // špecifický prípad - vymazávanie zo začiatku
        if (pos == 0) {
            DOUBLYLINKEDLIST_ITEM *backup = list->first;
            *data = list->first->data;
            list->first = list->first->next;
            // rozdiel medzi LL a DLL
            list->first->previous = NULL;
            
            free(backup);
            // toto nastavenie nemá zmysel - lokálna premenná aktualna_polozka = NULL;
            
            // nezabudnúť znížiť počet prvkov
            list->size--;
            return true;
        } 
                
        DOUBLYLINKEDLIST_ITEM *aktualna_polozka;
        if (pos < list->size / 2) {
           aktualna_polozka = get_item(list, pos); 
        } else {
           aktualna_polozka = get_item_reverse(list, pos); 
        }
        
        DOUBLYLINKEDLIST_ITEM *predosla_polozka = aktualna_polozka->previous;
        
        if (pos == list->size - 1){
            *data = list->last->data;
            list->last = predosla_polozka;
            predosla_polozka->next = NULL;
            // previous tu nikde nenastavujem            
            
            free(aktualna_polozka);
        } else {
            *data = aktualna_polozka->data;
            // mažem niekde v strede
            predosla_polozka->next = aktualna_polozka->next;
            aktualna_polozka->previous = predosla_polozka;
            
            free(aktualna_polozka);
        }
        
        // nezabudnúť znížiť počet prvkov
        list->size--;
        return true;
    } else {
        return false;
    }
}
