#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ifaddrs.h>
#include <pthread.h>

#include "../headers/chat_tcp.h"
#include "../headers/discovery_udp.h"
#include "../headers/structs_threads.h"
#include "../headers/universal.h"
#include "../headers/files_processor.h"
#include "../headers/cli.h"
#include "../headers/hudba_chat.h"

#define MENO_BUFFER 30
#define POLOZKY_THREAD 4

#define POCET_STATUSOV 5

// globalna premenna
int debug_avs_chat = 1;

/**
 * Hlavny program. Skontroluje sa spravnost zadanych argumentov v tvare <cislo portu TCP servera> <cislo portu UDP discovery> <debug level, nepovinny parameter>.
 * Nasledne sa pripravia data platne pre cely beh programu. V dalsej casti sa poziada od pouzivatela jeho meno, ktore je jedinecnym identifikatorom na chate.
 * Pripravia sa data pre jednotlive vlakna a spustia sa v spracuj_chat a v spracuj_discovery v samostatnych vlaknach.
 * Dalej sa pouzivatelovi zobrazi textove menu, ktore sa pouziva na ovladanie programu. Na konci tejto funkcie su uvedene prikazy potrebne pre vycistenie obsadenej
 * pamate v halde.
 */
// poradie argumentov <TCP server> <DISCOVERY SERVER> <debug level 0 az 2 - dobrovolny>
// napriklad 10100 10001 1
int main(int argc, char **argv) {
	/**
	 * Pocet dodatocnych argumentov je 2 alebo 3 (ak chce nastavit explicitne debug level). Implicitny argument 0 je nazov programu.
	 */
	if (argc != 3 && argc != 4) {
		vypis_chybu("Vyzaduju sa cisla portov ako argumenty programu.");
		vypis_chybu("Takto: <TCP server> <DISCOVERY SERVER> <DEBUG LEVEL>");
		return EXIT_FAILURE;
	}

	// porty a ich cisla
	int cisla_portov[2];
	memset(cisla_portov, 0, sizeof(cisla_portov));
	for (int i = 1; i < argc; i++) {
		cisla_portov[i - 1] = atoi(argv[i]);
		if (cisla_portov[i - 1] < 1 || cisla_portov[i - 1] > 65535) {
			vypis_chybu("Skontrolujte si parametre programu - od 1 po 65535.");
			return EXIT_FAILURE;
		}
	}

	if (argc == 4) {
		int debug_test = atoi(argv[3]);
		if (debug_test >= 0 && debug_test <= 2) {
			debug_avs_chat = debug_test;
		}
	}

	/**
	 * Priprava dat potrebnych pre beh programu.
	 */
	// program nebol ukonceny
	bool indikator_pokracuj = true;
	// prijimam subory
	bool indikator_subory = true;

	pthread_t thread_accept, thread_spracovanie, thread_discovery, thread_discovery_posli;
	pthread_t *pole_thread[POLOZKY_THREAD] = { &thread_accept, &thread_spracovanie, &thread_discovery, &thread_discovery_posli };

	char *pomenovania_statusov[] = { "dostupny", "zaneprazdneny", "nerusit", "nedostupny", "oficialne neznamy" };
	int aktualny_stav = 0;

	struct ifaddrs *prva_polozka_zoznamu;
    int stav_adresy = getifaddrs(&prva_polozka_zoznamu);
    osetri_chybu("Nepodarilo sa zistit broadcastove adresy portov.", stav_adresy, -1, false, 0);

    // priprava hudby
    sfSoundBuffer *hudba_buffer_sprava = NULL;
    sfSoundBuffer *hudba_buffer_odhlasenie = NULL;
    priprav_hudbu(&hudba_buffer_sprava, &hudba_buffer_odhlasenie);

    /**
     * Pouzivatelovi sa zobrazi nazov programu a poziada sa od neho meno. Meno ma byt bez medzery.
     */
	vypis_nadpis("P2P chat - semestralna praca");

	char meno[MENO_BUFFER];
	memset(meno, 0, sizeof(meno));
	vypis_popisok("Zadajte meno (jedinecny identifikator): ");
	scanf("%s", meno);

	/**
	 * Priprava dat pre vlakna a priprava linkedlistov na ukladanie dat. Data su ukladane v halde.
	 */
	struct data_discovery *data_discovery = (struct data_discovery *)vytvor_nastav_malloc(sizeof(struct data_discovery), "Nepodarilo sa alokovat priestor pre data_discovery.");
	struct data_discovery_zistovanie *data_discovery_zistovanie = (struct data_discovery_zistovanie *)vytvor_nastav_malloc(sizeof(struct data_discovery_zistovanie), "Nepodarilo sa alokovat priestor pre data_discovery_zistovanie.");

	struct data_accept *data_accept = (struct data_accept *)vytvor_nastav_malloc(sizeof(struct data_accept), "Nepodarilo sa alokovat priestor pre data_accept.");
	struct data_read_write *data_read_write = (struct data_read_write *)vytvor_nastav_malloc(sizeof(struct data_read_write), "Nepodarilo sa alokovat priestor pre data_read_write.");

	DOUBLYLINKEDLIST list_accept, list_connect;
	initDLL(&list_accept);
	initDLL(&list_connect);

	data_accept->list_accept = &list_accept;

	struct data_odoslanie_suboru *data_odoslanie_suboru = (struct data_odoslanie_suboru *)vytvor_nastav_malloc(sizeof(struct data_odoslanie_suboru), "Nepodarilo sa alokovat priestor pre data_odoslanie_suboru.");

	// pridanie sameho seba do zoznamu list_connect
	struct accept_info info_ja;
	memset(&info_ja, 0, sizeof(info_ja));
	strncpy(info_ja.meno, meno, sizeof(info_ja.meno));
	addDLL(&list_connect, info_ja);

	/**
	 * Spustenie socketov a prace s chatom a s discovery cez UDP.
	 */
	int chat_socket = spracuj_chat(&indikator_pokracuj, &indikator_subory, &aktualny_stav, meno, &list_connect, &list_accept, POCET_STATUSOV, cisla_portov[0],
			&thread_accept, &thread_spracovanie, data_accept, data_read_write, hudba_buffer_sprava, hudba_buffer_odhlasenie);

	int discovery_socket = spracuj_discovery(&indikator_pokracuj, cisla_portov[0], cisla_portov[1],
			&thread_discovery, &thread_discovery_posli, data_discovery, data_discovery_zistovanie, chat_socket, meno, &list_connect, prva_polozka_zoznamu);

	/**
	 * Menu a ovladanie programu.
	 */
	zoznam_menu();
	while (indikator_pokracuj) {
		char volba = vyber_volby();
		switch (volba) {
			case 'a':
			{
				zobraz_list_accept(&list_accept);
				break;
			}
			case 'b':
			{
				zobraz_list_connect(&list_connect, meno, pomenovania_statusov, POCET_STATUSOV);
				break;
			}
			case 'c':
			{
				posli_spravu(&list_connect, meno, pomenovania_statusov, POCET_STATUSOV);
				break;
			}
			case 'd':
			{
				zobraz_moj_status(pomenovania_statusov, POCET_STATUSOV, &aktualny_stav);
				break;
			}
			case 'e':
			{
				zmen_moj_status(pomenovania_statusov, POCET_STATUSOV, &aktualny_stav, &list_connect, meno);
				break;
			}
			case 'f':
			{
				aktualizuj_statusy(&list_connect, meno);
				break;
			}
			case 'g':
			{
				zobraz_subory_adresar_cli();
				break;
			}
			case 'h':
			{
				odoslat_subor_cli(&list_connect, meno, pomenovania_statusov, POCET_STATUSOV, data_odoslanie_suboru);
				break;
			}
			case 'i':
			{
				invertuj_indikator_subory(&indikator_subory);
				break;
			}
			case 'x':
			{
				ukoncenie_programu(&indikator_pokracuj, chat_socket, discovery_socket, pole_thread, POLOZKY_THREAD);
				break;
			}
			case 'z':
			{
				uprav_hodnotu_debug();
				break;
			}
			case '?':
			{
				zoznam_menu();
				break;
			}
			default:
			{
				nespravna_hodnota();
			}
		}
	}

	/**
	 * Cakanie na ukoncenie vlakna a vycistenie dat pridelenych vlaknam.
	 */
	for (int i = 0; i < POLOZKY_THREAD; i++) {
		pthread_join(*(pole_thread[i]), NULL);
	}

	disposeDLL(&list_connect);
	disposeDLL(data_accept->list_accept);

	vycisti_buffre_hudba(&hudba_buffer_sprava, &hudba_buffer_odhlasenie);

	freeifaddrs(prva_polozka_zoznamu);

	dealokuj_malloc((void **)&data_odoslanie_suboru);
	dealokuj_malloc((void **)&data_read_write);
	dealokuj_malloc((void **)&data_accept);
	dealokuj_malloc((void **)&data_discovery_zistovanie);
	dealokuj_malloc((void **)&data_discovery);

	vypis_nadpis("KONIEC");
	return EXIT_SUCCESS;
}
