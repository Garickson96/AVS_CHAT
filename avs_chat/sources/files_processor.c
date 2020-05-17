#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "../headers/chat_tcp.h"
#include "../headers/files_processor.h"
#include "../headers/universal.h"
#include "../headers/structs_threads.h"

#define VELKOST_SUBOROVEHO_BUFFRA 1024
#define NAZOV_SUBORU_VELKOST 255

// velkost SMSky
#define TEXT_VELKOST 160
#define MINI_BUFFER 4

#define MAX_LISTEN_SUBOR 1

/**
 * Vrati absolutnu cestu pre obrazok. Predpoklada sa, ze obrazok je ulozeny v ceste programu - preto je tam prikaz getcwd, ktorý vráti aktuálnu pozíciu (vracia bez /).
 * Ak bola zadana absolutna cesta (zacina sa na /), tak sa vykona jednoducha kopia. Dlzka znamena velkost cieloveho buffra, kvoli preteceniu.
 */
void vrat_absolutnu_cestu(const char *nazov_suboru, char *plna_cesta_subor, int dlzka) {
	if (*nazov_suboru == ADRESAR_SEPARATOR) {
		// predpoklada sa, ze to nie je relativna cesta
		memcpy(plna_cesta_subor, nazov_suboru, dlzka);
	} else {
		getcwd(plna_cesta_subor, dlzka);

		char *aktualna_pozicia = plna_cesta_subor + strlen(plna_cesta_subor);
		*aktualna_pozicia = ADRESAR_SEPARATOR;
		*(aktualna_pozicia + 1) = '\0';
		strcat(plna_cesta_subor, nazov_suboru);
	}
}

/**
 * Vrati relativnu cestu, ak bola zadana absolutna cesta. Odkopiruje sa text od posledneho znaku /. Pri pouziti spatnej lomky to nefunguje.
 */
void vrat_relativnu_cestu(const char *nazov_suboru, char *vystup, int dlzka) {
	char *pozicia = strrchr(nazov_suboru, ADRESAR_SEPARATOR);
	if (pozicia == NULL) {
		memcpy(vystup, nazov_suboru, dlzka);
	} else {
		memcpy(vystup, pozicia + 1, dlzka);
	}
}

/**
 * Pre posielanie suborov sa vytvori novy TCP socket na porte 9001 (fixna hodnota). Informuje sa druha strana, ze ideme posielat subor - preto,
 * aby som vykonal connect na novy port. V jednom momente moze prebiehat jeden prenos suborov. Je potrebne pockat na uvolnenie portu po odoslani
 * suboru chvilu. Spusta sa ako samostatne vlakno.
 */
void *priprav_socket_odosielanie_subor(void *data) {
	struct data_odoslanie_suboru *data_vnutorne = (struct data_odoslanie_suboru *)data;

	// vytvorenie socketu
	debug_sprava("Vytvaram spojenie pre odoslanie suboru..");
	int socket_id = nastav_chat_socket();
	nastav_chat_bind(socket_id, data_vnutorne->cislo_portu_odosielanie);
	nastav_chat_listen(socket_id, MAX_LISTEN_SUBOR);

	struct sockaddr od_koho;
	socklen_t velkost_od_koho = sizeof(od_koho);
	memset(&od_koho, 0, sizeof(od_koho));

	// odoslanie informacie o tom, ze idem na druhu stranu odosielat nejaky subor
	char text_odosli[VELKOST_SUBOROVEHO_BUFFRA];
	char plna_cesta_subor[NAZOV_SUBORU_VELKOST];
	char len_nazov_suboru[NAZOV_SUBORU_VELKOST];

	vrat_absolutnu_cestu(data_vnutorne->nazov_suboru, plna_cesta_subor, NAZOV_SUBORU_VELKOST);
	vrat_relativnu_cestu(data_vnutorne->nazov_suboru, len_nazov_suboru, NAZOV_SUBORU_VELKOST);

	// informovanie cez spravu, ze idem posielat subor
	sprintf(text_odosli, "%s#%s#%lu", len_nazov_suboru, data_vnutorne->moje_meno, zisti_velkost(plna_cesta_subor));
	chat_akcia_zapis(data_vnutorne->socket_id, "SUBOR", text_odosli);

	// otvorenie seba sa ako servera
	int accept_socket = accept(socket_id, &od_koho, &velkost_od_koho);
	osetri_chybu("Nepodaril sa accept pre odoslanie suboru - kriticka chyba.", accept_socket, -1, true, socket_id);

	// kontrola ack, nack
	char buffer_odpoved[MINI_BUFFER];
	int pocet_kontrola = recv(accept_socket, buffer_odpoved, MINI_BUFFER, 0);
	osetri_chybu("Nepodaril sa recv pri preberani suborov - potvrdenie.", pocet_kontrola, -1, true, accept_socket);

	if (strcmp(buffer_odpoved, "OK") == 0) {
		debug_sprava("Ziadost o odoslanie suboru bola prijata.");

		// odosielanie spravy
		nahraj_subor(accept_socket, data_vnutorne->nazov_suboru);
	} else {
		debug_sprava("Ziadost o odoslanie suboru bola zamietnuta.");
	}

	// ukoncenie
	close(accept_socket);
	close(socket_id);

	return NULL;
}

