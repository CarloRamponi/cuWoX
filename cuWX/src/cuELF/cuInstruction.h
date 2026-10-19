#pragma once

#include <cstddef>
#include <cstdint>

namespace cuInstruction {

    class Instruction;
    class CuControlCode;

    namespace OPCODE {
        enum : uint16_t {
            RET    = 0b100101010000,
            LOP3   = 0b100000010010,
            TRAP   = 0b100101011100,
            ISETP  = 0b100000001100,
            BRA    = 0b100101000111,
            NOP    = 0b100100011000,
            LD     = 0b100110000000,
            LDL    = 0b100110000011,
            LDG    = 0b100110000001,
            LDC    = 0b101110000010,
            IADD_R = 0b001000010000,
            IADD_I = 0b100000010000
        };
    };

    namespace REG {
        enum: uint8_t {
            R0,  R1, R2, R3, R4, R5, R6, R7, R8, R9, R10,
            R11, R12, R13, R14, R15, R16, R17, R18, R19, R20,
            R21, R22, R23, R24, R25, R26, R27, R28, R29, R30,
            R31, R32, R33, R34, R35, R36, R37, R38, R39, R40,
            R41, R42, R43, R44, R45, R46, R47, R48, R49, R50,
            R51, R52, R53, R54, R55, R56, R57, R58, R59, R60,
            R61, R62, R63, R64, R65, R66, R67, R68, R69, R70,
            R71, R72, R73, R74, R75, R76, R77, R78, R79, R80,
            R81, R82, R83, R84, R85, R86, R87, R88, R89, R90,
            R91, R92, R93, R94, R95, R96, R97, R98, R99, R100,
            R101, R102, R103, R104, R105, R106, R107, R108, R109, R110,
            R111, R112, R113, R114, R115, R116, R117, R118, R119, R120,
            R121, R122, R123, R124, R125, R126, R127, R128, R129, R130,
            R131, R132, R133, R134, R135, R136, R137, R138, R139, R140,
            R141, R142, R143, R144, R145, R146, R147, R148, R149, R150,
            R151, R152, R153, R154, R155, R156, R157, R158, R159, R160,
            R161, R162, R163, R164, R165, R166, R167, R168, R169, R170,
            R171, R172, R173, R174, R175, R176, R177, R178, R179, R180,
            R181, R182, R183, R184, R185, R186, R187, R188, R189, R190,
            R191, R192, R193, R194, R195, R196, R197, R198, R199, R200,
            R201, R202, R203, R204, R205, R206, R207, R208, R209, R210,
            R211, R212, R213, R214, R215, R216, R217, R218, R219, R220,
            R221, R222, R223, R224, R225, R226, R227, R228, R229, R230,
            R231, R232, R233, R234, R235, R236, R237, R238, R239, R240,
            R241, R242, R243, R244, R245, R246, R247, R248, R249, R250,
            R251, R252, R253, R254, RZ
        };
    };

    namespace PRED {
        enum: uint8_t {
            P0, P1, P2, P3, P4, P5, P6, PT,
            NP0, NP1, NP2, NP3, NP4, NP5, NP6, NPT
        };
    };

    class CuControlCode {
        public:
            union {
                uint8_t data[3];
                struct {
                    uint8_t : 1; // Reserved
                    uint8_t STALL : 4;
                    uint8_t YIELD : 1;
                    uint8_t WRITE_MASK : 3;
                    uint8_t READ_MASK : 3;
                    uint8_t WAIT_MASK : 6;
                    uint8_t REUSE : 4;
                    uint8_t : 2; // Reserved
                } __attribute__((packed));
            } __attribute__((packed));

            void print();
            static CuControlCode create(
                uint8_t STALL = 0b1111,
                uint8_t YIELD = 0b0,
                uint8_t WRITE_MASK = 0b111,
                uint8_t READ_MASK = 0b111,
                uint8_t WAIT_MASK = 0b000011,
                uint8_t REUSE = 0b0000
            );
    };

