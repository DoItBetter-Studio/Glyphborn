#include "core/registry_id.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* Portable strdup – works on Linux, Windows, everywhere */
static char *damascus_strdup(const char *s)
{
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

char *registry_id_make_full(const char *type,
                            const char *namespace_id,
                            const char *name)
{
    if (!type || !namespace_id || !name) return NULL;
    size_t len = strlen(type) + 1 + strlen(namespace_id) + 1 + strlen(name) + 1;
    char *id = malloc(len);
    if (id)
        snprintf(id, len, "%s.%s.%s", type, namespace_id, name);
    return id;
}

char *registry_id_make_shorthand(const char *namespace_id, const char *name)
{
    if (!namespace_id || !name) return NULL;
    size_t len = strlen(namespace_id) + 1 + strlen(name) + 1;
    char *id = malloc(len);
    if (id)
        snprintf(id, len, "%s:%s", namespace_id, name);
    return id;
}

bool registry_id_parse_shorthand(const char *shorthand,
                                 char **out_namespace,
                                 char **out_name)
{
    if (!shorthand || !out_namespace || !out_name) return false;

    const char *colon = strchr(shorthand, ':');
    if (!colon || colon == shorthand || !*(colon + 1)) return false;

    size_t ns_len = (size_t)(colon - shorthand);
    *out_namespace = malloc(ns_len + 1);
    *out_name      = damascus_strdup(colon + 1);

    if (!*out_namespace || !*out_name) {
        free(*out_namespace);
        free(*out_name);
        *out_namespace = *out_name = NULL;
        return false;
    }

    memcpy(*out_namespace, shorthand, ns_len);
    (*out_namespace)[ns_len] = '\0';
    return true;
}