#include "core/registry.h"
#include "core/registry_id.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Portable strdup – works on Linux, Windows, everywhere */
static char *damascus_strdup(const char *s)
{
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* ------------------------------------------------------------------ */
/*  Simple open-addressing hash table                                  */
/* ------------------------------------------------------------------ */

#define REG_INITIAL_CAP 32

typedef struct {
    char *key;          /* owned full_id */
    void *value;        /* owned by the registry */
    bool  occupied;
} RegSlot;

struct Registry {
    char          *type_name;
    RegistryFreeFn free_fn;
    RegSlot       *slots;
    size_t         capacity;
    size_t         count;
};

static unsigned hash_str(const char *s)
{
    unsigned h = 5381;
    while (*s)
        h = ((h << 5) + h) + (unsigned char)*s++;
    return h;
}

static RegSlot *find_slot(Registry *reg, const char *key, bool for_insert)
{
    if (!reg->capacity) return NULL;
    unsigned h = hash_str(key) % reg->capacity;
    for (size_t i = 0; i < reg->capacity; i++) {
        size_t idx = (h + i) % reg->capacity;
        RegSlot *s = &reg->slots[idx];
        if (!s->occupied)
            return for_insert ? s : NULL;
        if (strcmp(s->key, key) == 0)
            return s;
    }
    return NULL;
}

static bool registry_grow(Registry *reg)
{
    size_t old_cap = reg->capacity;
    RegSlot *old   = reg->slots;

    size_t new_cap = old_cap ? old_cap * 2 : REG_INITIAL_CAP;
    RegSlot *neu   = calloc(new_cap, sizeof(RegSlot));
    if (!neu) return false;

    reg->slots    = neu;
    reg->capacity = new_cap;
    reg->count    = 0;

    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].occupied) {
            RegSlot *s = find_slot(reg, old[i].key, true);
            *s = old[i];
            reg->count++;
        }
    }
    free(old);
    return true;
}

/* ------------------------------------------------------------------ */
/*  Registry public API                                                */
/* ------------------------------------------------------------------ */

Registry *registry_create(const char *type_name, RegistryFreeFn free_fn)
{
    if (!type_name) return NULL;

    Registry *r = calloc(1, sizeof(*r));
    if (!r) return NULL;

    r->type_name = damascus_strdup(type_name);
    r->free_fn   = free_fn;
    r->capacity  = REG_INITIAL_CAP;
    r->slots     = calloc(r->capacity, sizeof(RegSlot));

    if (!r->type_name || !r->slots) {
        registry_destroy(r);
        return NULL;
    }
    return r;
}

void registry_destroy(Registry *reg)
{
    if (!reg) return;

    for (size_t i = 0; i < reg->capacity; i++) {
        if (reg->slots[i].occupied) {
            free(reg->slots[i].key);
            if (reg->free_fn && reg->slots[i].value)
                reg->free_fn(reg->slots[i].value);
        }
    }
    free(reg->slots);
    free(reg->type_name);
    free(reg);
}

bool registry_register(Registry *reg, const char *full_id, void *entry)
{
    if (!reg || !full_id) return false;
    if (registry_exists(reg, full_id)) return false;

    if (reg->count * 4 >= reg->capacity * 3) {
        if (!registry_grow(reg)) return false;
    }

    RegSlot *s = find_slot(reg, full_id, true);
    if (!s) return false;

    s->key      = damascus_strdup(full_id);
    s->value    = entry;
    s->occupied = true;
    reg->count++;
    return s->key != NULL;
}

bool registry_replace(Registry *reg, const char *full_id, void *entry)
{
    if (!reg || !full_id) return false;

    RegSlot *s = find_slot(reg, full_id, false);
    if (s) {
        if (reg->free_fn && s->value)
            reg->free_fn(s->value);
        s->value = entry;
        return true;
    }
    return registry_register(reg, full_id, entry);
}

