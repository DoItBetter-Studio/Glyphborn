#ifdef _WIN32

#include "audio/audio.h"
#include "audio/gbaud.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Single output device — all channels sum into one stream
static HWAVEOUT g_wave_out;
static WAVEHDR  g_wave_headers[NUM_BUFFERS];
static int16_t  g_mix_buffers[NUM_BUFFERS][BUFFER_SAMPLES];
static int32_t      g_buffer_index;
static int32_t accum[BUFFER_SAMPLES];

typedef struct
{
	uint32_t loop_start;
	uint32_t loop_end;
	bool     has_loop;

	const unsigned char* playing_sample;
	int32_t  sample_length;
	int32_t  sample_position;
	bool is_playing;

	// channel 0 only: saved state to resume after an interrupt/jingle
	const unsigned char* saved_sample;
	int32_t  saved_length;
	int32_t  saved_position;
	bool interrupted;
} AudioChannel;

static AudioChannel audio_channels[NUM_CHANNELS];

// Accumulates one channel's contribution into an int32 buffer.
// Caller zeroes accum before the first channel; we add into it.
static void fill_channel(AudioChannel* channel, int32_t* accum, int32_t count)
{
	for (int32_t i = 0; i < count; i++)
	{
		if (channel->playing_sample && channel->sample_position < channel->sample_length)
		{
			signed char s8 = (signed char)channel->playing_sample[channel->sample_position++];
			accum[i] += (int32_t)s8 << 8;

			if (channel->has_loop && channel->sample_position >= (int32_t)channel->loop_end)
				channel->sample_position = (int32_t)channel->loop_start;
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
static void mix_to_buffer(int16_t* out, int32_t count)
{
	memset(accum, 0, count * sizeof(int32_t));

	for (int32_t ch = 0; ch < NUM_CHANNELS; ch++)
	{
		AudioChannel* channel = &audio_channels[ch];
		if (channel->is_playing)
			fill_channel(channel, accum, count);
	}

	for (int32_t i = 0; i < count; i++)
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

	WAVEFORMATEX fmt =
	{
		.wFormatTag      = WAVE_FORMAT_PCM,
		.nChannels       = 1,
		.nSamplesPerSec  = SAMPLE_RATE,
		.wBitsPerSample  = 16,
		.nBlockAlign     = 2,
		.nAvgBytesPerSec = SAMPLE_RATE * 2,
		.cbSize          = 0
	};

	if (waveOutOpen(&g_wave_out, WAVE_MAPPER, &fmt, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		MessageBox(NULL, "Failed to open audio device.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	for (int32_t i = 0; i < NUM_BUFFERS; i++)
	{
		memset(g_mix_buffers[i], 0, sizeof(g_mix_buffers[i]));
		g_wave_headers[i].lpData         = (LPSTR)g_mix_buffers[i];
		g_wave_headers[i].dwBufferLength = BUFFER_SAMPLES * sizeof(int16_t);
		g_wave_headers[i].dwFlags        = 0;

		waveOutPrepareHeader(g_wave_out, &g_wave_headers[i], sizeof(WAVEHDR));
		waveOutWrite(g_wave_out, &g_wave_headers[i], sizeof(WAVEHDR));
	}

	g_buffer_index = 0;
}

void audio_update(void)
{
	if (!g_wave_out) return;

	WAVEHDR* hdr = &g_wave_headers[g_buffer_index];

	if (hdr->dwFlags & WHDR_DONE)
	{
		mix_to_buffer((int16_t*)hdr->lpData, BUFFER_SAMPLES);
		waveOutWrite(g_wave_out, hdr, sizeof(WAVEHDR));
		g_buffer_index = (g_buffer_index + 1) % NUM_BUFFERS;
	}
}

void audio_shutdown(void)
{
	if (!g_wave_out) return;

	waveOutReset(g_wave_out);
	for (int32_t i = 0; i < NUM_BUFFERS; i++)
		waveOutUnprepareHeader(g_wave_out, &g_wave_headers[i], sizeof(WAVEHDR));
	waveOutClose(g_wave_out);
	g_wave_out = NULL;
}

static void audio_play_sample_channel(int32_t channelIndex, const unsigned char* sample_data)
{
	if (channelIndex < 0 || channelIndex >= NUM_CHANNELS) return;

	AudioChannel* channel = &audio_channels[channelIndex];
	const GbaudHeader* hdr = (const GbaudHeader*)sample_data;
	const unsigned char* pcm = sample_data + hdr->data_offset;

	channel->has_loop        = (hdr->flags & GBAUD_FLAG_LOOP) != 0;
	channel->loop_start      = hdr->loop_start;
	channel->loop_end        = hdr->loop_end;
	channel->playing_sample  = pcm;
	channel->sample_length   = (int32_t)hdr->sample_count;
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
	for (int32_t ch = 1; ch < NUM_CHANNELS; ch++)
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