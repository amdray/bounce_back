/**
 * Resource Loader for Bounce Back
 * 
 * Loads binary resource containers in the format used by J2ME class c.java
 * 
 * Container format:
 *   [2 bytes] count (big-endian uint16)
 *   [count * 2 bytes] element sizes (big-endian uint16 each)
 *   [variable] element data (concatenated)
 * 
 * Reference: DEOBFUSCATION.md, c.java lines 1-86
 */

#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include <stdint.h>
#include <stddef.h>

/**
 * Resource container structure.
 * Holds the entire file in memory for fast random access.
 */
typedef struct {
    uint8_t*  data;       // Raw file data (owned by this structure)
    size_t    data_size;  // Total size of file
    uint16_t  count;      // Number of elements
    uint32_t* offsets;    // Offset of each element (count items, owned)
    uint16_t* sizes;      // Size of each element (count items, owned)
} ResourceContainer;

/**
 * Load a resource container from file.
 * 
 * @param path Path to the resource file (e.g., "res/b")
 * @return Pointer to ResourceContainer, or NULL on error
 * 
 * Caller must free with resource_free()
 */
ResourceContainer* resource_load(const char* path);

/**
 * Get a specific element from the container.
 * 
 * @param rc Resource container
 * @param index Element index (0-based)
 * @param out_size [out] Size of the returned data (can be NULL)
 * @return Pointer to element data (valid until resource_free()), or NULL if index invalid
 * 
 * The returned pointer points into rc->data, DO NOT free it separately.
 */
const uint8_t* resource_get_element(const ResourceContainer* rc, int index, size_t* out_size);

/**
 * Free a resource container and all associated memory.
 * 
 * @param rc Resource container to free (can be NULL)
 */
void resource_free(ResourceContainer* rc);

#endif // RESOURCE_LOADER_H
