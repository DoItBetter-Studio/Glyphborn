#include "dialogue/dialogue.h"
#include <stdlib.h>
#include <string.h>

DialogueDatabase g_Dialogue = {0};

void dialogue_load(void) {
    const Blob* blob = &g_DialogueTable[0];
    if (!blob || !blob->data) {
        return;
    }

    const uint8_t* ptr = blob->data;

    uint32_t magic = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);
    if (magic != DIALOGUE_MAGIC) return;

    uint16_t version = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);
    if (version != DIALOGUE_VERSION) return;

    g_Dialogue.conversation_count = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    g_Dialogue.node_count = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    g_Dialogue.conversations = malloc(sizeof(DialogueConversation) * g_Dialogue.conversation_count);
    g_Dialogue.nodes = malloc(sizeof(DialogueNode) * g_Dialogue.node_count);

    for (uint32_t i = 0; i < g_Dialogue.conversation_count; i++) {
        uint16_t name_length = *(const uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        g_Dialogue.conversations[i].name = (const char*)ptr;
        g_Dialogue.conversations[i].name_length = name_length;
        ptr += name_length;

        g_Dialogue.conversations[i].entry_index = *(const int32_t*)ptr;
        ptr += sizeof(int32_t);

        g_Dialogue.conversations[i].start_node_index = *(const uint32_t*)ptr;
        ptr += sizeof(uint32_t);

        g_Dialogue.conversations[i].node_count = *(const uint32_t*)ptr;
        ptr += sizeof(uint32_t);
    }

    for (uint32_t i = 0; i < g_Dialogue.node_count; i++) {
        DialogueNode* node = &g_Dialogue.nodes[i];

        node->speaker_id = *(const uint32_t*)ptr;
        ptr += sizeof(uint32_t);

        node->text_id = *(const uint32_t*)ptr;
        ptr += sizeof(uint32_t);

        node->next_node_index = *(const int32_t*)ptr;
        ptr += sizeof(int32_t);

        node->flags = *(const uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        node->choice_count = *(const uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        if (node->choice_count > 0) {
            node->choices = (const DialogueChoice*)ptr;
            ptr += sizeof(DialogueChoice) * node->choice_count;
        } else {
            node->choices = NULL;
        }
    }
}

void dialogue_free(void) {
    if (g_Dialogue.conversations) {
        free(g_Dialogue.conversations);
        g_Dialogue.conversations = NULL;
    }
    if (g_Dialogue.nodes) {
        free(g_Dialogue.nodes);
        g_Dialogue.nodes = NULL;
    }
    // g_Dialogue.nodes[i].choices is never freed here — it was never
    // separately allocated, it points straight into the blob.
    g_Dialogue.conversation_count = 0;
    g_Dialogue.node_count = 0;
}

DialogueConversation* dialogue_get_conversation(uint32_t index) {
    if (index >= g_Dialogue.conversation_count) return NULL;
    return &g_Dialogue.conversations[index];
}

DialogueNode* dialogue_get_node(uint32_t index) {
    if (index >= g_Dialogue.node_count) return NULL;
    return &g_Dialogue.nodes[index];
}