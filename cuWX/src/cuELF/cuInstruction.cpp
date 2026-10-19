#include <cuInstruction.h>
#include <cstdio>
#include <cstdint>
#include <stdlib.h>

/******************************* Init function *******************************/

void __attribute__((constructor)) init() {
    static_assert(sizeof(cuInstruction::Instruction) == 16, "Instruction size is not 16 bytes");
}

/******************************* CuControlCode *******************************/

void cuInstruction::CuControlCode::print() {
    printf("[B");
    for(int i = 0; i < 6; i++) {
        if (WAIT_MASK >> i & 1) {
            printf("%d", i);
        } else {
            printf("-");
        }
    }

    printf(":R");
    if(READ_MASK == 7) {
        printf("-");
    } else {
        printf("%d", READ_MASK);
    }

    printf(":W");
    if(WRITE_MASK == 7) {
        printf("-");
    } else {
        printf("%d", WRITE_MASK);
    }

    if(YIELD != 0) {
        printf(":-");
    } else {
        printf(":Y");
    }

    printf(":S%02d]", STALL);
}

cuInstruction::CuControlCode cuInstruction::CuControlCode::create(
    uint8_t STALL,
    uint8_t YIELD,
    uint8_t WRITE_MASK,
    uint8_t READ_MASK,
    uint8_t WAIT_MASK,
    uint8_t REUSE
) {
    CuControlCode cc;
    
    for(int i = 0; i < 3; i++) {
        cc.data[i] = 0;
    }

    cc.STALL = STALL;
    cc.YIELD = YIELD;
    cc.WRITE_MASK = WRITE_MASK;
    cc.READ_MASK = READ_MASK;
    cc.WAIT_MASK = WAIT_MASK;
    cc.REUSE = REUSE;
    return cc;
}

/******************************* INSTRUCTION *******************************/

void cuInstruction::Instruction::print(uint64_t instruction_offset) {
    printf("%04lx:\t", instruction_offset);
    control_code.print();
    printf("\t");

    if (pred != 0b111) {
        printf("   @");
        if (pred >> 3 & 1) {
            printf("!P");
        } else {
            printf("P");
        }
        printf("%d", pred & 0b111);
    }
    printf("\t");

    switch (opcode) {
        case cuInstruction::OPCODE::BRA:
            bra.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::LOP3:
            lop3.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::TRAP:
            trap.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::ISETP:
            isetp.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::RET:
            ret.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::NOP:
            nop.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::LD:
            ld.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::LDL:
            ldl.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::LDG:
            ldg.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::LDC:
            ldc.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::IADD_R:
            iadd_r.print(instruction_offset);
            break;
        case cuInstruction::OPCODE::IADD_I:
            iadd_i.print(instruction_offset);
            break;
        default:
            printf("Unknown opcode: %04x", opcode);
            break;
    }
    printf("\n");
}

