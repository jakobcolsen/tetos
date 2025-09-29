#include "include/fdt_parser.h"

// FDT uses big-endian while RISC-V uses little endian. 
// This fixes that.
uint32_t read_be32(const void* pointer) {
    const unsigned char* p = (const unsigned char*) pointer;
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
           | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

uint64_t read_be64(const void* pointer) {
    const unsigned char* p = (const unsigned char*) pointer;
    return ((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48)
           | ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32)
           | ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16)
           | ((uint64_t)p[6] << 8) | (uint64_t)p[7];
}

static inline const unsigned char* fdt_align4(const unsigned char* base, const unsigned char* pointer) {
    uintptr_t offset = (uintptr_t) (pointer - base);
    offset = (offset + 3u) & ~3u; // Align to next multiple of 4
    return (const unsigned char*) (base + offset);
}

// Check if given pointer / length is within the bounds of given blob (error checking, yay)
static int in_bounds(const FDTView_t* fdt, const void* pointer, size_t length) {
    const unsigned char* p = (const unsigned char*) pointer;
    if (p < fdt->base) return 0; // Pointer before start of blob
    if (p + length > fdt->base + fdt->size) return 0; // Pointer after end of blob
    if ((p - fdt->base) + length > fdt->size) return 0; // Overflow
    return 1;
}

static int strlen_bounded(const char* pointer, size_t max_length, size_t* out_length) {
    for (size_t i = 0; i < max_length; i++) {
        if (pointer[i] == '\0') {
            if (out_length) *out_length = i;
            return 1;
        }
    }
    return 0; // No NULL, bad
}

int fdt_init(FDTView_t* fdt, const void* blob, size_t size) {
    if (!fdt || !blob || size < sizeof(FDTHeader_t)) return -1; // Bad input
    fdt->base = (const unsigned char*) blob;
    fdt->size = size;

    // Raw header fields
    const FDTHeader_t* header = (const FDTHeader_t*) blob;
    if (read_be32(&header->magic) != 0xd00dfeed) return -2; // Not valid FDT

    fdt->totalsize = read_be32(&header->totalsize);
    fdt->off_dt_struct = read_be32(&header->off_dt_struct);
    fdt->off_dt_strings = read_be32(&header->off_dt_strings);
    fdt->off_mem_rsvmap = read_be32(&header->off_mem_rsvmap);
    fdt->size_dt_strings = read_be32(&header->size_dt_strings);
    fdt->size_dt_struct = read_be32(&header->size_dt_struct);
    if (fdt->totalsize > size) return -3; // Blob is smaller than expected

    // Compute pointers and check bounds
    fdt->struct_begin = fdt->base + fdt->off_dt_struct;
    fdt->struct_end = fdt->struct_begin + fdt->size_dt_struct;
    if (!in_bounds(fdt, fdt->struct_begin, fdt->size_dt_struct)) return -4; // Structure block out of bounds

    fdt->strings_begin = fdt->base + fdt->off_dt_strings;
    fdt->strings_end = fdt->strings_begin + fdt->size_dt_strings;
    if (!in_bounds(fdt, fdt->strings_begin, fdt->size_dt_strings)) return -5; // Strings block out of bounds

    fdt->memrsv_begin = fdt->base + fdt->off_mem_rsvmap;
    fdt->memrsv_end = fdt->struct_begin; // Memory reservation block ends where structure block begins
    if (!in_bounds(fdt, fdt->memrsv_begin, fdt->memrsv_end - fdt->memrsv_begin)) return -6; // Memory reservation block out of bounds

    return 0; // We win!
}

// FDT Prop functions
int fdt_prop_is(const FDTProp_t* prop, const char* name) {
    return (strcmp((const char*) prop->name, name) == 0);
}