/**
 * Vytvori spojenie na pocitac, ktory ma odosielat subor. Pre povolenie alebo zamietnutie suboru sa posiela prva sprava OK.
 */
void priprav_socket_prijimanie_subor(struct sockaddr adresa, socklen_t dlzka_adresy, const char *nazov_suboru, bool *indikator_subory) {
	// vytvorenie socketu
	debug_sprava("Pripajam sa ako klient pre prevzatie suboru...");
	int socket_klient = nastav_chat_socket();
	int connect_status = connect(socket_klient, &adresa, dlzka_adresy);

	debug_sprava_rozsirena(nazov_suboru);
	osetri_chybu("Nepodaril sa connect pre prijatie suboru - kriticka chyba.", connect_status, -1, true, socket_klient);

	// odosielanie suboru
	if (*indikator_subory == true) {
		vypis_informaciu("Prijimanie suborov je v programe povolene...");
		send(socket_klient, "OK", 3, 0);

		preber_subor(socket_klient, nazov_suboru);
	} else {
		vypis_informaciu("Prijimanie suborov nie je v programe povolene. Poziadavka je zamietnuta...");
	}

	close(socket_klient);
}

/**
 * Vykonanie citania suboru a posielania suboru. Telo pre priprav_socket_odosielanie_subor. Subor sa cita ako binarny. Chyba pri otvoreni
 * suboru nie je kriticka pre beh programu. Subor sa cita postupne po 1024 B a posiela sa postupne. Nealokuje sa jeden velky buffer, ktory
 * by pri velmi velkych suboroch mohol sposobit problemy.
 */
void nahraj_subor(int socket_id, const char *nazov_suboru) {
	debug_sprava("Idem nahravat subor...");

	char plna_cesta_subor[NAZOV_SUBORU_VELKOST];
	vrat_absolutnu_cestu(nazov_suboru, plna_cesta_subor, NAZOV_SUBORU_VELKOST);

	// sprintf(buffer_plna_cesta, "%s%s", "/root/git/AVS_CHAT/avs_chat/Debug/", nazov_suboru);
	// memcpy(buffer_plna_cesta, nazov_suboru, sizeof(buffer_plna_cesta));
	FILE *subor_citanie = fopen(plna_cesta_subor, "rb");
	if (osetri_chybu_suboru("Nepodarilo sa otvorit subor na citanie.", subor_citanie)) {
		return;
	}

	char buffer_subor[VELKOST_SUBOROVEHO_BUFFRA];
	int precitane = 0;

	do {
		// citanie zo suboru
		precitane = fread(buffer_subor, 1, VELKOST_SUBOROVEHO_BUFFRA, subor_citanie);

		// posielanie na druhu stranu
		if (precitane > 0) {
			int pocet_znakov_zapis = send(socket_id, buffer_subor, precitane, 0);
			if (pocet_znakov_zapis == -1) {
				vypis_chybu("Nepodarilo sa odosielanie suboru - nekriticka chyba.");
				break;
			}
		}
	} while (precitane > 0);

	fclose(subor_citanie);
}

/**
 * Telo pre priprav_socket_prijimanie_subor. Cita z TCP streamu po 1024 B a nasledne ich uklada postupne do suboru. V pripade neotvorenia\
 * suboru sa v tejto funkcii nepokracuje - ale program pokracuje.
 */
