/* =========================================================================
   gbaud.h  —  Glyphborn Audio format, shared by editor (Echo) and runtime
   =========================================================================
 
   Binary layout (36 bytes, little-endian, no padding needed — all fields
   are naturally aligned):
 
     Offset  Size  Field
     ------  ----  -----
      0       4    magic[4]       'G','B','A','U'
      4       2    version        1
      6       4    sample_rate    e.g. 44100
     10       1    channels       1  (mono)
     11       1    bit_depth      8
     12       1    flags          bit 0 = GBAUD_FLAG_LOOP
     13       1    _pad           0
     14       4    sample_count   number of s8 PCM samples (trimmed length)
     18       4    trim_start     absolute sample index in original recording
     22       4    trim_end       absolute sample index in original recording
     26       4    loop_start     relative to start of PCM block (i.e. 0-based)
     30       4    loop_end       relative to start of PCM block
     34       4    data_offset    byte offset from file start to PCM data (= 36)
   [ 36 ]         raw signed 8-bit PCM samples follow immediately
 
   Usage in runtime:
     const GbaudHeader* hdr = (const GbaudHeader*)asset_ptr;
     const unsigned char* pcm = asset_ptr + hdr->data_offset;
     audio_play_music(pcm, hdr->sample_count, false);
 
   The runtime fill_buffer must check GBAUD_FLAG_LOOP and seek back to
   loop_start when sample_position reaches loop_end.  See patch below.
   ========================================================================= */

#ifndef GBAUD_H
#define GBAUD_H

#include <stdint.h>

#define GBAUD_MAGIC_0  'G'
#define GBAUD_MAGIC_1  'B'
#define GBAUD_MAGIC_2  'A'
#define GBAUD_MAGIC_3  'U'
#define GBAUD_VERSION  1
#define GBAUD_FLAG_LOOP  0x01

#pragma pack(push, 1)
typedef struct
{
    char     magic[4];       /* 'G','B','A','U'                          */
    uint16_t version;        /* 1                                        */
    uint32_t sample_rate;    /* 44100                                    */
    uint8_t  channels;       /* 1                                        */
    uint8_t  bit_depth;      /* 8                                        */
    uint8_t  flags;          /* GBAUD_FLAG_LOOP etc.                     */
    uint8_t  _pad;           /* 0                                        */
    uint32_t sample_count;   /* trimmed PCM length in samples            */
    uint32_t trim_start;     /* absolute, informational                  */
    uint32_t trim_end;       /* absolute, informational                  */
    uint32_t loop_start;     /* relative to PCM block start              */
    uint32_t loop_end;       /* relative to PCM block start              */
    uint32_t data_offset;    /* byte offset from file start → PCM data   */
} GbaudHeader;
#pragma pack(pop)

/* Validate at compile time that the header is exactly 36 bytes */
typedef char gbaud_header_size_check[ (sizeof(GbaudHeader) == 38) ? 1 : -1 ];

#endif /* GBAUD_H */