int fdt_prop_next_string(const FDTProp_t* prop, const char** output, size_t* cursor) {
    if (!output || !cursor) return 0; // Bad input
    if (*cursor >= prop->length) return 0; // Out of bounds

    const char* start = (const char*) prop->value + *cursor;
    size_t string_length;
    if (!strlen_bounded(start, prop->length - *cursor, &string_length)) return 0; // No NULL found, bad

    *output = start;
    *cursor += string_length + 1; // Move cursor past this string and NULL terminator
    return 1; // Success
}

int fdt_prop_stringlist_contains(const FDTProp_t* prop, const char* string) {
    const char* current = NULL;
    size_t offset = 0;
    while (fdt_prop_next_string(prop, &current, &offset)) {
        if (strcmp(current, string) == 0) return 1; // Found it!
    }
    
    return 0; // Not found
}
int fdt_prop_read_u32(const FDTProp_t* prop, uint32_t* output, size_t index) {
    if (!output) return 0; // Bad input

    size_t offset = index * 4u;
    if (offset + 4u > prop->length) return 0; // Out of bounds
    *output = read_be32((const unsigned char*) prop->value + offset);

    return 1; // Success
}

int fdt_prop_read_u64(const FDTProp_t* prop, uint64_t* output, size_t index) {
    if (!output) return 0; // Bad input

    size_t offset = index * 8u;
    if (offset + 8u > prop->length) return 0; // Out of bounds
    *output = read_be64((const unsigned char*) prop->value + offset);

    return 1; // Success
}

int fdt_next(FDTCursor_t* cursor, FDTView_t* fdt, FDTToken_t* token, const char** name, FDTProp_t* prop) {
    if (!cursor || !fdt || !token) return -1; // Bad input
    if (cursor->current >= cursor->end) return -2; // End of structure block

    // Token
    uint32_t raw_token = read_be32(cursor->current);
    cursor->current += 4;

    switch (raw_token) {
        case FDT_BEGIN_NODE: {
            *token = FDT_BEGIN_NODE;
            const char* node_name = (const char*) cursor->current;
            size_t max_length = cursor->end - cursor->current;
            size_t name_length = 0;
            if (!strlen_bounded(node_name, max_length, &name_length)) return -3; // No NULL found, bad
            if (name) *name = node_name;
            cursor->current += name_length + 1; // Move past name and NULL terminator
            cursor->current = fdt_align4(fdt->base, cursor->current);
            return 0;
        }

        case FDT_END_NODE: {
            *token = FDT_END_NODE;
            if (name) *name = NULL;
            return 0;
        }

        case FDT_PROP: {
            *token = FDT_PROP;

            if ((size_t) (cursor->end - cursor->current) < 8u) return -4; // Not enough space for prop header
            uint32_t prop_length = read_be32(cursor->current);
            cursor->current += 4;
            uint32_t name_offset = read_be32(cursor->current);
            cursor->current += 4;

            if ((size_t) (cursor->end - cursor->current) < prop_length) return -5; // Not enough space for prop value

            if (prop) {
                prop->name = (const char*) (fdt->strings_begin + name_offset);
                prop->name_offset = name_offset;
                prop->value = (const void*) cursor->current;
                prop->length = prop_length;
            }

            cursor->current += prop_length;
            cursor->current = fdt_align4(fdt->base, cursor->current);
            if (name) *name = NULL; // No name for prop
            return 0;
        }
        
        case FDT_NOP: {
            *token = FDT_NOP;
            if (name) *name = NULL;
            return 0;
        }

        case FDT_END: {
            *token = FDT_END;
            if (name) *name = NULL;
            return 1; // End of structure block
        }

        default:
            return -6; // Unknown token
    }

    return 0; // Success
}

// FDT path functions
static void path_push(FDTPathStack_t* stack, const char* name, size_t length) {
    if (!stack || !name || length == 0 || stack->depth >= 32) return; // Bad input or stack full

    stack->paths[stack->depth].string = name;
    stack->paths[stack->depth].length = length;
    stack->depth++;
}

static void path_pop(FDTPathStack_t* stack) {
    if (!stack || stack->depth == 0) return; // Bad input or stack empty
    stack->depth--;
}

