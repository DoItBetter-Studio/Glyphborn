#ifndef AUDIO_H
#define AUDIO_H

#define SAMPLE_RATE 44100
#define BUFFER_SAMPLES 2048
#define NUM_BUFFERS 2
#define NUM_CHANNELS 2

#include <stdbool.h>
#include <string.h>

void audio_init(void);
void audio_update(void);
void audio_shutdown(void);

void audio_play_music(const signed char* music_data, int length);
void audio_play_sound(const signed char* sound_data, int length);

#endif // !AUDIO_H
