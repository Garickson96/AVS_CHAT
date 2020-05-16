#ifndef HEADERS_HUDBA_CHAT_H_
#define HEADERS_HUDBA_CHAT_H_

#include <SFML/Audio.h>

void priprav_hudbu(sfSoundBuffer **hudba_buffer_sprava, sfSoundBuffer **hudba_buffer_odhlasenie);
void vycisti_buffre_hudba(sfSoundBuffer **hudba_buffer_sprava, sfSoundBuffer **hudba_buffer_odhlasenie);

void *prehratie_zvuku(void *data);

void prehraj_zvuk_sprava(sfSoundBuffer *hudba_buffer_sprava);
void prehraj_zvuk_odhlasenie(sfSoundBuffer *hudba_buffer_odhlasenie);

#endif /* HEADERS_HUDBA_CHAT_H_ */
