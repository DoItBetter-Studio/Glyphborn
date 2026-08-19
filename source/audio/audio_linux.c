#ifdef __linux__

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <alsa/asoundlib.h>
#include "audio/audio.h"
#include "audio/gbaud.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Single output device — all channels sum into one stream
static snd_pcm_t* g_pcm;
static int16_t    g_mix_buffers[NUM_BUFFERS][BUFFER_SAMPLES];
static int        g_buffer_index;
static int32_t accum[BUFFER_SAMPLES];

typedef struct
{
	uint32_t loop_start;
	uint32_t loop_end;
	bool     has_loop;

	const unsigned char* playing_sample;
	int  sample_length;
	int  sample_position;
	bool is_playing;

	// channel 0 only: saved state to resume after an interrupt/jingle
	const unsigned char* saved_sample;
	int  saved_length;
	int  saved_position;
	bool interrupted;
} AudioChannel;

static AudioChannel audio_channels[NUM_CHANNELS];

// Accumulates one channel's contribution into an int32 buffer.
// Caller zeroes accum before the first channel; we add into it.
static void fill_channel(AudioChannel* channel, int32_t* accum, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (channel->playing_sample && channel->sample_position < channel->sample_length)
		{
			signed char s8 = (signed char)channel->playing_sample[channel->sample_position++];
			accum[i] += (int32_t)s8 << 8;

			if (channel->has_loop && channel->sample_position >= (int)channel->loop_end)
				channel->sample_position = (int)channel->loop_start;
		}
		else if (channel == &audio_channels[0] && channel->interrupted && channel->saved_sample)
		{
			channel->playing_sample  = channel->saved_sample;
			channel->sample_length   = channel->saved_length;
			channel->sample_position = channel->saved_position;
			channel->interrupted     = false;

			if (channel->sample_position < channel->sample_length)
			{
				signed char s8 = (signed char)channel->playing_sample[channel->sample_position++];
				accum[i] += (int32_t)s8 << 8;
			}
			else
			{
				channel->playing_sample = NULL;
			}
		}
	}

	if (!channel->playing_sample && !(channel == &audio_channels[0] && channel->interrupted))
		channel->is_playing = false;
}

// Mixes all active channels into out[], clamping the int32 sum to int16.
static void mix_to_buffer(int16_t* out, int count)
{
	memset(accum, 0, count * sizeof(int32_t));

	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		AudioChannel* channel = &audio_channels[ch];
		if (channel->is_playing)
			fill_channel(channel, accum, count);
	}

	for (int i = 0; i < count; i++)
	{
		int32_t s = accum[i];
		if (s >  32767) s =  32767;
		if (s < -32768) s = -32768;
		out[i] = (int16_t)s;
	}
}

void audio_init(void)
{
	memset(audio_channels, 0, sizeof(audio_channels));

	int err = snd_pcm_open(&g_pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0)
	{
		fprintf(stderr, "ALSA: cannot open playback device (%s)\n", snd_strerror(err));
		g_pcm = NULL;
		return;
	}

	err = snd_pcm_set_params(g_pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
	                          1, SAMPLE_RATE, 1, 500000);
	if (err < 0)
	{
		fprintf(stderr, "ALSA: cannot set params (%s)\n", snd_strerror(err));
		snd_pcm_close(g_pcm);
		g_pcm = NULL;
		return;
	}

	// Prime with silence so the device is already running when play starts
	for (int i = 0; i < NUM_BUFFERS; i++)
	{
		memset(g_mix_buffers[i], 0, sizeof(g_mix_buffers[i]));
		snd_pcm_writei(g_pcm, g_mix_buffers[i], BUFFER_SAMPLES);
	}

	g_buffer_index = 0;
}

void audio_update(void)
{
	if (!g_pcm) return;

	mix_to_buffer(g_mix_buffers[g_buffer_index], BUFFER_SAMPLES);

	snd_pcm_sframes_t wrote = snd_pcm_writei(g_pcm, g_mix_buffers[g_buffer_index], BUFFER_SAMPLES);
	if (wrote < 0)
		snd_pcm_recover(g_pcm, wrote, 0);

	g_buffer_index = (g_buffer_index + 1) % NUM_BUFFERS;
}

void audio_shutdown(void)
{
	if (!g_pcm) return;

	snd_pcm_drain(g_pcm);
	snd_pcm_close(g_pcm);
	g_pcm = NULL;
}

static void audio_play_sample_channel(int channelIndex, const unsigned char* sample_data)
{
	if (channelIndex < 0 || channelIndex >= NUM_CHANNELS) return;

	AudioChannel* channel = &audio_channels[channelIndex];
	const GbaudHeader* hdr = (const GbaudHeader*)sample_data;
	const unsigned char* pcm = sample_data + hdr->data_offset;

	channel->has_loop        = (hdr->flags & GBAUD_FLAG_LOOP) != 0;
	channel->loop_start      = hdr->loop_start;
	channel->loop_end        = hdr->loop_end;
	channel->playing_sample  = pcm;
	channel->sample_length   = (int)hdr->sample_count;
	channel->sample_position = 0;
	channel->is_playing      = true;
}

void audio_play_music(const unsigned char* music_data, bool interrupt)
{
	AudioChannel* bgm = &audio_channels[0];

	if (interrupt && bgm->is_playing && bgm->playing_sample)
	{
		bgm->saved_sample   = bgm->playing_sample;
		bgm->saved_length   = bgm->sample_length;
		bgm->saved_position = bgm->sample_position;
		bgm->interrupted    = true;
	}
	else
	{
		bgm->saved_sample   = NULL;
		bgm->saved_length   = 0;
		bgm->saved_position = 0;
		bgm->interrupted    = false;
	}

	audio_play_sample_channel(0, music_data);
}

void audio_play_sound(const unsigned char* sound_data)
{
	for (int ch = 1; ch < NUM_CHANNELS; ch++)
	{
		if (!audio_channels[ch].is_playing)
		{
			audio_play_sample_channel(ch, sound_data);
			return;
		}
	}
	// All channels busy — steal channel 1 (oldest by convention)
	audio_play_sample_channel(1, sound_data);
}

#endif