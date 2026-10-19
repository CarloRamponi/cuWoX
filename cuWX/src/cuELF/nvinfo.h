#pragma once

#include <cstdio>
#include <cstdint>
#include <vector>

enum EIFMT: uint8_t {
    EIFMT_NVAL = 1,
    EIFMT_BVAL = 2,
    EIFMT_HVAL = 3,
    EIFMT_SVAL = 4
};

enum EIATTR: uint16_t {
    EIATTR_CTAIDZ_USED = 0x0401,
    EIATTR_MAX_THREADS = 0x0504,
    EIATTR_PARAM_CBANK = 0x0a04,
    EIATTR_EXTERNS = 0x0f04,
    EIATTR_REQNTID = 0x1004,
    EIATTR_FRAME_SIZE = 0x1104,
    EIATTR_MIN_STACK_SIZE = 0x1204,
    EIATTR_BINDLESS_TEXTURE_BANK = 0x1502,
    EIATTR_BINDLESS_SURFACE_BANK = 0x1602,
    EIATTR_KPARAM_INFO = 0x1704,
    EIATTR_CBANK_PARAM_SIZE = 0x1903,
    EIATTR_MAXREG_COUNT = 0x1b03,
    EIATTR_EXIT_INSTR_OFFSETS = 0x1c04,
    EIATTR_S2RCTAID_INSTR_OFFSETS = 0x1d04,
    EIATTR_CRS_STACK_SIZE = 0x1e04,
    EIATTR_NEED_CNP_WRAPPER = 0x1f01,
    EIATTR_NEED_CNP_PATCH = 0x2001,
    EIATTR_EXPLICIT_CACHING = 0x2101,
    EIATTR_MAX_STACK_SIZE = 0x2304,
    EIATTR_LD_CACHEMOD_INSTR_OFFSETS = 0x2504,
    EIATTR_ATOM_SYS_INSTR_OFFSETS = 0x2704,
    EIATTR_COOP_GROUP_INSTR_OFFSETS = 0x2804,
    EIATTR_SW1850030_WAR = 0x2a01,
    EIATTR_WMMA_USED = 0x2b01,
    EIATTR_ATOM16_EMUL_INSTR_REG_MAP = 0x2e04,
    EIATTR_REGCOUNT = 0x2f04,
    EIATTR_SW2393858_WAR = 0x3001,
    EIATTR_INT_WARP_WIDE_INSTR_OFFSETS = 0x3104,
    EIATTR_INDIRECT_BRANCH_TARGETS = 0x3404,
    EIATTR_SW2861232_WAR = 0x3501,
    EIATTR_SW_WAR = 0x3604,
    EIATTR_CUDA_API_VERSION = 0x3704,
    EIATTR_SYSCALL_OFFSETS = 0x4604,
};

const char *eiattr_str(EIATTR attr);

class NVInfoAttribute {
public:
    NVInfoAttribute(const char *data);
    NVInfoAttribute(EIATTR attr, uint16_t i_value);
    NVInfoAttribute(EIATTR attr, uint32_t *s_value, uint16_t s_count);
    ~NVInfoAttribute();
    NVInfoAttribute(const NVInfoAttribute &other);
    NVInfoAttribute &operator=(const NVInfoAttribute &other);
    void encode(char *data);
    void dump();

    const char *name;
    uint16_t i_value;
    uint16_t s_count;
    uint32_t *s_value;
    size_t size;
    EIFMT fmt;
    EIATTR attr;
};

class NVInfo {
public:
    NVInfo(const char *data, size_t size);
    NVInfo(const NVInfo &other);
    NVInfo &operator=(const NVInfo &other);
    void encode(char *data);
    void dump();
    void add_attribute(const NVInfoAttribute &attr);

    size_t size;
    std::vector<NVInfoAttribute> attributes;
};