#include <nvinfo.h>
#include <stdexcept>

NVInfoAttribute::NVInfoAttribute(const char *data): s_value(nullptr) {
    this->attr = *(EIATTR *)data;
    this->fmt = EIFMT(this->attr & 0xff);
    data += 2;

    switch (this->fmt) {
        case EIFMT_NVAL:
            this->i_value = *(uint16_t *)data;
            this->size = 2 + 2;
            break;
        case EIFMT_BVAL:
            this->i_value = *(uint8_t *)data;
            this->size = 2 + 1;
            break;
        case EIFMT_HVAL:
            this->i_value = *(uint16_t *)data;
            this->size = 2 + 2;
            break;
        case EIFMT_SVAL:
            this->s_count = (*(uint16_t *)data) >> 2;
            data += 2;
            this->size = 2 + 2 + (this->s_count << 2);
            this->s_value = new uint32_t[this->s_count];
            for (uint16_t i = 0; i < this->s_count; i++) {
                this->s_value[i] = ((uint32_t *)data)[i];
            }
            break;
    }

    this->name = eiattr_str(this->attr);
}

NVInfoAttribute::~NVInfoAttribute() {
    if(this->s_value) {
        delete[] this->s_value;
    }
}

void NVInfoAttribute::encode(char *data) {
    *(EIATTR *)data = this->attr;
    data += 2;

    switch (this->fmt) {
        case EIFMT_NVAL:
            *(uint16_t *)data = this->i_value;
            break;
        case EIFMT_BVAL:
            *(uint8_t *)data = this->i_value;
            break;
        case EIFMT_HVAL:
            *(uint16_t *)data = this->i_value;
            break;
        case EIFMT_SVAL:
            *(uint16_t *)data = this->s_count << 2;
            data += 2;
            for (uint16_t i = 0; i < this->s_count; i++) {
                ((uint32_t *)data)[i] = this->s_value[i];
            }
            break;
    }
}

void NVInfoAttribute::dump() {
    printf("%s:", this->name);
    if (this->fmt == EIFMT_SVAL) {
        printf("\n");
        for(int i = 0; i < this->s_count; i++) {
            printf("\t0x%08x\n", this->s_value[i]);
        }
    } else {
        printf("\t\t0x%08x\n", this->i_value);
    }
}

NVInfoAttribute::NVInfoAttribute(const NVInfoAttribute &other) 
    : attr(other.attr), fmt(other.fmt), i_value(other.i_value), size(other.size), s_count(other.s_count), name(other.name) {
    if (other.s_value) {
        this->s_value = new uint32_t[other.s_count];
        for (uint16_t i = 0; i < other.s_count; i++) {
            this->s_value[i] = other.s_value[i];
        }
    } else {
        this->s_value = nullptr;
    }
}

NVInfoAttribute &NVInfoAttribute::operator=(const NVInfoAttribute &other) {
    if (this == &other) {
        return *this; // Handle self-assignment
    }

    // Clean up existing resources
    if (this->s_value) {
        delete[] this->s_value;
    }

    // Copy data from the other object
    this->attr = other.attr;
    this->fmt = other.fmt;
    this->i_value = other.i_value;
    this->size = other.size;
    this->s_count = other.s_count;
    this->name = other.name;

    if (other.s_value) {
        this->s_value = new uint32_t[other.s_count];
        for (uint16_t i = 0; i < other.s_count; i++) {
            this->s_value[i] = other.s_value[i];
        }
    } else {
        this->s_value = nullptr;
    }

    return *this;
}

