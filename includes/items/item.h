#ifndef ITEM_H
#define ITEM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Item Item;

/* ------------------------------------------------------------------ */
/*  Behaviour                                                         */
/* ------------------------------------------------------------------ */

typedef struct ItemVTable
{
	void (*destroy)(Item *self);
	void (*use)(Item *self, void *user);
	bool (*can_stack_with)(const Item *self, const Item *other);
} ItemVTable;

/* ------------------------------------------------------------------ */
/*  Kind                                                              */
/* ------------------------------------------------------------------ */

typedef enum
{
	ITEM_KIND_GENERIC = 0,
	ITEM_KIND_WEAPON,
	ITEM_KIND_CONSUMABLE,
	ITEM_KIND_TOOL,
	ITEM_KIND_ARMOR,
	ITEM_KIND_SCRIPT,
} ItemKind;

/* ------------------------------------------------------------------ */
/*  Base item – registry owns these                                   */
/* ------------------------------------------------------------------ */

struct Item
{
	const ItemVTable   *vtable;
	ItemKind			kind;

	char 			   *id;
	char			   *display_name;
	int					max_stack;
	int					weight;

	uint32_t			dense_id;
};

/* ------------------------------------------------------------------ */
/*  Helpers                                                           */
/* ------------------------------------------------------------------ */

static inline void item_destroy(Item *item) {
	if (item && item->vtable && item->vtable->destroy)
		item->vtable->destroy(item);
}

static inline void item_use(Item *item, void *user) {
	if (item && item->vtable && item->vtable->use)
		item->vtable->use(item, user);
}

bool item_can_stack_with(const Item *a, const Item *b);

/* Create a plain generic item. Caller (or registry) owns the result. */
Item *item_create_generic(const char *full_id, const char *display_name, int max_stack, int weight);

/* Pass to registry_create as the free function. */
void item_registry_free(void *entry);

#endif // ITEM_H