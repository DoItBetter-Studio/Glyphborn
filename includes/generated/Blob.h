#ifndef BLOB_H
#define BLOB_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    const uint8_t* data;
    size_t size;
} Blob;

#endif
