#include "items/items_registry.h"

#include <stdlib.h>

Registry *g_items = NULL;

static Item   **s_table = NULL;
static uint32_t s_count = 0;

typedef struct {
	Item   **list;
	uint32_t count;
	uint32_t cap;
} BuildCtx;

static void build_walk(const char *full_id, void *entry, void *ctx)
{
	(void)full_id;
	BuildCtx *b = ctx;
	Item *item = entry;

	if (b->count == b->cap) {
		uint32_t nc = b->cap ? b->cap * 2 : 64;
		Item **n = realloc(b->list, nc * sizeof(*n));
		if (!n)
			return;
		b->list = n;
		b->cap  = nc;
	}

	/* dense ids are 1-based so 0 can mean invalid */
	item->dense_id = b->count + 1;
	b->list[b->count++] = item;
}

bool items_registry_init(void)
{
	if (g_items)
		return true;

	g_items = registry_create("item", item_registry_free);
	return g_items != NULL;
}

void items_registry_shutdown(void)
{
	free(s_table);
	s_table = NULL;
	s_count = 0;

	registry_destroy(g_items);
	g_items = NULL;
}

bool items_registry_build_dense_table(void)
{
	if (!g_items)
		return false;

	free(s_table);
	s_table = NULL;
	s_count = 0;

	BuildCtx ctx = {0};
	registry_foreach(g_items, build_walk, &ctx);

	s_table = ctx.list;
	s_count = ctx.count;
	return true;
}

Item *item_from_id(ItemId id)
{
	if (id == ITEM_ID_INVALID || id > s_count || !s_table)
		return NULL;
	return s_table[id - 1];
}

ItemId item_id_from_string(const char *full_id)
{
	Item *item = registry_get(g_items, full_id);
	return item ? item->dense_id : ITEM_ID_INVALID;
}

uint32_t items_registry_count(void)
{
	return s_count;
}