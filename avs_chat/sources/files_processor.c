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

#define MAX_LISTEN_SUBOR 1

/**
 *
 */
// int cislo_portu_odosielanie, char *nazov_suboru
void *priprav_socket_odosielanie_subor(void *data) {
	struct data_odoslanie_suboru *data_vnutorne = (struct data_odoslanie_suboru *)data;
	int cislo_portu_odosielanie = data_vnutorne->cislo_portu_odosielanie;
	int socket_tcp_mimo = data_vnutorne->socket_id;
	char *nazov_suboru = data_vnutorne->nazov_suboru;
	char *moje_meno = data_vnutorne->moje_meno;

	debug_sprava("Vytvaram spojenie pre odoslanie suboru..");
	int socket_id = nastav_chat_socket();
	nastav_chat_bind(socket_id, cislo_portu_odosielanie);
	nastav_chat_listen(socket_id, MAX_LISTEN_SUBOR);

	// todo> prerobit na samostatnu funkciu
	struct sockaddr od_koho;
	socklen_t velkost_od_koho = sizeof(od_koho);
	memset(&od_koho, 0, sizeof(od_koho));

	char text_odosli[TEXT_VELKOST];
	sprintf(text_odosli, "%s#%s#%ld", nazov_suboru, moje_meno, (long)0);
	chat_akcia_zapis(socket_tcp_mimo, "SUBOR", text_odosli);

	int accept_socket = accept(socket_id, &od_koho, &velkost_od_koho);
	osetri_chybu("Nepodaril sa accept pre odoslanie suboru - kriticka chyba.", accept_socket, -1, true, socket_id);

	// kontrola ack, nack
	//ok/nok nad accept


	// odosielanie spravy
	nahraj_subor(accept_socket, nazov_suboru);

	// ukoncenie - todo>skontroluj
	close(accept_socket);
	close(socket_id);

	return NULL;
}

/**
 *
 */
void priprav_socket_prijimanie_subor(struct sockaddr adresa, socklen_t dlzka_adresy, char *nazov_suboru) {
	debug_sprava("Pripajam sa ako klient pre prevzatie suboru...");
	int socket_klient = nastav_chat_socket();
	int connect_status = connect(socket_klient, &adresa, dlzka_adresy);
	printf("%s\n", nazov_suboru);
	osetri_chybu("Nepodaril sa connect pre prijatie suboru - kriticka chyba.", connect_status, -1, true, socket_klient);

	// prijatie suboru
	preber_subor(socket_klient, nazov_suboru);

	// ukoncenie
	close(socket_klient);
}

/**
 *
 */
void nahraj_subor(int socket_id, char *nazov_suboru) {
	debug_sprava("Idem nahravat subor...");


    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);


	char buffer_plna_cesta[200];
	sprintf(buffer_plna_cesta, "./%s", nazov_suboru);
	// sprintf(buffer_plna_cesta, "%s%s", "/root/git/AVS_CHAT/avs_chat/Debug/", nazov_suboru);
	//memcpy(buffer_plna_cesta, nazov_suboru, sizeof(buffer_plna_cesta));

	FILE *subor_citanie = fopen(buffer_plna_cesta, "rb");
	if (osetri_chybu_suboru("Nepodarilo sa otvorit subor na citanie.", subor_citanie)) {
		return;
	}

	char buffer_subor[VELKOST_SUBOROVEHO_BUFFRA];
	//char mini_buff[2];
	int precitane = 0;
	do {
		precitane = fread(buffer_subor, 1, VELKOST_SUBOROVEHO_BUFFRA, subor_citanie);

		//int pocet_kontrola = recv(socket_id, mini_buff, 3, 0);
		if (precitane > 0) {
			int pocet_znakov_zapis = send(socket_id, buffer_subor, precitane, 0);
			if (pocet_znakov_zapis == -1) {
				printf("[ERROR]: Nepodarilo sa odosielanie suboru - nekriticka chyba.\n");
				break;
			}
		}
	} while (precitane != 0);

	fclose(subor_citanie);
}

/**
 *
 */
void preber_subor(int socket_id, char *nazov_suboru) {
	debug_sprava("Idem ukladat subor...");
	FILE *subor_uloz = fopen(nazov_suboru, "wb");
	if (osetri_chybu_suboru("Nepodarilo sa otvorit subor na zapis.", subor_uloz)) {
		return;
	}

	char buffer_subor[VELKOST_SUBOROVEHO_BUFFRA];
	int pocet_znakov_zapis = 0;
	//int status = -1;
	/*while (status < 0) {
		status = send(socket_id, "OK", 3, 0);
	}*/

	do {
		pocet_znakov_zapis = recv(socket_id, buffer_subor, VELKOST_SUBOROVEHO_BUFFRA, 0);
		if (pocet_znakov_zapis == -1) {
			printf("[ERROR]: Nepodarilo sa prijatie suboru - nekriticka chyba.\n");
			break;
		}

		if (pocet_znakov_zapis > 0) {
			int status = fwrite(buffer_subor, 1, pocet_znakov_zapis, subor_uloz);
			if (status == -1) {
				printf("[ERROR]: Nepodarilo sa prijatie suboru - nekriticka chyba.\n");
				break;
			}
		}
	} while (pocet_znakov_zapis > 0);

	fclose(subor_uloz);
}

/**
 *
 */
void zobraz_subory_adresar(char *cesta) {
    struct dirent *adresare;

    // todo> kontrola ci cesta existuje
    DIR *adresar = opendir(cesta);
    while (adresar) {
        adresare = readdir(adresar);
        if (adresare != NULL) {

        	// todo> farebne zvyraznenia
        	switch (adresare->d_type) {
        	case DT_REG:
        	{
        		char nazov_buffer[NAZOV_SUBORU_VELKOST];
        		strcpy(nazov_buffer, cesta);
        		*(nazov_buffer + strlen(cesta)) = '/';
        		*(nazov_buffer + strlen(cesta) + 1) = '\0';
        		strcat(nazov_buffer, adresare->d_name);

        		printf("\tsubor %ld %s\n", zisti_velkost(nazov_buffer), adresare->d_name);
        		break;
        	}
        	case DT_DIR:
        	{
        		 printf("adresar %s\n", adresare->d_name);
        		 break;
        	}
        	case DT_LNK:
        	{
        		printf("link %s\n", adresare->d_name);
        		break;
        	}
        	default:
        	{
        		printf("neznamy %s\n", adresare->d_name);
        		break;
        	}
        	}
        }
    }

    closedir(adresar);
}

unsigned long zisti_velkost(char *cesta) {
	struct stat statistiky;
	int status = stat(cesta, &statistiky);
	unsigned long velkost_suboru = statistiky.st_size;

	if (status == -1) {
		return -1;
	} else {
		return velkost_suboru;
	}
}