// Joins the current path stack into a single string, separated by '/'.
static const char* path_join(const FDTPathStack_t* stack, char* buffer, size_t buffer_size) {
    if (!stack || !buffer || buffer_size == 0) return NULL; // Bad input

    size_t position = 0;
    if (buffer_size == 0) return buffer; // No space for anything
    buffer[position++] = '/'; // Start with root

    for (int i = 0; i < stack->depth; i++) {
        if (i > 0) {
            if (position < buffer_size)  {
                buffer[position++] = '/'; // Separator
            }
        }
        
        for (int j = 0; j < stack->paths[i].length; j++) {
            if (position < buffer_size) {
                buffer[position++] = stack->paths[i].string[j];
            }
        }
    }

    if (position >= buffer_size) {
        position = buffer_size - 1; // Ensure space for NULL terminator
    }

    buffer[position] = '\0'; // Null-terminate
    return buffer;
}

// Compares the current path stack to a given path string (e.g. "/soc/uart@10000000").
static int path_equals_str(const FDTPathStack_t* stack, const char* path) {
    if (!stack || !path) return 0; // Bad input

    int comparison = 0;
    size_t index = 0;

    if (path[0] == '/') {
        index++; // Skip leading '/'
    }

    while (comparison < stack->depth) {
        size_t string_index = index;
        while (path[string_index] && path[string_index] != '/') {
            string_index++;
        }

        size_t segment_length = string_index - index;
        if (segment_length != stack->paths[comparison].length) return 0; // Length mismatch

        if (memcmp(stack->paths[comparison].string, &path[index], segment_length) != 0) {
            return 0; // Content mismatch
        }

        if (path[string_index] == '/') {
            string_index++; // Skip '/'
        }

        index = string_index;
        comparison++;
    }

    return path[index] == '\0'; // Ensure we've reached the end of the input path
}

// Log file - O(1) add
static void alias_add(FDTAliasTable_t* table, const char* key, const char* value) {
    if (!table || !key || !value || table->count >= 32) return; // Bad input or table full

    if (table->count < (int) sizeof(table->entries) / sizeof(table->entries[0])) {
        table->entries[table->count++] = (FDTAlias_t) {key, value}; // Add new struct entry
    }
}

// O(n) lookup - Improve later???
static const char* alias_lookup(const FDTAliasTable_t* table, const char* key) {
    if (!table || !key) return NULL; // Bad input

    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].key, key) == 0) {
            return table->entries[i].value;
        }
    }

    return NULL; // Not found
}

// Removes colon and everything after (e.g. "serial0:115200" -> "serial0")
static const char* stdout_trim(const char* string, char* output, size_t size) {
    if (!string || !output || size == 0) return NULL; // Bad input

    size_t i = 0;
    while (string[i] && string[i] != ':' && i + 1 < size) {
        output[i] = string[i];
        i++;
    }

    output[i] = '\0'; // Null-terminate
    return output;
}

// Find abs path from /chosen stdout
static void chosen_stdout(const FDTProp_t* prop, const FDTAliasTable_t* aliases, FDTStdOut_t* output) {
    if (!prop || !aliases || !output) return; // Bad input

    output->raw = (const char *) prop->value;
    char token[sizeof(output->abs_buffer)];

    stdout_trim(output->raw, token, sizeof(token));
    if (token[0] == '/') {
        size_t length = strlen(token);
        if (length >= sizeof(output->abs_buffer)) {
            length = sizeof(output->abs_buffer) - 1; // clamp for null terminator
        }

        memcpy(output->abs_buffer, token, length);
        output->abs_buffer[length] = '\0';   // manual null-termination
        output->abs_path = output->abs_buffer;
    } else {
        output->abs_path = alias_lookup(aliases, token); // Lookup alias
    }
}

static int path_equals_abs(const FDTPathStack_t* stack, const char* path) {
    if (!stack || !path) return 0; // Bad input

    char buffer[256];
    path_join(stack, buffer, sizeof(buffer));
    return (strcmp(buffer, path) == 0);
}

