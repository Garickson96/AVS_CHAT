#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>

#include "../headers/universal.h"
#include "../headers/hudba_chat.h"

#define ZVUCKA_SPRAVA "sprava.wav"
#define ZVUCKA_ODHLASENIE "odhlasenie.wav"

#define CAS_CAKANIA 5000000

/**
 * Nacita zvukove subory prilozene k programu v adresari Debug. Vytvori im buffer v halde. V pripade chyby oznam.
 * Spracovanie zvuku (jeho prehratie) je cez kniznicu SFML. Pouzivat ako private.
 */
sfSoundBuffer *priprav_jednu_hudbu(const char *nazov_suboru) {
	debug_sprava("Nahravam zvukovy subor pre program...");

	sfSoundBuffer *buffer = sfSoundBuffer_createFromFile(nazov_suboru);
	osetri_chybu_suboru("Nepodarilo sa otvorit hudobny subor pre program.", buffer);
	return buffer;
}

/**
 * Pre beh programu sa pripravia 2 zvucky - pre prijatu spravu a pre notifikaciu odhlasenia ineho pouzivatela zo systemu.
 */
void priprav_hudbu(sfSoundBuffer **hudba_buffer_sprava, sfSoundBuffer **hudba_buffer_odhlasenie) {
	*hudba_buffer_sprava = priprav_jednu_hudbu(ZVUCKA_SPRAVA);
	*hudba_buffer_odhlasenie = priprav_jednu_hudbu(ZVUCKA_ODHLASENIE);
}

/**
 * Vycisti buffre vytvorene pre hudbu. Pouzivaju sa funkcie dodane CSFML.
 */
void vycisti_buffre_hudba(sfSoundBuffer **hudba_buffer_sprava, sfSoundBuffer **hudba_buffer_odhlasenie) {
	sfSoundBuffer_destroy(*hudba_buffer_sprava);
	*hudba_buffer_sprava = NULL;

	sfSoundBuffer_destroy(*hudba_buffer_odhlasenie);
	*hudba_buffer_odhlasenie = NULL;
}

/**
 * Prehratie zvuku prebieha v samostatnom vlakne - je to blokujuca operacia. Pozor, prehravanie trva urcity cas a buffer spolu so
 * sound musia byt dostupne v okamihu prehravania - inak to nepojde. Preto je tu na konci dodane uspatie - okolo 5 sekund.
 */
void *prehratie_zvuku(void *data) {
	sfSoundBuffer *buffer = (sfSoundBuffer *)data;

	sfSound *zvuk = sfSound_create();
	osetri_chybu_suboru("Nepodarilo sa vytvorit hudbu na prehratie.", zvuk);

	sfSound_setBuffer(zvuk, buffer);
	sfSound_play(zvuk);

	usleep(CAS_CAKANIA);
	sfSound_destroy(zvuk);

	return NULL;
}

/**
 * Vytvori vlakno k prehratiu zvuku spravy. Do vlakna sa posle buffer so zvukom vo formate WAV. Vlakna nie su joinovane.
 */
void prehraj_zvuk_sprava(sfSoundBuffer *hudba_buffer_sprava) {
	// zvuk nie je joinovany
	debug_sprava("Prehrava sa zvuk k prijatej sprave...");

	pthread_t thread_prehravanie;
	pthread_create(&thread_prehravanie, NULL, &prehratie_zvuku, hudba_buffer_sprava);
}

/**
 * Vytvori vlakno k prehratiu zvuku pre odhlasenie. Do vlakna sa posle buffer so zvukom vo formate WAV. Vlakna nie su joinovane.
 */
void prehraj_zvuk_odhlasenie(sfSoundBuffer *hudba_buffer_odhlasenie) {
	debug_sprava("Prehrava sa zvuk k odhlaseniu...");

	pthread_t thread_prehravanie;
	pthread_create(&thread_prehravanie, NULL, &prehratie_zvuku, hudba_buffer_odhlasenie);
}
