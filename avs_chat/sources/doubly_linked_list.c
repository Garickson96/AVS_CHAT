#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../headers/doubly_linked_list.h"

/**
 * Inicializuje prazdny obojstranne zretazeny zoznam. Nastavi smernik na prvy prvok a na posledny prvok na NULL. Pocet prvkov v prazdnom zozname je 0.
 */
void initDLL(DOUBLYLINKEDLIST *list) {
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
}

/**
 * Inicializuje novu polozku v zozname. Skopiruju sa data zo struktury a smerniky na prvky sa nastavuju neskor, preto NULL.
 */
void initDLLItem(DOUBLYLINKEDLIST_ITEM *item, ACCEPT_INFO data) {
    memcpy(&(item->data), &data, sizeof(data));
    item->next = NULL;
    item->previous = NULL;
}

/**
 * Vycisti zoznam. Postupne sa prechadzaju prvky a zavola sa nad nimi free. Nasledne sa nastavia predvolene hodnoty pre nastavenia listu.
 */
void disposeDLL(DOUBLYLINKEDLIST *list) {
    DOUBLYLINKEDLIST_ITEM *item = list->first;
    
    // postupne prechadzanie prvkov
    while (item != NULL) {
        list->first = list->first->next;
        
        free(item);
        item = list->first;
    }
    
    list->last = NULL;
    list->size = 0;
} 

/**
 * Postupne sa prezeraju v smere od zaciatku prvky a vrati sa prvok na zadanom indexe.
 */
DOUBLYLINKEDLIST_ITEM *get_item(DOUBLYLINKEDLIST *list, int pos) {
    DOUBLYLINKEDLIST_ITEM *aktualna_polozka = list->first;   
    int i = 1;
    
    while (i <= pos) {
        aktualna_polozka = aktualna_polozka->next;
        i++;
    }
    
    return aktualna_polozka;
}

/**
 * Postupne sa prezeraju v smere od konca prvky a vrati sa prvok na zadanom indexe. Vracia prvok, nie data.
 */
DOUBLYLINKEDLIST_ITEM *get_item_reverse(DOUBLYLINKEDLIST *list, int pos) {
    DOUBLYLINKEDLIST_ITEM *aktualna_polozka = list->last;   
    int i = list->size - 1;
            
    while (i > pos) {
        aktualna_polozka = aktualna_polozka->previous;
        i--;
    }
    
    return aktualna_polozka;
}

/**
 * Prida sa na zaciatok DLL novy prvok. Data sa kopiruju v initDLLItem.
 */
void addDLL(DOUBLYLINKEDLIST *list, ACCEPT_INFO data) {
    // musi to byt na halde
    DOUBLYLINKEDLIST_ITEM *new_item = (DOUBLYLINKEDLIST_ITEM *)malloc(sizeof(DOUBLYLINKEDLIST_ITEM));
    initDLLItem(new_item, data);
    
    if (list->size == 0) {
        // pridavam novy prvok do prazdneho linked listu
        list->first = new_item;
        list->last = new_item;
    } else {
        new_item->previous = list->last;
        list->last->next = new_item;
        list->last = new_item;      
    }
    
    list->size++;
} 

/**
 * Prida sa novy prvok na pozadovanu poziciu v linkedliste. V projekte to nie je aktualne pouzite. V pripade uspechu vracia true, inak false.
 */
bool tryInsertDLL(DOUBLYLINKEDLIST *list, ACCEPT_INFO data, int pos) {
    if (pos >= 0 && pos <= list->size) {
        DOUBLYLINKEDLIST_ITEM *new_item = (DOUBLYLINKEDLIST_ITEM *)malloc(sizeof(DOUBLYLINKEDLIST_ITEM));
        if (new_item == NULL) {
            return false;
        }
        initDLLItem(new_item, data);
        
        // vkladanie na koniec
        if (pos == list->size) {
            addDLL(list, data);
            return true;
        } else if (pos == 0) {
        	// vkladanie na zaciatok
            DOUBLYLINKEDLIST_ITEM *teraz_druhy = list->first;
            list->first = new_item;
            new_item->next = teraz_druhy;

        } else {
        	// vkladanie do stredu - musim sa dostat na poziciu pred a vykonat zmeny
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
        
        // zvysenie poctu prvkov
        list->size++;
        return true;
    } else {
        return false;
    }
}

/**
 * Nastavi hodnotu pre data pre prvok na indexe pos. V projekte to aktualne nie je pouzivane. V pripade uspechu vracia true, inak false.
 */
bool trySetDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO data) {
    if (pos >= 0 && pos < list->size) {
        DOUBLYLINKEDLIST_ITEM *aktualna_polozka;
        if (pos < list->size / 2) {
            aktualna_polozka = get_item(list, pos);
        } else {
            aktualna_polozka = get_item_reverse(list, pos);
        }
        
        memcpy(&(aktualna_polozka->data), &data, sizeof(data));
        return true;
    } else {
        return false;
    }
}

/**
 * Ziska hodnotu pre data na prvok na zadanom indexe. Vyuziva obojstrannost zoznamu. V projekte sme vacsinou vyhladavali podla
 * nejakej vlastnosti, preto to nie je aktivne v projekte pouzivane. V pripade uspechu vracia true, inak false.
 */
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

/**
 * Vymaze prvok na zadanej pozicii. Vracia data cez posledny parameter. V pripade uspechu vracia true, inak false.
 */
bool tryRemoveDLL(DOUBLYLINKEDLIST *list, int pos, ACCEPT_INFO *data) {
    if (pos >= 0 && pos < list->size) {   
        // velmi specificky pripad
        if (list->size == 1) {
            // vystupom bude prazdny zoznam
            *data = list->first->data;
            free(list->first);
            
            list->first = NULL;
            list->last = NULL;
            list->size--;
            
            return true;
        }
        
        // specificky pripad - vymazavanie od zaciatku
        if (pos == 0) {
            DOUBLYLINKEDLIST_ITEM *backup = list->first;
            *data = list->first->data;
            list->first = list->first->next;
            // rozdiel medzi LL a DLL
            list->first->previous = NULL;
            
            free(backup);
            
            // znizit pocet prvkov
            list->size--;
            return true;
        } 
                
        // vymazavanie v strede zoznamu
        // dostat sa na zadanu poziciu
        DOUBLYLINKEDLIST_ITEM *aktualna_polozka;
        if (pos < list->size / 2) {
           aktualna_polozka = get_item(list, pos); 
        } else {
           aktualna_polozka = get_item_reverse(list, pos); 
        }
        
        DOUBLYLINKEDLIST_ITEM *predosla_polozka = aktualna_polozka->previous;
        
        // mazanie predposlednej polozky
        if (pos == list->size - 1){
        	memcpy(data, &(list->last->data), sizeof(ACCEPT_INFO));
            list->last = predosla_polozka;
            predosla_polozka->next = NULL;
            // previous tu nikde nenastavujem            
            
            free(aktualna_polozka);
        } else {
        	// ostatne polozky
        	memcpy(data, &(list->last->data), sizeof(ACCEPT_INFO));
            // mažem niekde v strede
            predosla_polozka->next = aktualna_polozka->next;
            aktualna_polozka->previous = predosla_polozka;
            
            free(aktualna_polozka);
        }
        
        // znizenie poctu prvkov v zozname
        list->size--;
        return true;
    } else {
        return false;
    }
}