// Address size frame functions
static void asf_init_root(FDTAddressSizeStack_t* stack, unsigned int address_cells_root, unsigned int size_cells_root) {
    if (!stack) return; // Bad input

    stack->depth = 0;
    FDTAddressSizeFrame_t root = {
        .reg_address_cells = address_cells_root,
        .reg_size_cells = size_cells_root,
        .child_address_cells = address_cells_root,
        .child_size_cells = size_cells_root
    };

    stack->frames[stack->depth++] = root;
}

static void asf_push_child(FDTAddressSizeStack_t* stack) {
    if (!stack || stack->depth == 0 || stack->depth >= 32) return; // Bad input or stack full

    FDTAddressSizeFrame_t parent = stack->frames[stack->depth - 1];
    FDTAddressSizeFrame_t child = {
        .reg_address_cells = parent.child_address_cells,
        .reg_size_cells = parent.child_size_cells,
        .child_address_cells = parent.child_address_cells,
        .child_size_cells = parent.child_size_cells
    };

    stack->frames[stack->depth++] = child;
}

static void asf_pop(FDTAddressSizeStack_t* stack) {
    if (!stack || stack->depth == 0) return; // Bad input or stack empty
    stack->depth--;
}

static FDTAddressSizeFrame_t* asf_top(FDTAddressSizeStack_t* stack) {
    if (!stack || stack->depth == 0) return NULL; // Bad input or stack empty
    return &stack->frames[stack->depth - 1];
}

static uint64_t be_cells_to_u64(const uint8_t* pointer, size_t cell_count) {
    if (!pointer || cell_count == 0 || cell_count > 2) return 0; // Bad input

    uint64_t result = 0;
    for (int i = 0; i < cell_count; i++) {
        uint32_t cell = read_be32(pointer + (i * 4u));
        result = (result << 32) | (uint64_t) cell;
    }

    return result;
}

static int reg_decode_regions(const FDTProp_t* prop, int address_cells, int size_cells, FDTRegRegion_t* output, int max_regions) {
    if (!prop || !output || max_regions == 0 || address_cells <= 0 || size_cells <= 0) return -1; // Bad input
    if (!prop || prop->length == 0) return -2; // No prop

    const int stride = (address_cells + size_cells) * 4u;
    if (stride <= 0 || (prop->length % stride) != 0) return -3; // Invalid prop length

    int region_count = (int) (prop->length / stride);
    if (region_count > max_regions) {
        region_count = max_regions; // Limit to max regions
    }

    const uint8_t* pointer = (const uint8_t*) prop->value;
    for (int i = 0; i < region_count; i++) {
        output[i].base = be_cells_to_u64(pointer, address_cells);
        output[i].size = be_cells_to_u64(pointer + (address_cells * 4u), size_cells);
        pointer += stride;
    }

    return region_count; // Return number of regions decoded
}

