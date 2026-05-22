#ifdef __YGGDRASIL__
#include "audio.h"
#include "gbaud.h"
#include <yggdrasil.h>

typedef struct {
    int8_t  buffers[NUM_BUFFERS][BUFFER_SAMPLES];
    int     buffer_index;
    uint32_t loop_start;
    uint32_t loop_end;
    bool     has_loop;

    const unsigned char *playing_sample;
    int                  sample_length;
    int                  sample_position;
    bool                 is_playing;

    const unsigned char *saved_sample;
    int                  saved_length;
    int                  saved_position;
    bool                 interrupted;
} AudioChannel;

static AudioChannel audio_channels[NUM_CHANNELS];

static inline int8_t clamp_s8(int32_t v) {
    if (v >  127) return  127;
    if (v < -128) return -128;
    return (int8_t)v;
}

static void fill_buffer(AudioChannel *ch, int8_t *buf, int count) {
    for (int i = 0; i < count; i++) {
        int8_t sample = 0;

        if (ch->playing_sample &&
            ch->sample_position < ch->sample_length) {
            sample = (int8_t)ch->playing_sample[ch->sample_position];
            ch->sample_position++;

            if (ch->has_loop && ch->sample_position >= (int)ch->loop_end)
                ch->sample_position = (int)ch->loop_start;
        } else {
            ch->playing_sample = NULL;

            if (ch == &audio_channels[0] &&
                ch->interrupted && ch->saved_sample) {
                ch->playing_sample  = ch->saved_sample;
                ch->sample_length   = ch->saved_length;
                ch->sample_position = ch->saved_position;
                ch->interrupted     = false;

                if (ch->playing_sample &&
                    ch->sample_position < ch->sample_length) {
                    sample = (int8_t)ch->playing_sample[ch->sample_position];
                    ch->sample_position++;

                    if (ch->has_loop && ch->sample_position >= (int)ch->loop_end)
                        ch->sample_position = (int)ch->loop_start;
                } else {
                    ch->playing_sample = NULL;
                }
            }
        }
        buf[i] = sample;
    }

    if (!ch->playing_sample &&
        !(ch == &audio_channels[0] && ch->interrupted))
        ch->is_playing = false;
}

void audio_init(void) {
    memset(audio_channels, 0, sizeof(audio_channels));
}

void audio_update(void) {
    if (!ac97_is_initialized()) return;
    if (ac97_needs_data()) {
        static int8_t buf0[BUFFER_SAMPLES];
        static int8_t buf1[BUFFER_SAMPLES];
        static int8_t mixed[BUFFER_SAMPLES];

        fill_buffer(&audio_channels[0], buf0, BUFFER_SAMPLES);
        fill_buffer(&audio_channels[1], buf1, BUFFER_SAMPLES);

        for (int i = 0; i < BUFFER_SAMPLES; i++)
            mixed[i] = clamp_s8((int32_t)buf0[i] + (int32_t)buf1[i]);

        ac97_submit(mixed, BUFFER_SAMPLES);
    }
}

void audio_shutdown(void) {
    pit_set_callback_hz(NULL, 0);
}

void audio_play_music(const unsigned char *data, bool interrupt) {
    AudioChannel *bgm = &audio_channels[0];

    if (interrupt && bgm->is_playing && bgm->playing_sample) {
        bgm->saved_sample   = bgm->playing_sample;
        bgm->saved_length   = bgm->sample_length;
        bgm->saved_position = bgm->sample_position;
        bgm->interrupted    = true;
    } else {
        bgm->saved_sample   = NULL;
        bgm->saved_length   = 0;
        bgm->saved_position = 0;
        bgm->interrupted    = false;
    }

    const GbaudHeader *hdr  = (const GbaudHeader *)data;
    const unsigned char *pcm = data + hdr->data_offset;

    bgm->has_loop        = (hdr->flags & GBAUD_FLAG_LOOP) != 0;
    bgm->loop_start      = hdr->loop_start;
    bgm->loop_end        = hdr->loop_end;
    bgm->playing_sample  = pcm;
    bgm->sample_length   = (int)hdr->sample_count;
    bgm->sample_position = 0;
    bgm->is_playing      = true;
}

void audio_play_sound(const unsigned char *data) {
    AudioChannel *sfx    = &audio_channels[1];

    const GbaudHeader *hdr  = (const GbaudHeader *)data;
    const unsigned char *pcm = data + hdr->data_offset;

    sfx->has_loop        = (hdr->flags & GBAUD_FLAG_LOOP) != 0;
    sfx->loop_start      = hdr->loop_start;
    sfx->loop_end        = hdr->loop_end;
    sfx->playing_sample  = pcm;
    sfx->sample_length   = (int)hdr->sample_count;
    sfx->sample_position = 0;
    sfx->is_playing      = true;
}
#endif /* __YGGDRASIL__ */