#include "dialogue/dialogue_player.h"
#include "dialogue/dialogue.h"
#include "dialogue/locale.h"
#include "ui/ui.h"
#include <string.h>

#define DIALOGUE_TEXT_BUFFER_SIZE 1024
#define DIALOGUE_CHOICE_BUFFER_SIZE 256
#define DIALOGUE_CHOICE_BUTTON_HEIGHT 28
#define DIALOGUE_CHOICE_SPACING 4

// -1, not NO_INDEX — this tracks a node index, and DialogueNode.next_node_index
// is already int32_t with negative meaning "none", so this stays consistent
// with that rather than mixing in the unsigned NO_INDEX convention.
static int32_t g_current_node = -1;

static bool names_equal(const char* a, uint16_t a_len, const char* b, uint16_t b_len)
{
    return a_len == b_len && (a_len == 0 || memcmp(a, b, a_len) == 0);
}

static void goto_node_or_end(int32_t next_node_index)
{
    g_current_node = (next_node_index >= 0) ? next_node_index : -1;
}

// locale_get_string returns length-prefixed, non-null-terminated data
// pointing straight into the loaded blob — ui_draw_text_wrapped needs a
// real C string, so this copies into a caller-owned buffer. Truncates
// silently if the resolved text is longer than buffer_size.
static const char* resolve_cstr(uint32_t text_id, char* buffer, size_t buffer_size)
{
    uint16_t length = 0;
    const char* data = locale_get_string(text_id, &length);
    if (!data) { buffer[0] = '\0'; return buffer; }
 
    size_t copy_len = (length < buffer_size - 1) ? length : buffer_size - 1;
    memcpy(buffer, data, copy_len);
    buffer[copy_len] = '\0';
    return buffer;
}
 
// ui_dialogue_box only takes one text pointer — no separate speaker field —
// so speaker + body get combined here as "Speaker\n\nBody text" into one
// buffer. This is a layout assumption, not something derived from the UI
// widget itself; swap out if you want the speaker rendered separately.
static const char* build_dialogue_text(const DialogueNode* node, char* buffer, size_t buffer_size)
{
    uint16_t speaker_len = 0;
    const char* speaker = locale_get_string(node->speaker_id, &speaker_len);
 
    uint16_t body_len = 0;
    const char* body = locale_get_string(node->text_id, &body_len);
 
    size_t offset = 0;
 
    if (speaker && speaker_len > 0 && offset + 1 < buffer_size)
    {
        size_t copy_len = speaker_len < (buffer_size - offset - 1) ? speaker_len : (buffer_size - offset - 1);
        memcpy(buffer + offset, speaker, copy_len);
        offset += copy_len;
 
        if (offset + 2 < buffer_size)
        {
            buffer[offset++] = '\n';
            buffer[offset++] = '\n';
        }
    }
 
    if (body && body_len > 0 && offset + 1 < buffer_size)
    {
        size_t copy_len = body_len < (buffer_size - offset - 1) ? body_len : (buffer_size - offset - 1);
        memcpy(buffer + offset, body, copy_len);
        offset += copy_len;
    }
 
    buffer[offset] = '\0';
    return buffer;
}
 
void dialogue_start(const char* name, uint16_t name_length)
{
    for (uint32_t i = 0; i < g_Dialogue.conversation_count; i++)
    {
        DialogueConversation* conv = &g_Dialogue.conversations[i];
        if (names_equal(conv->name, conv->name_length, name, name_length))
        {
            dialogue_start_index(i);
            return;
        }
    }
}
 
void dialogue_start_index(uint32_t conversation_index)
{
    if (conversation_index >= g_Dialogue.conversation_count) return;
 
    DialogueConversation* conv = &g_Dialogue.conversations[conversation_index];
    if (conv->entry_index < 0) return;
 
    g_current_node = conv->entry_index;
}
 
void dialogue_end(void)
{
    g_current_node = -1;
}
 
bool dialogue_is_active(void)
{
    return g_current_node >= 0;
}
 
void dialogue_ui_draw(int32_t x, int32_t y, int32_t width, int32_t height, const uint32_t* portrait)
{
    if (g_current_node < 0) return;
 
    DialogueNode* node = dialogue_get_node((uint32_t)g_current_node);
    if (!node) { g_current_node = -1; return; }
 
    int32_t choice_row_height = DIALOGUE_CHOICE_BUTTON_HEIGHT + DIALOGUE_CHOICE_SPACING;
    int32_t choice_area_height = (node->choice_count > 0)
        ? node->choice_count * choice_row_height
        : choice_row_height;
 
    int32_t text_height = height - choice_area_height;
 
    char text_buffer[DIALOGUE_TEXT_BUFFER_SIZE];
    const char* text = build_dialogue_text(node, text_buffer, sizeof(text_buffer));
    ui_dialogue_box(x, y, width, text_height, text, portrait);
 
    int32_t choice_y = y + text_height + DIALOGUE_CHOICE_SPACING;
 
    if (node->choice_count > 0)
    {
        for (uint16_t i = 0; i < node->choice_count; i++)
        {
            char choice_buffer[DIALOGUE_CHOICE_BUFFER_SIZE];
            const char* choice_text = resolve_cstr(node->choices[i].choice_id, choice_buffer, sizeof(choice_buffer));
 
            if (ui_button(x, choice_y, width, DIALOGUE_CHOICE_BUTTON_HEIGHT, choice_text, UI_COLOR_BLACK))
            {
                goto_node_or_end(node->choices[i].next_node_index);
                return; // node is about to change — stop drawing the rest of this frame's choices
            }
 
            choice_y += choice_row_height;
        }
    }
    else
    {
        // Not localized — this is a UI-level prompt, not authored dialogue
        // content, so it isn't going through locale_get_string.
        if (ui_button(x, choice_y, width, DIALOGUE_CHOICE_BUTTON_HEIGHT, "Continue", UI_COLOR_BLACK))
        {
            goto_node_or_end(node->next_node_index);
        }
    }
}