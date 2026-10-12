#ifndef ITEMS_REGISTRY_H
#define ITEMS_REGISTRY_H

#include "core/registry.h"
#include "items/item.h"
#include <stdint.h>
#include <stdbool.h>

/* String-keyed registry (mods + bootstrap). */
extern Registry *g_items;

/* Dense table built after deferred_manager_finalize_all().
   Hot path: item_from_id(id) – no strings. */
typedef uint32_t ItemId;

#define ITEM_ID_INVALID 0

bool items_registry_init(void);
void items_registry_shutdown(void);

/* Call once after all deferred item registrations have been finalized. */
bool items_registry_build_dense_table(void);

Item   *item_from_id(ItemId id);
ItemId  item_id_from_string(const char *full_id);
uint32_t items_registry_count(void);

#endif /* ITEMS_REGISTRY_H */