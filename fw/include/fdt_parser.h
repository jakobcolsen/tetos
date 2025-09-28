#include <stdint.h>
#include <stddef.h>

#ifndef FDT_PARSER_H
#define FDT_PARSER_H

typedef struct {
    uint32_t magic; // 0xd00dfeed
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
} FDTHeader_t;

typedef struct {
    const unsigned char* base;     // start of the FDT blob
    size_t size;                   // size of the FDT blob

    // raw header fields (host-endian)
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;

    // computed pointers (bounds-checked)
    const unsigned char* struct_begin;
    const unsigned char* struct_end;
    const unsigned char* strings_begin;
    const unsigned char* strings_end;
    const unsigned char* memrsv_begin;
    const unsigned char* memrsv_end;
} FDTView_t;

typedef struct {
    uint64_t address;
    uint64_t size;
} FDTMemReserveEntry_t;

typedef enum {
    FDT_BEGIN_NODE = 1,
    FDT_END_NODE = 2,
    FDT_PROP = 3,
    FDT_NOP = 4,
    FDT_END = 9
} FDTToken_t;

typedef struct {
    const unsigned char* current; // Current position in structure block
    const unsigned char* end;     // End of structure block
} FDTCursor_t;

typedef struct {
    const unsigned char* name;    // Property name (string)
    const unsigned char* value;   // Property value (blob)
    size_t length;            // Size of property value
    uint32_t name_offset;        // Offset into strings block for name
} FDTProp_t;

typedef struct {
    const char* string; 
    size_t length;
} FDTPath_t;

typedef struct {
    FDTPath_t paths[32];
    size_t depth;
} FDTPathStack_t;

typedef struct {
    const char* key;
    const char* value;
} FDTAlias_t;

typedef struct {
    FDTAlias_t* entries[32];
    size_t count;
} FDTAliasTable_t;



// Helper functions
static inline uint32_t read_be32(const void* pointer);
static inline uint64_t read_be64(const void* pointer);
static inline const unsigned char* fdt_align4(const unsigned char* base, const unsigned char* pointer);
static int in_bounds(const FDTView_t* fdt, const void* pointer, size_t length);
static int strlen_bounded(const char* pointer, size_t max_length, size_t* out_length);
int fdt_init(FDTView_t* fdt, const void* blob, size_t size);
int fdt_next(FDTCursor_t* cursor, FDTView_t* fdt, FDTToken_t* token, const char** name, FDTProp_t* prop);

// FDT path
static void path_push(FDTPathStack_t* stack, const char* name, size_t length);
static void path_pop(FDTPathStack_t* stack);
static void path_join(const FDTPathStack_t* stack, char* buffer, size_t buffer_size);
static int path_equals_str(const FDTPathStack_t* stack, const char* path);

static void alias_add(FDTAliasTable_t* table, const char* key, const char* value);
static const char* alias_lookup(const FDTAliasTable_t* table, const char* key);

// Mini_lib because I'm trying to avoid libc
extern int strcmp(const char*, const char*);
extern size_t strlen(const char*);
extern int memcmp(const void*, const void*, size_t);

// FDT Prop helpers
int fdt_prop_is(const FDTProp_t* prop, const char* name);
int fdt_prop_next_string(const FDTProp_t* prop, const char** output, size_t* cursor);
int fdt_prop_stringlist_contains(const FDTProp_t* prop, const char* string);
int fdt_prop_read_u32(const FDTProp_t* prop, uint32_t* output, size_t index);
int fdt_prop_read_u64(const FDTProp_t* prop, uint64_t* output, size_t index);

#endif // FDT_PARSER_H