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

static inline uint32_t read_be32(const void* pointer);
static inline uint64_t read_be64(const void* pointer);
static int in_bounds(const FDTView_t* fdt, const void* pointer, size_t length);
static int fdt_init(FDTView_t* fdt, const void* blob, size_t size);

#endif // FDT_PARSER_H