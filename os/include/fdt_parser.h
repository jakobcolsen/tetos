#ifndef FDT_PARSER_H
#define FDT_PARSER_H

#include <stdint.h>
#include <stddef.h>
#include <mini_lib.h>

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
    FDTAlias_t entries[32];
    size_t count;
} FDTAliasTable_t;

typedef struct {
    const char* raw;
    const char* abs_path;
    char abs_buffer[128];
} FDTStdOut_t;

typedef struct {
    unsigned int reg_address_cells;
    unsigned int reg_size_cells;

    // What the child should inherit.
    unsigned int child_address_cells;
    unsigned int child_size_cells;
} FDTAddressSizeFrame_t;

typedef struct {
    FDTAddressSizeFrame_t frames[32];
    size_t depth;
} FDTAddressSizeStack_t;

typedef struct {
    uint64_t base;
    uint64_t size;
} FDTRegRegion_t;

// Helper functions
uint32_t read_be32(const void* pointer);
uint64_t read_be64(const void* pointer);
int fdt_init(FDTView_t* fdt, const void* blob, size_t size);
int fdt_next(FDTCursor_t* cursor, FDTView_t* fdt, FDTToken_t* token, const char** name, FDTProp_t* prop);
int fdt_resolve_stdout_uart(const FDTView_t* fdt, uint64_t* base, uint64_t* size, const char** path, const char** compatible);
int fdt_read_timebase_frequency(const FDTView_t* fdt, uint32_t* hz, const char** src_path);

#endif // FDT_PARSER_H