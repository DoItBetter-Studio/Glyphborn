#include "items/item.h"

#include <stdlib.h>
#include <string.h>

static char *damascus_strdup(const char *s) {
	if (!s) return NULL;
	size_t n = strlen(s) + 1;
	char *p = malloc(n);
	if (p) memcpy(p, s, n);
	return p;
}

static void generic_item_destroy(Item *self)
{
	free(self->id);
	free(self->display_name);
	free(self);
}

static void generic_item_use(Item *self, void *user)
{
	(void)self;
	(void)user;
}

static const ItemVTable generic_vtable = {
	.destroy		= generic_item_destroy,
	.use			= generic_item_use,
	.can_stack_with = NULL,
};

bool item_can_stack_with(const Item *a, const Item *b)
{
	if (!a || !b || a->kind != b->kind)
		return false;
	if (a->vtable && a->vtable->can_stack_with)
		return a->vtable->can_stack_with(a, b);
	if (a->max_stack <= 1)
		return false;
	if (!a->id || !b->id)
		return false;
	return strcmp(a->id, b->id) == 0;
}

Item *item_create_generic(const char *full_id, const char *display_name,
                          int max_stack, int weight)
{
	if (!full_id)
		return NULL;

	Item *item = calloc(1, sizeof(*item));
	if (!item)
		return NULL;

	item->vtable       = &generic_vtable;
	item->kind         = ITEM_KIND_GENERIC;
	item->id           = damascus_strdup(full_id);
	item->display_name = damascus_strdup(display_name ? display_name : full_id);
	item->max_stack    = max_stack > 0 ? max_stack : 1;
	item->weight       = weight;
	item->dense_id     = 0;

	if (!item->id || !item->display_name) {
		item_destroy(item);
		return NULL;
	}
	return item;
}

void item_registry_free(void *entry)
{
	item_destroy((Item *)entry);
}