void preber_subor(int socket_id, const char *nazov_suboru) {
	debug_sprava("Idem ukladat subor...");
	FILE *subor_uloz = fopen(nazov_suboru, "wb");
	if (osetri_chybu_suboru("Nepodarilo sa otvorit subor na zapis.", subor_uloz)) {
		return;
	}

	char buffer_subor[VELKOST_SUBOROVEHO_BUFFRA];
	int pocet_znakov_zapis = 0;

	do {
		pocet_znakov_zapis = recv(socket_id, buffer_subor, VELKOST_SUBOROVEHO_BUFFRA, 0);
		if (pocet_znakov_zapis == -1) {
			vypis_chybu("Nepodarilo sa prijatie suboru - nekriticka chyba.");
			break;
		}

		if (pocet_znakov_zapis > 0) {
			int status = fwrite(buffer_subor, 1, pocet_znakov_zapis, subor_uloz);
			if (status == -1) {
				vypis_chybu("Nepodarilo sa prijatie suboru - nekriticka chyba.");
				break;
			}
		}
	} while (pocet_znakov_zapis > 0);

	fclose(subor_uloz);
}

/**
 * Skontroluje existenciu adresara zadaneho ako absolutna cesta. V pripade existencie vracia true.
 */
bool skontroluj_adresar(const char *cesta) {
	DIR* adresar = opendir(cesta);

	if (adresar != NULL) {
		closedir(adresar);
		return true;
	} else {
		return false;
	}
}

/**
 * Skontroluje existenciu suboru zadaneho ako absolutna alebo relativna cesta (od programu). V pripade existencie vracia true.
 */
bool skontroluj_subor(const char *cesta) {
	return access(cesta, F_OK) != -1;
}

/**
 * Zobrazi informacie o suboroch v zadanych v absolutnej ceste. Vypisuje do konzoly, pricom sa zistuje velkost suborov a jednotlive
 * polozky sa farebne odlisuju. Ak cesta neexistuje, tak chybove hlasenie.
 */
void zobraz_subory_adresar(const char *cesta) {
    struct dirent *adresare;

	if (skontroluj_adresar(cesta) && *cesta == ADRESAR_SEPARATOR) {
	    DIR *adresar = opendir(cesta);
	    while (adresare) {
	        adresare = readdir(adresar);
	        if (adresare != NULL && strcmp(adresare->d_name, ".") != 0 && strcmp(adresare->d_name, "..") != 0) {
	        	switch (adresare->d_type) {
	        	case DT_REG:
	        	{
	        		char nazov_buffer[NAZOV_SUBORU_VELKOST];
	        		strcpy(nazov_buffer, cesta);
	        		*(nazov_buffer + strlen(cesta)) = '/';
	        		*(nazov_buffer + strlen(cesta) + 1) = '\0';
	        		strcat(nazov_buffer, adresare->d_name);

	        		printf("%s%-10s%-12ld%-30s%s\n", FARBA_SUBOR, "subor", zisti_velkost(nazov_buffer), adresare->d_name, KONIEC_FORMATOVANIA);
	        		break;
	        	}
	        	case DT_DIR:
	        	{
	        		 printf("%s%-22s%-30s%s\n", FARBA_ADRESAR, "adresar", adresare->d_name, KONIEC_FORMATOVANIA);
	        		 break;
	        	}
	        	case DT_LNK:
	        	{
	        		printf("%s%-22s%-30s%s\n", FARBA_LINK, "link", adresare->d_name, KONIEC_FORMATOVANIA);
	        		break;
	        	}
	        	default:
	        	{
	        		printf("%-22s%-30s\n", "", adresare->d_name);
	        		break;
	        	}
	        	}
	        }
	    }

	    closedir(adresar);
	} else {
		vypis_chybu("Pozadovany adresar neexistuje.");
	}
}

/**
 * Zisti velkost suboru zadaneho cez parameter v bajtoch. V pripade chyby vracia hodnotu 0.
 */
unsigned long zisti_velkost(const char *cesta) {
	struct stat statistiky;
	int status = stat(cesta, &statistiky);
	unsigned long velkost_suboru = statistiky.st_size;

	if (status == -1) {
		return 0;
	} else {
		return velkost_suboru;
	}
}
