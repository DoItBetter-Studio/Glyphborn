#ifdef _WIN32

#include "audio.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

typedef struct
{
	HWAVEOUT hWaveOut;
	WAVEHDR buffers[NUM_BUFFERS];
	short samples[NUM_BUFFERS][BUFFER_SAMPLES];
	int buffer_index;
	uint32_t loop_start;   // 0 if no loop
	uint32_t loop_end;     // 0 if no loop
	bool     has_loop;

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

static void fill_buffer(AudioChannel* channel, short* buffer, int samplesPerBuffer)
{
	for (int i = 0; i < samplesPerBuffer; i++)
	{
		short sample = 0;

		if (channel->playing_sample && channel->sample_position < channel->sample_length)
		{
			int s8 = channel->playing_sample[channel->sample_position++];
			sample = (short)(s8 << 8); // Convert 8-bit signed to 16-bit signed

			if (channel->has_loop && channel->sample_position >= (int)channel->loop_end)
				channel->sample_position = (int)channel->loop_start;
		}
		else if (channel == &audio_channels[0] && channel->interrupted && channel->saved_sample)
		{
			channel->playing_sample = channel->saved_sample;
			channel->sample_length = channel->saved_length;
			channel->sample_position = channel->saved_position;
			channel->interrupted = false;

			if (channel->sample_position < channel->sample_length)
			{
				int s8 = channel->playing_sample[channel->sample_position++];
				sample = (short)(s8 << 8);
			}
			else
			{
				channel->playing_sample = NULL;
			}
		}

		buffer[i] = sample;
	}

	if (!channel->playing_sample && !(channel == &audio_channels[0] && channel->interrupted))
		channel->is_playing = false;
}

void audio_init(void)
{
	WAVEFORMATEX fmt =
	{
		.wFormatTag = WAVE_FORMAT_PCM,
		.nChannels = 1,
		.nSamplesPerSec = SAMPLE_RATE,
		.wBitsPerSample = 16,
		.nBlockAlign = 2,
		.nAvgBytesPerSec = SAMPLE_RATE * 2,
		.cbSize = 0
	};

	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		AudioChannel* channel = &audio_channels[ch];
		channel->playing_sample = NULL;
		channel->sample_length = 0;
		channel->sample_position = 0;
		channel->is_playing = false;
		channel->buffer_index = 0;

		if (waveOutOpen(&channel->hWaveOut, WAVE_MAPPER, &fmt, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		{
			MessageBox(NULL, "Failed to open audio device.", "Error", MB_OK | MB_ICONERROR);
			return;
		}
		else
		{
			for (int i = 0; i < NUM_BUFFERS; ++i)
			{
				fill_buffer(channel, channel->samples[i], BUFFER_SAMPLES);
				channel->buffers[i].lpData = (LPSTR)channel->samples[i];
				channel->buffers[i].dwBufferLength = BUFFER_SAMPLES * sizeof(short);
				channel->buffers[i].dwFlags = 0;

				waveOutPrepareHeader(channel->hWaveOut, &channel->buffers[i], sizeof(WAVEHDR));
				waveOutWrite(channel->hWaveOut, &channel->buffers[i], sizeof(WAVEHDR));
			}
		}
	}
}

void audio_update(void)
{
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		AudioChannel* channel = &audio_channels[ch];
		if (!channel->is_playing || !channel->playing_sample)
			continue;

		WAVEHDR* hdr = &channel->buffers[channel->buffer_index];

		if (hdr->dwFlags & WHDR_DONE)
		{
			fill_buffer(channel, (short*)hdr->lpData, BUFFER_SAMPLES);
			waveOutWrite(channel->hWaveOut, hdr, sizeof(WAVEHDR));

			channel->buffer_index = (channel->buffer_index + 1) % NUM_BUFFERS;

			// If sample ended, mark channel as stopped
			if (channel->playing_sample == NULL && channel == &audio_channels[0] && channel->interrupted)
			{
				fill_buffer(channel, (short*)hdr->lpData, BUFFER_SAMPLES);
				waveOutWrite(channel->hWaveOut, hdr, sizeof(WAVEHDR));
			}
		}
	}
}

void audio_shutdown(void)
{
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		AudioChannel* channel = &audio_channels[ch];
		if (!channel->hWaveOut)
			continue;

		waveOutReset(channel->hWaveOut);
		for (int i = 0; i < NUM_BUFFERS; i++)
		{
			waveOutUnprepareHeader(channel->hWaveOut, &channel->buffers[i], sizeof(WAVEHDR));
		}
		waveOutClose(channel->hWaveOut);
		channel->hWaveOut = NULL;
	}
}

static void audio_play_sample_channel(int channelIndex, const unsigned char* sample_data)
{
	if (channelIndex < 0 || channelIndex >= NUM_CHANNELS)
		return;

	AudioChannel* channel = &audio_channels[channelIndex];

	const GbaudHeader* hdr = (const GbaudHeader*)sample_data;
	const unsigned char* pcm = sample_data + hdr->data_offset;
	int pcm_length = (int)hdr->sample_count;

	channel->has_loop	= (hdr->flags & GBAUD_FLAG_LOOP) != 0;
	channel->loop_start = hdr->loop_start;
	channel->loop_end	= hdr-> loop_end;
	
	if (channelIndex == 0)
	{
		waveOutReset(channel->hWaveOut);
		for (int i = 0; i < NUM_BUFFERS; ++i)
			waveOutUnprepareHeader(channel->hWaveOut, &channel->buffers[i], sizeof(WAVEHDR));

		for (int i = 0; i < NUM_BUFFERS; ++i)
		{
			channel->buffers[i].dwFlags = 0;
			waveOutPrepareHeader(channel->hWaveOut, &channel->buffers[i], sizeof(WAVEHDR));
		}

		channel->buffer_index = 0;

		for (int i = 0; i < NUM_BUFFERS; ++i)
		{
			fill_buffer(channel, channel->samples[i], BUFFER_SAMPLES);
			channel->buffers[i].lpData = (LPSTR)channel->samples[i];
			channel->buffers[i].dwBufferLength = BUFFER_SAMPLES * sizeof(short);
			channel->buffers[i].dwFlags = 0;

			waveOutPrepareHeader(channel->hWaveOut, &channel->buffers[i], sizeof(WAVEHDR));
			waveOutWrite(channel->hWaveOut, &channel->buffers[i], sizeof(WAVEHDR));
		}
	}

	channel->playing_sample = pcm;
	channel->sample_length = pcm_length;
	channel->sample_position = 0;
	channel->is_playing = true;
}

void audio_play_music(const unsigned char* music_data, bool interrupt)
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

	audio_play_sample_channel(0, music_data);
}

void audio_play_sound(const unsigned char* sound_data)
{
	// Channel 0 is always music, SFX gets 1..NUM_CHANNELS-1
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