    class ISETP {
        public:
            struct {
                uint8_t : 8;
                uint8_t reg_src: 8;
                uint32_t imm: 32;
                uint8_t : 8;
                struct {
                    uint8_t EX: 1;
                    uint8_t SIGN: 1;
                    uint8_t OR: 1;
                    uint8_t XOR: 1;
                    uint8_t LT: 1;
                    uint8_t EQ: 1;
                    uint8_t GT: 1;
                    uint8_t : 1;
                } __attribute__((packed)) flags;
                uint8_t : 1;
                uint8_t pred_dst: 3;
                uint8_t pred_src1: 3;
                uint8_t pred_src2: 4;
                uint16_t : 13;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_src = REG::RZ,
                uint32_t imm = 0,
                uint8_t EX = 0,
                uint8_t SIGN = 0,
                uint8_t OR = 0,
                uint8_t XOR = 0,
                uint8_t LT = 0,
                uint8_t EQ = 0,
                uint8_t GT = 0,
                uint8_t pred_dst = PRED::PT,
                uint8_t pred_src1 = PRED::PT,
                uint8_t pred_src2 = PRED::PT,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class TRAP {
        public:
            struct {
                uint32_t : 18;
                uint8_t trap_num: 3;
                uint64_t : 47;
                uint8_t fixed_011: 3;
                uint8_t : 6;
                uint8_t fixed_0: 1;
                uint16_t: 10;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t trap_num = 0b000,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class BRA {
        public:
            struct {
                uint8_t offset_lo: 8;
                uint8_t : 8;
                uint8_t flag_U: 1;
                uint8_t flag_DIV: 1;
                uint64_t offset: 48;
                uint8_t : 3;
                uint8_t flag_INC: 1;
                uint8_t flag_DEC: 1;
                uint8_t pred_dst: 3;
                uint16_t : 14;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);

            int64_t get_offset80(uint64_t instruction_offset = 0);
            int64_t get_offset90(uint64_t instruction_offset = 0);
            void set_offset80(uint64_t instruction_offset = 0, int64_t offset = 0);
            void set_offset90(uint64_t instruction_offset = 0, int64_t offset = 0);

            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t flag_U = 0,
                uint8_t flag_DIV = 0,
                uint8_t flag_INC = 0,
                uint8_t flag_DEC = 0,
                uint8_t pred_dst = PRED::PT,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class RET {
        public:
            struct {
                uint8_t offset_lo: 8;
                uint8_t reg_src: 8;
                uint8_t : 2;
                uint64_t offset: 48;
                uint8_t : 3;
                uint8_t flag_ABS: 1;
                uint8_t flag_NODEC: 1;
                uint8_t pred_dst: 4;
                uint8_t flag_UREG: 1;
                uint16_t : 12;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);

            int64_t get_offset80(uint64_t instruction_offset = 0);
            int64_t get_offset90(uint64_t instruction_offset = 0);
            void set_offset80(uint64_t instruction_offset = 0, int64_t offset = 0);
            void set_offset90(uint64_t instruction_offset = 0, int64_t offset = 0);

            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_src = REG::RZ,
                uint8_t flag_ABS = 0,
                uint8_t flag_NODEC = 0,
                uint8_t flag_UREG = 0,
                uint8_t pred_dst = PRED::PT,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class LOP3 {
        public:
            enum FuncCode : uint8_t {
                XOR = 0x3c,
            };

            struct {
                uint8_t reg_dst: 8;
                uint8_t reg_src1: 8;
                uint32_t imm: 32;
                uint8_t reg_src2: 8;
                uint8_t func_code: 8;
                uint8_t flag_pand: 1;
                uint8_t pred_dst: 3;
                uint8_t : 3;
                uint8_t pred_src: 4;
                uint16_t: 13;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_dst = REG::RZ,
                uint8_t reg_src1 = REG::RZ,
                uint32_t imm = 0,
                uint8_t reg_src2 = REG::RZ,
                uint8_t func_code = 0,
                uint8_t pred_dst = PRED::PT,
                uint8_t pred_src = PRED::PT,
                uint8_t flag_pand = 0b0,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class NOP {
        public:
            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class LD {
        public:
            struct {
                uint8_t reg_dst: 8;
                uint8_t reg_src: 8;
                uint8_t : 8;
                uint32_t imm: 24;
                uint8_t pred_src: 4;
                uint32_t weird_flags : 23;
                uint8_t flag_64b : 1;
                uint16_t : 9;
                uint8_t flag_desc : 1;
                uint8_t : 2;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_dst = REG::RZ,
                uint8_t reg_src = REG::RZ,
                uint32_t imm = 0,
                uint8_t pred_src = PRED::PT,
                uint8_t flag_64b = 0,
                uint8_t flag_desc = 0,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class LDL {
        public:
            enum data_size {
                SIZE_U8 = 0b000,
                SIZE_S8 = 0b001,
                SIZE_U16 = 0b010,
                SIZE_S16 = 0b011,
                SIZE_32 = 0b100,
                SIZE_64 = 0b101,
                SIZE_128 = 0b110,
                SIZE_INVALID = 0b111
            };

            enum flags {
                FLAG_EF = 0b000,
                FLAG_NONE = 0b001,
                FLAG_EL = 0b010,
                FLAG_LU = 0b011,
                FLAG_EU = 0b100,
                FLAG_NA = 0b101,
                FLAG_INVALID6 = 0b110,
                FLAG_INVALID7 = 0b111
            };

            struct {
                uint8_t reg_dst: 8;
                uint8_t reg_src: 8;
                uint8_t : 8;
                uint32_t imm: 24;
                uint16_t : 9;
                enum data_size data_size : 3;
                uint8_t : 8;
                enum flags flags : 3;
                uint16_t : 14;
                uint8_t flag_desc : 1;
                uint8_t : 2;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_dst = REG::RZ,
                uint8_t reg_src = REG::RZ,
                uint32_t imm = 0,
                enum data_size data_size = SIZE_32,
                enum flags flags = FLAG_NONE,
                uint8_t flag_desc = 0,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class LDG {
        public:
            struct {
                uint8_t reg_dst: 8;
                uint8_t reg_src: 8;
                uint8_t : 8;
                uint32_t imm: 24;
                // Haven't reversed the full instruction because I only needed destination register
                uint64_t : 40;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_dst = REG::RZ,
                uint8_t reg_src = REG::RZ,
                uint32_t imm = 0,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class LDC {
        public:
            struct {
                uint8_t reg_dst: 8;
                uint8_t reg_src: 8;
                uint8_t : 8;
                uint32_t imm: 24;
                // Haven't reversed the full instruction because I only needed destination register
                uint64_t : 40;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_dst = REG::RZ,
                uint8_t reg_src = REG::RZ,
                uint32_t imm = 0,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class IADD_R {
        public:
            struct {
                uint8_t reg_dst: 8;
                uint8_t reg_src1: 8;
                uint8_t reg_src2: 8;
                uint32_t : 23;
                uint8_t NEG_src2: 1;
                uint8_t reg_src3: 8;
                uint8_t NEG_src1: 1;
                uint8_t : 1;
                uint8_t flag_ADDX: 1;
                uint8_t NEG_src3: 1;
                uint8_t : 5;
                uint8_t pred_src1: 3;
                uint8_t pred_src2: 3;
                uint32_t : 17;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_dst = REG::RZ,
                uint8_t reg_src1 = REG::RZ,
                uint8_t reg_src2 = REG::RZ,
                uint8_t reg_src3 = REG::RZ,
                uint8_t NEG_src1 = 0,
                uint8_t NEG_src2 = 0,
                uint8_t NEG_src3 = 0,
                uint8_t pred_src1 = PRED::PT,
                uint8_t pred_src2 = PRED::PT,
                uint8_t flag_ADDX = 0,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class IADD_I {
        public:
            struct {
                uint8_t reg_dst: 8;
                uint8_t reg_src1: 8;
                uint32_t imm : 32;
                uint8_t reg_src2: 8;
                uint8_t NEG_src1: 1;
                uint8_t : 1;
                uint8_t flag_ADDX: 1;
                uint8_t NEG_src2: 1;
                uint8_t : 5;
                uint8_t pred_src1: 3;
                uint8_t pred_src2: 3;
                uint32_t : 17;
            } __attribute__((packed));

            void print(uint64_t instruction_offset = 0);
            static Instruction create(
                uint8_t pred = PRED::PT,
                uint8_t reg_dst = REG::RZ,
                uint8_t reg_src1 = REG::RZ,
                uint8_t reg_src2 = REG::RZ,
                uint32_t imm = 0,
                uint8_t NEG_src1 = 0,
                uint8_t NEG_src2 = 0,
                uint8_t pred_src1 = PRED::PT,
                uint8_t pred_src2 = PRED::PT,
                uint8_t flag_ADDX = 0,
                CuControlCode control_code = CuControlCode::create()
            );
    };

    class Instruction {
        public:
            union {
                uint8_t data[16] = {0};
                struct {
                    uint16_t opcode: 12;
                    uint8_t pred: 4;
                    union {
                        ISETP isetp;
                        LOP3 lop3;
                        RET ret;
                        BRA bra;
                        TRAP trap;
                        NOP nop;
                        LD ld;
                        LDL ldl;
                        LDC ldc;
                        LDG ldg;
                        IADD_R iadd_r;
                        IADD_I iadd_i;
                    };
                    CuControlCode control_code;
                } __attribute__((packed));
            };

            void print(uint64_t instruction_offset = 0);
            void print_bits();
    };
};