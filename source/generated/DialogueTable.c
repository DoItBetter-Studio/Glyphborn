// Auto-generated. DO NOT EDIT.

#include "generated/DialogueTable.h"

Blob g_DialogueTable[] = {
    { NULL, DIALOGUETABLE_DIALOGUE_SIZE },
};

const size_t g_DialogueTable_Count = 1;

void DialogueTable_init(void) {
    g_DialogueTable[0].data = platform_get_asset(DIALOGUETABLE_DIALOGUE_OFFSET);
}
