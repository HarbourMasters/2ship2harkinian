#ifndef ELF32_H
#define ELF32_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

enum {
    ELF_MACHINE_NONE = 0,
    ELF_MACHINE_MIPS = 8,
};

enum {
    ELF_TYPE_RELOC = 1,
    ELF_TYPE_EXEC,
    ELF_TYPE_SHARED,
    ELF_TYPE_CORE,
};

struct Elf32 {
    uint8_t endian;
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
    int symtabndx;
    int strtabndx;
    int numsymbols;

    const uint8_t* data;
    size_t dataSize;
    uint16_t (*read16)(const uint8_t*);
    uint32_t (*read32)(const uint8_t*);
};

enum {
    SHT_NULL = 0,
    SHT_PROGBITS,
    SHT_SYMTAB,
    SHT_STRTAB,
};

struct Elf32_Section {
    const char* name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t size;
    uint32_t offset;
    uint32_t addralign;
    uint32_t entsize;
};

#define SHN_UNDEF   0
#define SHN_ABS     0xFFF1
#define SHN_COMMON  0xFFF2

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6
#define STT_NUM     7

struct Elf32_Symbol {
    const char* name;
    uint32_t value;
    uint32_t size;
    uint8_t st_type;
    uint16_t shndx;
};

bool elf32_init(struct Elf32* e, const void* data, size_t size);
bool elf32_get_section(struct Elf32* e, struct Elf32_Section* sec, int secnum);
bool elf32_get_symbol(struct Elf32* e, struct Elf32_Symbol* sym, int symnum);

#endif