void *registry_get(Registry *reg, const char *full_id)
{
    if (!reg || !full_id) return NULL;
    RegSlot *s = find_slot(reg, full_id, false);
    return s ? s->value : NULL;
}

bool registry_exists(Registry *reg, const char *full_id)
{
    return registry_get(reg, full_id) != NULL;
}

void registry_foreach(Registry *reg, RegistryWalkFn fn, void *ctx)
{
    if (!reg || !fn) return;
    for (size_t i = 0; i < reg->capacity; i++)
        if (reg->slots[i].occupied)
            fn(reg->slots[i].key, reg->slots[i].value, ctx);
}

const char *registry_type_name(const Registry *reg)
{
    return reg ? reg->type_name : NULL;
}

/* ------------------------------------------------------------------ */
/*  DeferredRegister                                                   */
/* ------------------------------------------------------------------ */

typedef struct PendingEntry {
    char              *name;
    RegistryFactoryFn  factory;
    void              *user_data;
    struct PendingEntry *next;
} PendingEntry;

struct DeferredRegister {
    Registry     *target;
    char         *namespace_id;
    PendingEntry *pending;
    bool          finalized;
};

DeferredRegister *deferred_register_create(Registry *target,
                                           const char *namespace_id)
{
    if (!target || !namespace_id) return NULL;

    DeferredRegister *dr = calloc(1, sizeof(*dr));
    if (!dr) return NULL;

    dr->target       = target;
    dr->namespace_id = damascus_strdup(namespace_id);
    if (!dr->namespace_id) {
        free(dr);
        return NULL;
    }

    deferred_manager_add(dr);
    return dr;
}

void deferred_register_destroy(DeferredRegister *dr)
{
    if (!dr) return;

    for (PendingEntry *p = dr->pending; p; ) {
        PendingEntry *n = p->next;
        free(p->name);
        free(p);
        p = n;
    }
    free(dr->namespace_id);
    free(dr);
}

char *deferred_register(DeferredRegister *dr,
                        const char *name,
                        RegistryFactoryFn factory,
                        void *user_data)
{
    if (!dr || dr->finalized || !name || !factory)
        return NULL;

    for (PendingEntry *p = dr->pending; p; p = p->next)
        if (strcmp(p->name, name) == 0)
            return NULL;

    PendingEntry *e = calloc(1, sizeof(*e));
    if (!e) return NULL;

    e->name      = damascus_strdup(name);
    e->factory   = factory;
    e->user_data = user_data;
    if (!e->name) {
        free(e);
        return NULL;
    }

    e->next     = dr->pending;
    dr->pending = e;

    return registry_id_make_full(dr->target->type_name,
                                 dr->namespace_id,
                                 name);
}

/* ------------------------------------------------------------------ */
/*  Global manager                                                     */
/* ------------------------------------------------------------------ */

static DeferredRegister **g_registers = NULL;
static size_t             g_count     = 0;
static size_t             g_capacity  = 0;

void deferred_manager_add(DeferredRegister *dr)
{
    if (!dr) return;
    if (g_count == g_capacity) {
        size_t nc = g_capacity ? g_capacity * 2 : 8;
        DeferredRegister **nr = realloc(g_registers, nc * sizeof(*nr));
        if (!nr) return;
        g_registers = nr;
        g_capacity  = nc;
    }
    g_registers[g_count++] = dr;
}

void deferred_manager_finalize_all(void)
{
    for (size_t i = 0; i < g_count; i++) {
        DeferredRegister *dr = g_registers[i];
        if (!dr || dr->finalized) continue;

        for (PendingEntry *p = dr->pending; p; p = p->next) {
            char *full = registry_id_make_full(dr->target->type_name,
                                               dr->namespace_id,
                                               p->name);
            if (!full) continue;

            void *obj = p->factory(p->user_data);
            if (obj)
                registry_register(dr->target, full, obj);

            free(full);
        }
        dr->finalized = true;
    }
}

void deferred_manager_clear(void)
{
    free(g_registers);
    g_registers = NULL;
    g_count = g_capacity = 0;
}