// This will have to be generalized later for virtio and others
int fdt_resolve_stdout_uart(const FDTView_t* fdt, uint64_t* base, uint64_t* size, const char** path, const char** compatible)
{
    if (!fdt || !base || !size || !path || !compatible) return -1; // Bad input

    // Pass 1: collect /aliases and resolve /chosen stdout path
    FDTAliasTable_t aliases = { .count = 0 };
    FDTStdOut_t stdout_info = { .raw = NULL, .abs_path = NULL };

    {
        FDTCursor_t cursor = { .current = fdt->struct_begin, .end = fdt->struct_end };
        FDTPathStack_t path_stack = { .depth = 0 };
        FDTToken_t token;
        const char* name = NULL;
        FDTProp_t prop;

        while (1) {
            int node = fdt_next(&cursor, (FDTView_t*) fdt, &token, &name, &prop);
            if (node == 1) break;
            if (node < 0) return -2;

            switch (token) {
                case FDT_BEGIN_NODE:
                    path_push(&path_stack, name, strlen(name));
                    break;

                case FDT_END_NODE:
                    path_pop(&path_stack);
                    break;

                case FDT_PROP: {
                    // /aliases: store every key -> string value
                    if (path_stack.depth == 1 &&
                        path_stack.paths[0].length == 7 &&
                        memcmp(path_stack.paths[0].string, "aliases", 7) == 0)
                    {
                        const char* key = (const char*)prop.name;
                        const char* value = (const char*)prop.value;
                        size_t val_len;
                        if (strlen_bounded(value, prop.length, &val_len)) {
                            alias_add(&aliases, key, value);
                        }
                    }

                    // /chosen: capture stdout-path (preferred) or stdout
                    if (path_stack.depth == 1 &&
                        path_stack.paths[0].length == 6 &&
                        memcmp(path_stack.paths[0].string, "chosen", 6) == 0)
                    {
                        if (fdt_prop_is(&prop, "stdout-path") || fdt_prop_is(&prop, "stdout")) {
                            chosen_stdout(&prop, &aliases, &stdout_info);
                        }
                    }
                } break;

                default: break;
            }
        }
    }

    if (!stdout_info.abs_path) return -3; // couldn't resolve /chosen stdout path

    // -------- Pass 2: walk to the stdout node, track addr/size cells, read reg --------
    {
        FDTCursor_t cursor = { .current = fdt->struct_begin, .end = fdt->struct_end };
        FDTPathStack_t path_stack = { .depth = 0 };
        FDTAddressSizeStack_t address_stack;
        // Reasonable defaults if root omits them (common on some blobs)
        asf_init_root(&address_stack, /*address_cells_root=*/2, /*size_cells_root=*/2);

        FDTToken_t token;
        const char* name = NULL;
        FDTProp_t prop;

        uint64_t found_base = 0, found_size = 0;
        const char* found_compatible = NULL;
        int have_region = 0;

        while (1) {
            int node = fdt_next(&cursor, (FDTView_t*)fdt, &token, &name, &prop);
            if (node == 1) break;
            if (node < 0) return -4;

            switch (token) {
                case FDT_BEGIN_NODE:
                    path_push(&path_stack, name, strlen(name));
                    asf_push_child(&address_stack);
                    break;

                case FDT_END_NODE:
                    asf_pop(&address_stack);
                    path_pop(&path_stack);
                    break;

                case FDT_PROP: {
                    FDTAddressSizeFrame_t* address_frame = asf_top(&address_stack);
                    if (!address_frame) return -5;

                    // Update child address/size cells for this subtree
                    if (fdt_prop_is(&prop, "#address-cells")) {
                        uint32_t v;
                        if (fdt_prop_read_u32(&prop, &v, 0)) address_frame->child_address_cells = v;
                    } else if (fdt_prop_is(&prop, "#size-cells")) {
                        uint32_t v;
                        if (fdt_prop_read_u32(&prop, &v, 0)) address_frame->child_size_cells = v;
                    }

                    // Are we at the resolved stdout node?
                    if (path_equals_abs(&path_stack, stdout_info.abs_path)) {
                        if (fdt_prop_is(&prop, "compatible") && !found_compatible) {
                            // Use the start of the stringlist
                            found_compatible = (const char*)prop.value;
                        } else if (fdt_prop_is(&prop, "reg") && !have_region) {
                            FDTRegRegion_t region;
                            int n = reg_decode_regions(&prop,
                                                       (int)address_frame->reg_address_cells,
                                                       (int)address_frame->reg_size_cells,
                                                       &region, 1);
                            if (n > 0) {
                                found_base = region.base;
                                found_size = region.size;
                                have_region = 1;
                            }
                        }
                    }
                } break;

                default: break;
            }
        }

        if (!have_region) return -6;

        *base = found_base;
        *size = found_size;
        *path = stdout_info.abs_path;         // absolute path like "/soc/uart@10000000"
        *compatible = found_compatible ? found_compatible : ""; // may be a stringlist; first is fine
        return 0;
    }
}