void cuInstruction::Instruction::print_bits() {
    for (int i = 15; i >= 0; i--) {
        for (int j = 7; j >= 0; j--) {
            printf("%d", (data[i] >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

/******************************* RET *******************************/

void cuInstruction::RET::print(uint64_t instruction_offset) {
    printf("RET%s%s %sR%d 0x%lx (sm>=90: 0x%lx);",
        flag_ABS ? ".ABS" : ".REL",
        flag_NODEC ? ".NODEC" : "",
        flag_UREG ? "U" : "",
        reg_src,
        get_offset80(instruction_offset),
        get_offset90(instruction_offset)
    );
}

int64_t cuInstruction::RET::get_offset80(uint64_t instruction_offset) {
    // sign extend the offset from 48 bits to 64 bits
    int64_t ex_offset = (int64_t)(offset & 0x0000FFFFFFFFFFFF);
    if (ex_offset >> 47 & 1) {
        ex_offset |= 0xFFFF000000000000;
    }

    ex_offset = (ex_offset << 2);

    if (flag_ABS) {
        return ex_offset;
    } else {
        return instruction_offset + 0x10 + ex_offset;
    }
}

void cuInstruction::RET::set_offset80(uint64_t instruction_offset, int64_t offset) {
    if (!flag_ABS) {
        // If the instruction is not absolute, we need to calculate the offset
        // relative to the instruction address
        offset -= instruction_offset + 0x10;
    }
    
    // Right shift by 2 to reverse the left shift in get_offset
    offset = offset >> 2;

    // Mask to fit into 48 bits
    this->offset = offset & 0x0000FFFFFFFFFFFF;
}

int64_t cuInstruction::RET::get_offset90(uint64_t instruction_offset) {
    // sign extend the offset from 48 bits to 64 bits
    int64_t ex_offset = (int64_t)(offset & 0x0000FFFFFFFFFFFF);
    if (ex_offset >> 47 & 1) {
        ex_offset |= 0xFFFF000000000000;
    }

    ex_offset = (ex_offset << 10) | (offset_lo << 2);;

    if (flag_ABS) {
        return ex_offset;
    } else {
        return instruction_offset + 0x10 + ex_offset;
    }
}

void cuInstruction::RET::set_offset90(uint64_t instruction_offset, int64_t offset) {
    if (!flag_ABS) {
        // If the instruction is not absolute, we need to calculate the offset
        // relative to the instruction address
        offset -= instruction_offset + 0x10;
    }

    // Mask to fit into 48 bits
    this->offset = (offset >> 10) & 0x0000FFFFFFFFFFFF;
    this->offset_lo = (offset >> 2) & 0xFF;
}

cuInstruction::Instruction cuInstruction::RET::create(
    uint8_t pred,
    uint8_t reg_src,
    uint8_t flag_ABS,
    uint8_t flag_NODEC,
    uint8_t flag_UREG,
    uint8_t pred_dst,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::RET;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.ret.reg_src = reg_src;
    instr.ret.flag_ABS = flag_ABS;
    instr.ret.flag_NODEC = flag_NODEC;
    instr.ret.flag_UREG = flag_UREG;
    instr.ret.pred_dst = pred_dst;

    return instr;
}

/******************************* LOP3 *******************************/

void cuInstruction::LOP3::print(uint64_t instruction_offset) {
    printf("LOP3 R%d, R%d, 0x%x, R%d, 0x%02x, %sP%d ;",
        reg_dst,
        reg_src1,
        imm,
        reg_src2,
        func_code,
        (pred_src >> 3 & 1) ? "!" : "",
        pred_src & 0b111
    );
}

cuInstruction::Instruction cuInstruction::LOP3::create(
    uint8_t pred,
    uint8_t reg_dst,
    uint8_t reg_src1,
    uint32_t imm,
    uint8_t reg_src2,
    uint8_t func_code,
    uint8_t pred_dst,
    uint8_t pred_src,
    uint8_t flag_pand,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::LOP3;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.lop3.reg_dst = reg_dst;
    instr.lop3.reg_src1 = reg_src1;
    instr.lop3.imm = imm;
    instr.lop3.reg_src2 = reg_src2;
    instr.lop3.func_code = func_code;
    instr.lop3.pred_dst = pred_dst;
    instr.lop3.pred_src = pred_src;

    return instr;
}

/******************************* TRAP *******************************/

void cuInstruction::TRAP::print(uint64_t instruction_offset) {
    printf("TRAP 0x%x ;", trap_num);
}

cuInstruction::Instruction cuInstruction::TRAP::create(
    uint8_t pred,
    uint8_t trap_num,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::TRAP;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.trap.trap_num = trap_num;
    instr.trap.fixed_011 = 0b011;
    instr.trap.fixed_0 = 0b0;

    return instr;
}

/******************************* ISETP *******************************/

void cuInstruction::ISETP::print(uint64_t instruction_offset) {
    printf(
        "ISETP%s%s P%c, %cP%c, R%d, 0x%x, %cP%c ;",
        (flags.GT == 0 ? (
            flags.EQ == 0 ? (
                flags.LT == 0 ? ".F" : ".LT"
            ) : (
                flags.LT == 0 ? ".EQ" : ".LE"
            )
        ) : (
            flags.EQ == 0 ? (
                flags.LT == 0 ? ".GT" : ".NE"
            ) : (
                flags.LT == 0 ? ".GE" : ".T"
            )
        )),
        (flags.XOR == 0 ? (
            flags.OR == 0 ? ".AND" : ""
        ) : (
            flags.OR == 0 ? ".XOR" : ".OR"
        )),
        (pred_dst == 0b111 ? 'T' : '0' + pred_dst),
        (pred_src1 >> 3 & 1 ? '!' : ' '),
        ((pred_src1 & 0b111) == 0b111 ? 'T' : '0' + (pred_src1 & 0b111)),
        reg_src,
        imm,
        (pred_src2 >> 3 & 1 ? '!' : ' '),
        ((pred_src2 & 0b111) == 0b111 ? 'T' : '0' + (pred_src2 & 0b111))
    );
}

cuInstruction::Instruction cuInstruction::ISETP::create(
    uint8_t pred,
    uint8_t reg_src,
    uint32_t imm,
    uint8_t EX,
    uint8_t SIGN,
    uint8_t OR,
    uint8_t XOR,
    uint8_t LT,
    uint8_t EQ,
    uint8_t GT,
    uint8_t pred_dst,
    uint8_t pred_src1,
    uint8_t pred_src2,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::ISETP;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.isetp.reg_src = reg_src;
    instr.isetp.imm = imm;
    instr.isetp.flags.EX = EX;
    instr.isetp.flags.SIGN = SIGN;
    instr.isetp.flags.OR = OR;
    instr.isetp.flags.XOR = XOR;
    instr.isetp.flags.LT = LT;
    instr.isetp.flags.EQ = EQ;
    instr.isetp.flags.GT = GT;
    instr.isetp.pred_dst = pred_dst;
    instr.isetp.pred_src1 = pred_src1;
    instr.isetp.pred_src2 = pred_src2;

    return instr;
}

/******************************* BRA *******************************/

void cuInstruction::BRA::print(uint64_t instruction_offset) {
    printf(
        "BRA%s%s 0x%lx (sm>=90: 0x%lx);",
        flag_U && flag_DIV ? ".CONV" : flag_U ? ".U" : flag_DIV ? ".DIV" : "",
        flag_INC ? ".INC" : flag_DEC ? ".DEC" : "",
        get_offset80(instruction_offset),
        get_offset90(instruction_offset)
    );
}

int64_t cuInstruction::BRA::get_offset90(uint64_t instruction_offset) {
    // sign extend the offset from 48 bits to 64 bits
    int64_t ex_offset = (int64_t)(offset & 0x0000FFFFFFFFFFFF);
    if (ex_offset >> 47 & 1) {
        ex_offset |= 0xFFFF000000000000;
    }

    ex_offset = (ex_offset << 10) | (offset_lo << 2);
    return instruction_offset + 0x10 + ex_offset;
}

void cuInstruction::BRA::set_offset90(uint64_t instruction_offset, int64_t offset) {
    // Calculate the offset relative to the instruction address
    int64_t relative_offset = offset - (instruction_offset + 0x10);

    // Mask to fit into 48 bits
    this->offset = (relative_offset >> 10) & 0x0000FFFFFFFFFFFF;
    this->offset_lo = (relative_offset >> 2) & 0xFF;
}

int64_t cuInstruction::BRA::get_offset80(uint64_t instruction_offset) {
    // sign extend the offset from 48 bits to 64 bits
    int64_t ex_offset = (int64_t)(offset & 0x0000FFFFFFFFFFFF);
    if (ex_offset >> 47 & 1) {
        ex_offset |= 0xFFFF000000000000;
    }

    ex_offset = (ex_offset << 2);
    return instruction_offset + 0x10 + ex_offset;
}

void cuInstruction::BRA::set_offset80(uint64_t instruction_offset, int64_t offset) {
    // Calculate the offset relative to the instruction address
    int64_t relative_offset = offset - (instruction_offset + 0x10);

    // Right shift by 2 to reverse the left shift in get_offset
    relative_offset = relative_offset >> 2;

    // Mask to fit into 48 bits
    this->offset = relative_offset & 0x0000FFFFFFFFFFFF;
}

cuInstruction::Instruction cuInstruction::BRA::create(
    uint8_t pred,
    uint8_t flag_U,
    uint8_t flag_DIV,
    uint8_t flag_INC,
    uint8_t flag_DEC,
    uint8_t pred_dst,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::BRA;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.bra.flag_U = flag_U;
    instr.bra.flag_DIV = flag_DIV;
    instr.bra.flag_INC = flag_INC;
    instr.bra.flag_DEC = flag_DEC;
    instr.bra.pred_dst = pred_dst;

    return instr;
}

/******************************* NOP *******************************/

void cuInstruction::NOP::print(uint64_t instruction_offset) {
    printf("NOP ;");
}

cuInstruction::Instruction cuInstruction::NOP::create(
    uint8_t pred,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::NOP;
    instr.pred = pred;
    instr.control_code = control_code;
    return instr;
}

/******************************* LD *******************************/
void cuInstruction::LD::print(uint64_t instruction_offset) {
    printf("LD R%d, %s[R%d%s+0x%x], %sP%d ;",
        reg_dst,
        (flag_desc ? "desc" : ""),
        reg_src,
        (flag_64b ? ".64" : ""),
        imm,
        (pred_src >> 3 & 1) ? "!" : "",
        (pred_src & 0b111)
    );
}

cuInstruction::Instruction cuInstruction::LD::create(
    uint8_t pred,
    uint8_t reg_dst,
    uint8_t reg_src,
    uint32_t imm,
    uint8_t pred_src,
    uint8_t flag_64b,
    uint8_t flag_desc,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::LD;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.ld.reg_dst = reg_dst;
    instr.ld.reg_src = reg_src;
    instr.ld.imm = imm;
    instr.ld.weird_flags = 0b10000010000000100010000;
    instr.ld.pred_src = pred_src;
    instr.ld.flag_64b = flag_64b;
    instr.ld.flag_desc = flag_desc;

    return instr;
}

/******************************* LDL *******************************/
void cuInstruction::LDL::print(uint64_t instruction_offset) {
    printf("LDL");
    
    switch (data_size) {
        case SIZE_U8: printf(".U8"); break;
        case SIZE_S8: printf(".S8"); break;
        case SIZE_U16: printf(".U16"); break;
        case SIZE_S16: printf(".S16"); break;
        case SIZE_32: printf(".32"); break;
        case SIZE_64: printf(".64"); break;
        case SIZE_128: printf(".128"); break;
        default: printf(".INVALID");
    }

    switch (flags) {
        case FLAG_NONE: break;
        case FLAG_EF: printf(".EF"); break;
        case FLAG_EL: printf(".EL"); break;
        case FLAG_LU: printf(".LU"); break;
        case FLAG_EU: printf(".EU"); break;
        case FLAG_NA: printf(".NA"); break;
        case FLAG_INVALID6: printf(".INVALID6"); break;
        case FLAG_INVALID7: printf(".INVALID7"); break;
    }

    printf(" R%d, ", reg_dst);

    if(flag_desc) {
        printf("desc");
    }

    if(reg_src != REG::RZ) {
        int32_t signed_offset = (int32_t)imm;
        printf("[R%d%c0x%x]", reg_src, (signed_offset < 0 ? '-' : '+'), abs(signed_offset));
    } else {
        printf("[0x%x]", imm);
    }

    printf(" ;");
}

cuInstruction::Instruction cuInstruction::LDL::create(
    uint8_t pred,
    uint8_t reg_dst,
    uint8_t reg_src,
    uint32_t imm,
    enum data_size data_size,
    enum flags flags,
    uint8_t flag_desc,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::LDL;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.ldl.reg_dst = reg_dst;
    instr.ldl.reg_src = reg_src;
    instr.ldl.imm = imm;
    instr.ldl.flag_desc = flag_desc;
    instr.ldl.data_size = data_size;
    instr.ldl.flags = flags;

    return instr;
}

/******************************* LDG *******************************/
void cuInstruction::LDG::print(uint64_t instruction_offset) {
    printf("LDG R%d, [R%d+0x%x] [unreliable] ;",
        reg_dst,
        reg_src,
        imm
    );
}

cuInstruction::Instruction cuInstruction::LDG::create(
    uint8_t pred,
    uint8_t reg_dst,
    uint8_t reg_src,
    uint32_t imm,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::LDG;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.ldg.reg_dst = reg_dst;
    instr.ldg.reg_src = reg_src;
    instr.ldg.imm = imm;

    return instr;
}

/******************************* LDC *******************************/
void cuInstruction::LDC::print(uint64_t instruction_offset) {
    printf("LDC R%d, [R%d+0x%x] [unreliable] ;",
        reg_dst,
        reg_src,
        imm
    );
}

cuInstruction::Instruction cuInstruction::LDC::create(
    uint8_t pred,
    uint8_t reg_dst,
    uint8_t reg_src,
    uint32_t imm,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::LDC;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.ldc.reg_dst = reg_dst;
    instr.ldc.reg_src = reg_src;
    instr.ldc.imm = imm;

    return instr;
}

/******************************* IADD_R *******************************/

void cuInstruction::IADD_R::print(uint64_t instruction_offset) {
    printf("IADD%s R%d, P%d, P%d, %sR%d, %sR%d, %sR%d ;",
        (flag_ADDX ? ".X [unreliable]" : ""),
        reg_dst,
        pred_src1,
        pred_src2,
        (NEG_src1 ? "-" : ""),
        reg_src1,
        (NEG_src2 ? "-" : ""),
        reg_src2,
        (NEG_src3 ? "-" : ""),
        reg_src3
    );
}

cuInstruction::Instruction cuInstruction::IADD_R::create(
    uint8_t pred,
    uint8_t reg_dst,
    uint8_t reg_src1,
    uint8_t reg_src2,
    uint8_t reg_src3,
    uint8_t NEG_src1,
    uint8_t NEG_src2,
    uint8_t NEG_src3,
    uint8_t pred_src1,
    uint8_t pred_src2,
    uint8_t flag_ADDX,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::IADD_R;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.iadd_r.reg_dst = reg_dst;
    instr.iadd_r.reg_src1 = reg_src1;
    instr.iadd_r.reg_src2 = reg_src2;
    instr.iadd_r.reg_src3 = reg_src3;
    instr.iadd_r.NEG_src1 = NEG_src1;
    instr.iadd_r.NEG_src2 = NEG_src2;
    instr.iadd_r.NEG_src3 = NEG_src3;
    instr.iadd_r.pred_src1 = pred_src1;
    instr.iadd_r.pred_src2 = pred_src2;
    instr.iadd_r.flag_ADDX = flag_ADDX;

    return instr;
};

/******************************* IADD_I *******************************/

void cuInstruction::IADD_I::print(uint64_t instruction_offset) {
    printf("IADD%s R%d, P%d, P%d, %sR%d, 0x%x, %sR%d ;",
        (flag_ADDX ? ".X [unreliable]" : ""),
        reg_dst,
        pred_src1,
        pred_src2,
        (NEG_src1 ? "-" : ""),
        reg_src1,
        imm,
        (NEG_src2 ? "-" : ""),
        reg_src2
    );
}

cuInstruction::Instruction cuInstruction::IADD_I::create(
    uint8_t pred,
    uint8_t reg_dst,
    uint8_t reg_src1,
    uint8_t reg_src2,
    uint32_t imm,
    uint8_t NEG_src1,
    uint8_t NEG_src2,
    uint8_t pred_src1,
    uint8_t pred_src2,
    uint8_t flag_ADDX,
    CuControlCode control_code
) {
    cuInstruction::Instruction instr;
    instr.opcode = OPCODE::IADD_I;
    instr.pred = pred;
    instr.control_code = control_code;

    instr.iadd_i.reg_dst = reg_dst;
    instr.iadd_i.reg_src1 = reg_src1;
    instr.iadd_i.reg_src2 = reg_src2;
    instr.iadd_i.imm = imm;
    instr.iadd_i.NEG_src1 = NEG_src1;
    instr.iadd_i.NEG_src2 = NEG_src2;
    instr.iadd_i.pred_src1 = pred_src1;
    instr.iadd_i.pred_src2 = pred_src2;
    instr.iadd_i.flag_ADDX = flag_ADDX;

    return instr;
};