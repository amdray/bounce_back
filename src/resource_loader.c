/**
 * Resource Loader implementation
 * Based on J2ME class c.java format
 */

#include "resource_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Read big-endian uint16 from buffer.
 */
static inline uint16_t read_be16(const uint8_t* buf) {
    return ((uint16_t)buf[0] << 8) | (uint16_t)buf[1];
}

ResourceContainer* resource_load(const char* path) {
    if (!path) {
        fprintf(stderr, "resource_load: NULL path\n");
        return NULL;
    }

    // Open file
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "resource_load: failed to open '%s'\n", path);
        return NULL;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size < 2) {
        fprintf(stderr, "resource_load: file too small (%ld bytes)\n", file_size);
        fclose(f);
        return NULL;
    }

    // Allocate and read entire file
    uint8_t* data = (uint8_t*)malloc(file_size);
    if (!data) {
        fprintf(stderr, "resource_load: malloc failed for %ld bytes\n", file_size);
        fclose(f);
        return NULL;
    }

    size_t read_count = fread(data, 1, file_size, f);
    fclose(f);

    if (read_count != (size_t)file_size) {
        fprintf(stderr, "resource_load: read %zu bytes, expected %ld\n", read_count, file_size);
        free(data);
        return NULL;
    }

    // Parse header: count (big-endian uint16)
    uint16_t count = read_be16(data);

    size_t header_size = 2 + count * 2;  // 2 bytes count + count * 2 bytes sizes
    if (file_size < (long)header_size) {
        fprintf(stderr, "resource_load: file too small for header (count=%u, need %zu bytes)\n", 
                count, header_size);
        free(data);
        return NULL;
    }

    // Allocate container structure
    ResourceContainer* rc = (ResourceContainer*)malloc(sizeof(ResourceContainer));
    if (!rc) {
        fprintf(stderr, "resource_load: malloc failed for ResourceContainer\n");
        free(data);
        return NULL;
    }

    rc->data = data;
    rc->data_size = file_size;
    rc->count = count;

    // Allocate offsets and sizes arrays
    rc->offsets = (uint32_t*)malloc(count * sizeof(uint32_t));
    rc->sizes = (uint16_t*)malloc(count * sizeof(uint16_t));

    if (!rc->offsets || !rc->sizes) {
        fprintf(stderr, "resource_load: malloc failed for offsets/sizes\n");
        free(rc->offsets);
        free(rc->sizes);
        free(rc->data);
        free(rc);
        return NULL;
    }

    // Parse sizes and calculate offsets
    uint32_t offset = header_size;
    for (int i = 0; i < count; i++) {
        uint16_t size = read_be16(data + 2 + i * 2);
        rc->sizes[i] = size;
        rc->offsets[i] = offset;
        offset += size;
    }

    // Validate total size
    if (offset != (uint32_t)file_size) {
        fprintf(stderr, "resource_load: WARNING: calculated size %lu != file size %ld\n", 
                (unsigned long)offset, file_size);
    }

    printf("resource_load: loaded '%s' - %u elements, %ld bytes\n", path, count, file_size);

    return rc;
}

const uint8_t* resource_get_element(const ResourceContainer* rc, int index, size_t* out_size) {
    if (!rc) {
        fprintf(stderr, "resource_get_element: NULL container\n");
        return NULL;
    }

    if (index < 0 || index >= rc->count) {
        fprintf(stderr, "resource_get_element: index %d out of range [0..%u)\n", 
                index, rc->count);
        return NULL;
    }

    if (out_size) {
        *out_size = rc->sizes[index];
    }

    return rc->data + rc->offsets[index];
}

void resource_free(ResourceContainer* rc) {
    if (!rc) return;

    free(rc->offsets);
    free(rc->sizes);
    free(rc->data);
    free(rc);
}
