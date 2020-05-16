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
 *
 */
sfSoundBuffer *priprav_jednu_hudbu(const char *nazov_suboru) {
	debug_sprava("Nahravam zvukovy subor pre program...");

	sfSoundBuffer *buffer = sfSoundBuffer_createFromFile(nazov_suboru);
	osetri_chybu_suboru("Nepodarilo sa otvorit hudobny subor pre program.", buffer);
	return buffer;
}

/**
 *
 */
void priprav_hudbu(sfSoundBuffer **hudba_buffer_sprava, sfSoundBuffer **hudba_buffer_odhlasenie) {
	*hudba_buffer_sprava = priprav_jednu_hudbu(ZVUCKA_SPRAVA);
	*hudba_buffer_odhlasenie = priprav_jednu_hudbu(ZVUCKA_ODHLASENIE);
}

/**
 *
 */
void vycisti_buffre_hudba(sfSoundBuffer **hudba_buffer_sprava, sfSoundBuffer **hudba_buffer_odhlasenie) {
	sfSoundBuffer_destroy(*hudba_buffer_sprava);
	*hudba_buffer_sprava = NULL;

	sfSoundBuffer_destroy(*hudba_buffer_odhlasenie);
	*hudba_buffer_odhlasenie = NULL;
}

/**
 *
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
 *
 */
void prehraj_zvuk_sprava(sfSoundBuffer *hudba_buffer_sprava) {
	// zvuk nie je joinovany
	debug_sprava("Prehrava sa zvuk k prijatej sprave...");

	pthread_t thread_prehravanie;
	pthread_create(&thread_prehravanie, NULL, &prehratie_zvuku, hudba_buffer_sprava);
}

/**
 *
 */
void prehraj_zvuk_odhlasenie(sfSoundBuffer *hudba_buffer_odhlasenie) {
	debug_sprava("Prehrava sa zvuk k odhlaseniu...");

	pthread_t thread_prehravanie;
	pthread_create(&thread_prehravanie, NULL, &prehratie_zvuku, hudba_buffer_odhlasenie);
}
