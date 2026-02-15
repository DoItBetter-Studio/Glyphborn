#ifdef __linux__

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

#include "audio.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct
{
	snd_pcm_t* handle;

	int16_t buffers[NUM_BUFFERS][BUFFER_SAMPLES];
	int buffer_index;

	// Currently playing (could be bgm, jingle, or sfx)
	const unsigned char* playing_sample;
	int sample_length;
	int sample_position;
	bool is_playing;

	// Only used for channel 0 (music): saved state to resume after an interrupt/jingle
	const unsigned char* saved_sample;
	int saved_length;
	int saved_position;
	bool interrupted;
} AudioChannel;

static AudioChannel audio_channels[NUM_CHANNELS];

static int alsa_open_playback(snd_pcm_t** out, unsigned rate_hz)
{
	int err = snd_pcm_open(out, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) return err;

	err = snd_pcm_set_params(*out, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 1, rate_hz, 1, 500000);
	return err;
}

static inline int16_t s8_to_s16(signed char s8)
{
	return (int16_t)((int16_t)s8 << 8);
}

static void fill_buffer(AudioChannel* channel, int16_t* buffer, int samplesPerBuffer)
{
	for (int i = 0; i < samplesPerBuffer; ++i)
	{
		int16_t sample = 0;

		if (channel->playing_sample && channel->sample_position < channel->sample_length)
		{
			sample = s8_to_s16(channel->playing_sample[channel->sample_position++]);
		}
		else
		{
			channel->playing_sample = NULL;

			if (channel == &audio_channels[0] && channel->interrupted && channel->saved_sample)
			{
				channel->playing_sample = channel->saved_sample;
				channel->sample_length = channel->saved_length;
				channel->sample_position = channel->saved_position;
				channel->interrupted = false;

				if (channel->playing_sample && channel->sample_position < channel->sample_length)
				{
					sample = s8_to_s16(channel->playing_sample[channel->sample_position++]);
				}
				else
				{
					channel->playing_sample = NULL;
				}
			}
		}

		buffer[i] = sample;
	}

	if (channel->playing_sample == NULL && !(channel == &audio_channels[0] && channel->interrupted))
		channel->is_playing = false;
}

void audio_init(void)
{
	for (int ch = 0; ch < NUM_CHANNELS; ++ch)
	{
		AudioChannel* channel = &audio_channels[ch];
		channel->handle = NULL;
		channel->buffer_index = 0;
		channel->playing_sample = NULL;
		channel->sample_length = 0;
		channel->sample_position = 0;
		channel->is_playing = false;

		channel->saved_sample = NULL;
		channel->saved_length = 0;
		channel->saved_position = 0;
		channel->interrupted = false;

		int err = alsa_open_playback(&channel->handle, SAMPLE_RATE);
		if (err < 0)
		{
			fprintf(stderr, "ALSA: cannot open playback channel %d (%s)\n", ch, snd_strerror(err));
			channel->handle = NULL;
			continue;
		}

		for (int b = 0; b < NUM_BUFFERS; ++b)
		{
			for (int sample = 0; sample < BUFFER_SAMPLES; ++sample)
				channel->buffers[b][sample] = 0;

			snd_pcm_writei(channel->handle, channel->buffers[b], BUFFER_SAMPLES);
		}
	}
}

void audio_update(void)
{
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		AudioChannel* channel = &audio_channels[ch];
		if (!channel->handle) continue;

		if (channel->is_playing)
		{
			int16_t* buffer = channel->buffers[channel->buffer_index];
			fill_buffer(channel, buffer, BUFFER_SAMPLES);

			snd_pcm_sframes_t wrote = snd_pcm_writei(channel->handle, buffer, BUFFER_SAMPLES);
			if (wrote < 0)
				snd_pcm_recover(channel->handle, wrote, 0);

			channel->buffer_index = (channel->buffer_index + 1) % NUM_BUFFERS;
		}
	}
}

void audio_shutdown(void)
{
	for (int ch = 0; ch < NUM_CHANNELS; ++ch)
	{
		AudioChannel* channel = &audio_channels[ch];
		if (channel->handle)
		{
			int err = snd_pcm_drain(channel->handle);
			if (err < 0) snd_pcm_recover(channel->handle, err, 0);
			snd_pcm_close(channel->handle);
			channel->handle = NULL;
		}
	}
}

static void audio_play_sample_channel(int channelIndex, const unsigned char* sample_data, int length)
{
	if (channelIndex < 0 || channelIndex >= NUM_CHANNELS) return;

	AudioChannel* channel = &audio_channels[channelIndex];
	if (!channel->handle) return;

	channel->playing_sample = sample_data;
	channel->sample_length = length;
	channel->sample_position = 0;
	channel->is_playing = true;
	channel->buffer_index = 0;
}

void audio_play_music(const unsigned char* music_data, int length, bool interrupt)
{
	AudioChannel* bgm = &audio_channels[0];

	if (interrupt && bgm->is_playing && bgm->playing_sample)
	{
		bgm->saved_sample = bgm->playing_sample;
		bgm->saved_length = bgm->sample_length;
		bgm->saved_position = bgm->sample_position;
		bgm->interrupted = true;
	}
	else
	{
		bgm->saved_sample = NULL;
		bgm->saved_length = 0;
		bgm->saved_position = 0;
		bgm->interrupted = false;
	}

	audio_play_sample_channel(0, music_data, length);
}

void audio_play_sound(const unsigned char* sound_data, int length)
{
	audio_play_sample_channel(1, sound_data, length);
}
#endif 
