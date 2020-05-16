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
#define TEXT_VELKOST 160
#define MINI_BUFFER 4

#define MAX_LISTEN_SUBOR 1

/**
 *
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
 *
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
 *
 */
void *priprav_socket_odosielanie_subor(void *data) {
	struct data_odoslanie_suboru *data_vnutorne = (struct data_odoslanie_suboru *)data;

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
 *
 */
void priprav_socket_prijimanie_subor(struct sockaddr adresa, socklen_t dlzka_adresy, const char *nazov_suboru, bool *indikator_subory) {
	debug_sprava("Pripajam sa ako klient pre prevzatie suboru...");
	int socket_klient = nastav_chat_socket();
	int connect_status = connect(socket_klient, &adresa, dlzka_adresy);

	debug_sprava_rozsirena(nazov_suboru);
	osetri_chybu("Nepodaril sa connect pre prijatie suboru - kriticka chyba.", connect_status, -1, true, socket_klient);

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
 *
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
		precitane = fread(buffer_subor, 1, VELKOST_SUBOROVEHO_BUFFRA, subor_citanie);

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
 *
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
 *
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
 *
 */
bool skontroluj_subor(const char *cesta) {
	return access(cesta, F_OK) != -1;
}

/**
 *
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
 *
 */
unsigned long zisti_velkost(const char *cesta) {
	struct stat statistiky;
	int status = stat(cesta, &statistiky);
	unsigned long velkost_suboru = statistiky.st_size;

	if (status == -1) {
		return -1;
	} else {
		return velkost_suboru;
	}
}
