#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdint.h>
#include "generated/DialogueTable.h"

#define DIALOGUE_MAGIC 0x4C444247       // "GBDL" (little-endian) — matches ExportCompiler.cs's DialogueMagic
#define DIALOGUE_VERSION 1
#define NO_INDEX 0xFFFFFFFF

typedef struct {
    uint32_t choice_id;
    int32_t next_node_index;
} DialogueChoice;

typedef struct {
    uint32_t speaker_id;
    uint32_t text_id;
    int32_t next_node_index;
    uint16_t flags;
    uint16_t choice_count;
    const DialogueChoice* choices; // points directly into the blob, never separately allocated
} DialogueNode;

typedef struct {
    const char* name;
    uint16_t name_length;
    int32_t entry_index;
    uint32_t start_node_index;
    uint32_t node_count;
} DialogueConversation;

typedef struct {
    uint32_t conversation_count;
    uint32_t node_count;
    DialogueConversation* conversations;
    DialogueNode* nodes;
} DialogueDatabase;

extern DialogueDatabase g_Dialogue;

void dialogue_load(void);
void dialogue_free(void);

DialogueConversation* dialogue_get_conversation(uint32_t index);
DialogueNode* dialogue_get_node(uint32_t index);

#endif // !DIALOGUE_H