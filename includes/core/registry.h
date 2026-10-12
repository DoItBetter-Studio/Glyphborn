#ifndef REGISTRY_H
#define REGISTRY_H

#include <stddef.h>
#include <stdbool.h>

/* ------------------------------------------------------------------ */
/*  Forward declarations                                              */
/* ------------------------------------------------------------------ */
typedef struct Registry			Registry;
typedef struct DeferredRegister	DeferredRegister;

/* Function that frees an entry the registry owns. */
typedef void (*RegistryFreeFn)(void *entry);

/* Factory used by DeferredRegister. Must return a newly allocated
   object that the registry will take ownership of (or NULL on failure). */
typedef void *(*RegistryFactoryFn)(void *user_data);

/* Optional walk callback. */
typedef void (*RegistryWalkFn)(const char *full_id, void *entry, void *ctx);

/* ------------------------------------------------------------------ */
/*  Registry                                                          */
/* ------------------------------------------------------------------ */

/**
 * Create a new registry.
 * @param type_name  Short type name used in IDs ("item", "entity", …)
 * @param free_fn    Function used to free entries (may be NULL if you
 *                   never want the registry to free them)
 */
Registry *registry_create(const char *type_name, RegistryFreeFn free_fn);

/** Destroy the registry and free every owned entry. */
void registry_destroy(Registry *registry);

/**
 * Register an entry. The registry takes ownership of `entry`.
 * Returns true on success, false if the ID already exists or on OOM.
 */
bool registry_register(Registry *registry, const char *full_id, void *entry);

/**
 * Replace an existing entry (old entry is freed with free_fn).
 * If the ID does not exist it behaves like registry_register.
 */
bool registry_replace(Registry *registry, const char *full_id, void *entry);

/** Retrieve an entry (does not transfer ownership). Returns NULL if missing. */
void *registry_get(Registry *registry, const char *full_id);

bool registry_exists(Registry *registry, const char *full_id);

/** Iterate all entries. */
void registry_foreach(Registry *registry, RegistryWalkFn fn, void *ctx);

const char *registry_type_name(const Registry *registry);

/* ------------------------------------------------------------------ */
/*  DeferredRegister                                                  */
/* ------------------------------------------------------------------ */

/**
 * Create a deferred register bound to a target registry and a namespace.
 * The new DeferredRegister is automatically added to the global manager.
 */
DeferredRegister *deferred_register_create(Registry *target, const char *namespace_id);

/** Destroy a deferred register (does not affect already-finalized entries). */
void deferred_register_destroy(DeferredRegister *deferred_register);

/**
 * Queue a named entry for later registration.
 * Returns the full ID that will be used (caller must free it),
 * or NULL on error (duplicate name, already finalized, OOM, …).
 *
 * The factory is called only during deferred_manager_finalize_all().
 * Ownership of the object returned by the factory is transferred to
 * the target registry.
 */
char *deferred_register(DeferredRegister *deferred_register, const char *name, RegistryFactoryFn factory, void *user_data);

/* ------------------------------------------------------------------ */
/*  Global DeferredRegister manager                                   */
/* ------------------------------------------------------------------ */

/** Add a DeferredRegister to the global list (already done by create). */
void deferred_manager_add(DeferredRegister *deferred_register);

/**
 * Call FinalizeRegistration on every deferred register.
 * After this call the objects live in their target registries
 * and are owned by them.
 */
void deferred_manager_finalize_all(void);

/** Clear the manager lsit (does not destroy the DeferredRegister objects). */
void deferred_manager_clear(void);

#endif // REGISTRY_H