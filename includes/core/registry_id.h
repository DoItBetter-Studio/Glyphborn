#ifndef REGISTRY_ID_H
#define REGISTRY_ID_H

#include <stdbool.h>

/** Build "type.namespace.name". Caller must free the result. */
char *registry_id_make_full(const char *type, const char *namespace_id, const char *name);

/** Build "namespace:name" shorthand. Caller must free. */
char *registry_id_make_shorthand(const char *namespace_id, const char *name);

/**
 * Parse "namespace:name".
 * On success writes into the out parameters (caller must free them)
 * and returns true.
 */
bool registry_id_parse_shorthand(const char *shorthand, char **out_namespace, char **out_name);

#endif // REGISTRY_ID_H