const char *eiattr_str(EIATTR attr) {
    switch (attr) {
        case EIATTR_CTAIDZ_USED:
            return "EIATTR_CTAIDZ_USED";
            break;
        case EIATTR_MAX_THREADS:
            return "EIATTR_MAX_THREADS";
            break;
        case EIATTR_PARAM_CBANK:
            return "EIATTR_PARAM_CBANK";
            break;
        case EIATTR_EXTERNS:
            return "EIATTR_EXTERNS";
            break;
        case EIATTR_REQNTID:
            return "EIATTR_REQNTID";
            break;
        case EIATTR_FRAME_SIZE:
            return "EIATTR_FRAME_SIZE";
            break;
        case EIATTR_MIN_STACK_SIZE:
            return "EIATTR_MIN_STACK_SIZE";
            break;
        case EIATTR_BINDLESS_TEXTURE_BANK:
            return "EIATTR_BINDLESS_TEXTURE_BANK";
            break;
        case EIATTR_BINDLESS_SURFACE_BANK:
            return "EIATTR_BINDLESS_SURFACE_BANK";
            break;
        case EIATTR_KPARAM_INFO:
            return "EIATTR_KPARAM_INFO";
            break;
        case EIATTR_CBANK_PARAM_SIZE:
            return "EIATTR_CBANK_PARAM_SIZE";
            break;
        case EIATTR_MAXREG_COUNT:
            return "EIATTR_MAXREG_COUNT";
            break;
        case EIATTR_EXIT_INSTR_OFFSETS:
            return "EIATTR_EXIT_INSTR_OFFSETS";
            break;
        case EIATTR_S2RCTAID_INSTR_OFFSETS:
            return "EIATTR_S2RCTAID_INSTR_OFFSETS";
            break;
        case EIATTR_CRS_STACK_SIZE:
            return "EIATTR_CRS_STACK_SIZE";
            break;
        case EIATTR_NEED_CNP_WRAPPER:
            return "EIATTR_NEED_CNP_WRAPPER";
            break;
        case EIATTR_NEED_CNP_PATCH:
            return "EIATTR_NEED_CNP_PATCH";
            break;
        case EIATTR_EXPLICIT_CACHING:
            return "EIATTR_EXPLICIT_CACHING";
            break;
        case EIATTR_MAX_STACK_SIZE:
            return "EIATTR_MAX_STACK_SIZE";
            break;
        case EIATTR_LD_CACHEMOD_INSTR_OFFSETS:
            return "EIATTR_LD_CACHEMOD_INSTR_OFFSETS";
            break;
        case EIATTR_ATOM_SYS_INSTR_OFFSETS:
            return "EIATTR_ATOM_SYS_INSTR_OFFSETS";
            break;
        case EIATTR_COOP_GROUP_INSTR_OFFSETS:
            return "EIATTR_COOP_GROUP_INSTR_OFFSETS";
            break;
        case EIATTR_SW1850030_WAR:
            return "EIATTR_SW1850030_WAR";
            break;
        case EIATTR_WMMA_USED:
            return "EIATTR_WMMA_USED";
            break;
        case EIATTR_ATOM16_EMUL_INSTR_REG_MAP:
            return "EIATTR_ATOM16_EMUL_INSTR_REG_MAP";
            break;
        case EIATTR_REGCOUNT:
            return "EIATTR_REGCOUNT";
            break;
        case EIATTR_SW2393858_WAR:
            return "EIATTR_SW2393858_WAR";
            break;
        case EIATTR_INT_WARP_WIDE_INSTR_OFFSETS:
            return "EIATTR_INT_WARP_WIDE_INSTR_OFFSETS";
            break;
        case EIATTR_INDIRECT_BRANCH_TARGETS:
            return "EIATTR_INDIRECT_BRANCH_TARGETS";
            break;
        case EIATTR_SW2861232_WAR:
            return "EIATTR_SW2861232_WAR";
            break;
        case EIATTR_SW_WAR:
            return "EIATTR_SW_WAR";
            break;
        case EIATTR_CUDA_API_VERSION:
            return "EIATTR_CUDA_API_VERSION";
            break;
        case EIATTR_SYSCALL_OFFSETS:
            return "EIATTR_SYSCALL_OFFSETS";
            break;
        default:
            return "UNKNOWN_ATTRIBUTE";
            break;
    }
}

NVInfo::NVInfo(const char *data, size_t size): size(0) {
    char *end = (char *)data + size;
    while (data < end) {
        NVInfoAttribute attr(data);
        this->attributes.push_back(attr);
        this->size += attr.size;
        data += attr.size;
    }
}

NVInfo::NVInfo(const NVInfo &other): attributes(other.attributes), size(other.size) {
}

NVInfo &NVInfo::operator=(const NVInfo &other) {
    if (this == &other) {
        return *this; // Handle self-assignment
    }

    // Copy data from the other object
    this->attributes = other.attributes;
    this->size = other.size;

    return *this;
}

void NVInfo::encode(char *data) {
    for (auto attr : this->attributes) {
        attr.encode(data);
        data += attr.size;
    }
}

void NVInfo::dump() {
    for (auto attr : this->attributes) {
        attr.dump();
    }
}

void NVInfo::add_attribute(const NVInfoAttribute &attr) {
    this->attributes.push_back(attr);
    this->size += attr.size;
}

NVInfoAttribute::NVInfoAttribute(EIATTR attr, uint16_t i_value): s_value(nullptr) {
    this->attr = attr;
    this->fmt = EIFMT(attr & 0xff);

    if (this->fmt == EIFMT_SVAL) {
        throw std::runtime_error("Invalid format for integer value");
    }

    this->i_value = i_value;
    this->name = eiattr_str(attr);

    if (this->fmt == EIFMT_BVAL) {
        this->size = 2 + 1;
    } else {
        this->size = 2 + 2;
    }
}

NVInfoAttribute::NVInfoAttribute(EIATTR attr, uint32_t *s_value, uint16_t s_count): s_value(nullptr) {
    this->attr = attr;
    this->fmt = EIFMT(attr & 0xff);

    if (this->fmt != EIFMT_SVAL) {
        throw std::runtime_error("Invalid format for string value");
    }

    this->s_count = s_count;
    this->s_value = new uint32_t[s_count];
    for (uint16_t i = 0; i < s_count; i++) {
        this->s_value[i] = s_value[i];
    }
    this->name = eiattr_str(attr);
    this->size = 2 + 2 + (s_count << 2);
}