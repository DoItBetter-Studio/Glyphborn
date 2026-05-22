#ifndef AUDIO_H
#define AUDIO_H

#define SAMPLE_RATE 44100
#define BUFFER_SAMPLES 2048
#define NUM_BUFFERS 2
#define NUM_CHANNELS 32

#include "gbaud.h"
#include <stdbool.h>
#include <string.h>

void audio_init(void);
void audio_update(void);
void audio_shutdown(void);

void audio_play_music(const unsigned char* music_data, bool interrupt);
void audio_play_sound(const unsigned char* sound_data);

#endif // !AUDIO_H
