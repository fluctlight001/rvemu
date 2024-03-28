#include "rvemu.h"

#include "interp_util.h"

static void func_empty(state_t *state, inst_t *inst) {}

#define FUNC_LOAD(type) \
    u64 addr = state->gpr[inst->rs1] + (i64)inst->imm; \
    state->gpr[inst->rd] = *(type *)TO_HOST(addr)

#define FUNC_STORE(type) \
    u64 rs1 = state->gpr[inst->rs1];         \
    u64 rs2 = state->gpr[inst->rs2];         \
    *(type *)TO_HOST(rs1 + inst->imm) = (type)rs2

#define FUNC_ALU_I(expr) \
    u64 rs1 = state->gpr[inst->rs1]; \
    i64 imm = (i64)inst->imm;            \
    state->gpr[inst->rd] = (expr)  

#define FUNC_ALU_R(expr) \
    u64 rs1 = state->gpr[inst->rs1]; \
    u64 rs2 = state->gpr[inst->rs2]; \
    state->gpr[inst->rd] = (expr)

#define FUNC_BRANCH(expr) \
    u64 rs1 = state->gpr[inst->rs1];             \
    u64 rs2 = state->gpr[inst->rs2];             \
    u64 target_addr = state->pc + (i64)inst->imm;    \
    if (expr) {                                      \
        state->reenter_pc = state->pc = target_addr; \
        state->exit_reason = direct_branch;          \
        inst->cont = true;                           \
    }       

#define FUNC_CSR()                     \
    fatal("unsupported csr");          \
    state->gpr[inst->rd] = 0;          \


static void func_lb(state_t *state, inst_t *inst) {
    FUNC_LOAD(i8);
}

static void func_lh(state_t *state, inst_t *inst) {
    FUNC_LOAD(i16);
}

static void func_lw(state_t *state, inst_t *inst) {
    FUNC_LOAD(i32);
}

static void func_ld(state_t *state, inst_t *inst) {
    FUNC_LOAD(i64);
}

static void func_lbu(state_t *state, inst_t *inst) {
    FUNC_LOAD(u8);
}

static void func_lhu(state_t *state, inst_t *inst) {
    FUNC_LOAD(u16);
}

static void func_lwu(state_t *state, inst_t *inst) {
    FUNC_LOAD(u32);
}

static void func_addi(state_t *state, inst_t *inst) {
    FUNC_ALU_I(rs1 + imm);
}

static void func_slli(state_t *state, inst_t *inst) {
    FUNC_ALU_I(rs1 << (imm & 0x3f));
}

static void func_slti(state_t *state, inst_t *inst) {
    FUNC_ALU_I((i64)rs1 < (i64)imm);
}

static void func_sltiu(state_t *state, inst_t *inst) {
    FUNC_ALU_I((u64)rs1 < (u64)imm);
}

static void func_xori(state_t *state, inst_t *inst) {
    FUNC_ALU_I(rs1 ^ imm);
}

static void func_srli(state_t *state, inst_t *inst) {
    FUNC_ALU_I(rs1 >> (imm & 0x3f));
}

static void func_srai(state_t *state, inst_t *inst) {
    FUNC_ALU_I((i64)rs1 >> (imm & 0x3f));
}

static void func_ori(state_t *state, inst_t *inst) {
    FUNC_ALU_I(rs1 | (u64)imm);
}

static void func_andi(state_t *state, inst_t *inst) {
    FUNC_ALU_I(rs1 & (u64)imm);
}

static void func_auipc(state_t *state, inst_t *inst) {
    u64 val = state->pc + (i64)inst->imm;
    state->gpr[inst->rd] = val;
}

static void func_addiw(state_t *state, inst_t *inst) {
    FUNC_ALU_I((i64)(i32)(rs1 + imm));
}

static void func_slliw(state_t *state, inst_t *inst) {
    FUNC_ALU_I((i64)(i32)(rs1 << (imm & 0x1f)));
}

static void func_srliw(state_t *state, inst_t *inst) {
    FUNC_ALU_I((i64)(i32)((u32)rs1 >> (imm & 0x1f)));
}

static void func_sraiw(state_t *state, inst_t *inst) {
    FUNC_ALU_I((i64)((i32)rs1 >> (imm & 0x1f)));
}

static void func_sb(state_t *state, inst_t *inst) {
    FUNC_STORE(u8);
}

static void func_sh(state_t *state, inst_t *inst) {
    FUNC_STORE(u16);
}

static void func_sw(state_t *state, inst_t *inst) {
    FUNC_STORE(u32);
}

static void func_sd(state_t *state, inst_t *inst) {
    FUNC_STORE(u64);
}

static void func_add(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 + rs2);
}

static void func_sll(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 << (rs2 & 0x3f));
}

static void func_slt(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)rs1 < (i64)rs2);
}

static void func_sltu(state_t *state, inst_t *inst) {
    FUNC_ALU_R((u64)rs1 < (u64)rs2);
}

static void func_xor(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 ^ rs2);
}

static void func_srl(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 >> (rs2 & 0x3f));
}

static void func_or(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 | rs2);
}

static void func_and(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 & rs2);
}

static void func_mul(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 * rs2);
}

static void func_mulh(state_t *state, inst_t *inst) {
    FUNC_ALU_R(mulh(rs1, rs2));
}

static void func_mulhsu(state_t *state, inst_t *inst) {
    FUNC_ALU_R(mulhsu(rs1, rs2));
}

static void func_mulhu(state_t *state, inst_t *inst) {
    FUNC_ALU_R(mulhu(rs1, rs2));
}

static void func_div(state_t *state, inst_t *inst) {
    u64 rs1 = state->gpr[inst->rs1];
    u64 rs2 = state->gpr[inst->rs2];
    u64 rd = 0;
    if (rs2 == 0) {
        rd = UINT64_MAX;
    } else if (rs1 == INT64_MIN && rs2 == UINT64_MAX) {
        rd = INT64_MIN;
    } else {
        rd = (i64)rs1 / (i64)rs2;
    }
    state->gpr[inst->rd] = rd;
}

static void func_divu(state_t *state, inst_t *inst) {
    u64 rs1 = state->gpr[inst->rs1];
    u64 rs2 = state->gpr[inst->rs2];
    u64 rd = 0;
    if (rs2 == 0) {
        rd = UINT64_MAX;
    } else {
        rd = rs1 / rs2;
    }
    state->gpr[inst->rd] = rd;
}

static void func_rem(state_t *state, inst_t *inst) {
    u64 rs1 = state->gpr[inst->rs1];
    u64 rs2 = state->gpr[inst->rs2];
    u64 rd = 0;
    if (rs2 == 0) {
        rd = rs1;
    } else if (rs1 == INT64_MIN && rs2 == UINT64_MAX) {
        rd = 0;
    } else {
        rd = (i64)rs1 % (i64)rs2;
    }
    state->gpr[inst->rd] = rd;
}

static void func_remu(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs2 == 0 ? rs1 : rs1 % rs2);
}

static void func_sub(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs1 - rs2);
}

static void func_sra(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)rs1 >> (rs2 & 0x3f));
}

static void func_lui(state_t *state, inst_t *inst) {
    state->gpr[inst->rd] = (i64)inst->imm;
}

static void func_addw(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)(i32)(rs1 + rs2));
}

static void func_sllw(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)(i32)(rs1 << (rs2 & 0x1f)));
}

static void func_srlw(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)(i32)((u32)rs1 >> (rs2 & 0x1f)));
}

static void func_mulw(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)(i32)(rs1 * rs2));
}

static void func_divw(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs2 == 0 ? UINT64_MAX : (i32)((i64)(i32)rs1 / (i64)(i32)rs2));
}

static void func_divuw(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs2 == 0 ? UINT64_MAX : (i32)((u32)rs1 / (u32)rs2));
}

static void func_remw(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs2 == 0 ? (i64)(i32)rs1 : (i64)(i32)((i64)(i32)rs1 % (i64)(i32)rs2));
}

static void func_remuw(state_t *state, inst_t *inst) {
    FUNC_ALU_R(rs2 == 0 ? (i64)(i32)(u32)rs1 : (i64)(i32)((u32)rs1 % (u32)rs2));
}

static void func_subw(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)(i32)(rs1 - rs2));
}

static void func_sraw(state_t *state, inst_t *inst) {
    FUNC_ALU_R((i64)(i32)((i32)rs1 >> (rs2 & 0x1f)));
}

static void func_beq(state_t *state, inst_t *inst) {
    FUNC_BRANCH((u64)rs1 == (u64)rs2);
}

static void func_bne(state_t *state, inst_t *inst) {
    FUNC_BRANCH((u64)rs1 != (u64)rs2);
}

static void func_blt(state_t *state, inst_t *inst) {
    FUNC_BRANCH((i64)rs1 < (i64)rs2);
}

static void func_bge(state_t *state, inst_t *inst) {
    FUNC_BRANCH((i64)rs1 >= (i64)rs2);
}

static void func_bltu(state_t *state, inst_t *inst) {
    FUNC_BRANCH((u64)rs1 < (u64)rs2);
}

static void func_bgeu(state_t *state, inst_t *inst) {
    FUNC_BRANCH((u64)rs1 >= (u64)rs2);
}

static void func_jalr(state_t *state, inst_t *inst) {
    u64 rs1 = state->gpr[inst->rs1];
    state->gpr[inst->rd] = state->pc + (inst->rvc ? 2 : 4);
    state->exit_reason = indirect_branch;
    state->reenter_pc = (rs1 + (i64)inst->imm) & ~(u64)1;
    inst->cont = true;
    
}

static void func_jal(state_t *state, inst_t *inst) {
    state->gpr[inst->rd] = state->pc + (inst->rvc ? 2 : 4);
    state->reenter_pc = state->pc = state->pc + (i64)inst->imm;
    state->exit_reason = direct_branch;
    inst->cont = true;
}

static void func_ecall(state_t *state, inst_t *inst) {
    state->exit_reason = ecall;
    state->reenter_pc = state->pc + 4;
    inst->cont = true;
}

static void func_csrrw(state_t *state, inst_t *inst) { FUNC_CSR(); }
static void func_csrrs(state_t *state, inst_t *inst) { FUNC_CSR(); }
static void func_csrrc(state_t *state, inst_t *inst) { FUNC_CSR(); }
static void func_csrrwi(state_t *state, inst_t *inst) { FUNC_CSR(); }
static void func_csrrsi(state_t *state, inst_t *inst) { FUNC_CSR(); }
static void func_csrrci(state_t *state, inst_t *inst) { FUNC_CSR(); }

typedef void (func_t)(state_t *, inst_t *);

static func_t *funcs[] = {
    func_lb,
    func_lh,
    func_lw,
    func_ld,
    func_lbu,
    func_lhu,
    func_lwu,
    func_empty, // fence
    func_empty, // fence_i
    func_addi,
    func_slli,
    func_slti,
    func_sltiu,
    func_xori,
    func_srli,
    func_srai,
    func_ori,
    func_andi,
    func_auipc,
    func_addiw,
    func_slliw,
    func_srliw,
    func_sraiw,
    func_sb,
    func_sh,
    func_sw,
    func_sd,
    func_add,
    func_sll,
    func_slt,
    func_sltu,
    func_xor,
    func_srl,
    func_or,
    func_and,
    func_mul,
    func_mulh,
    func_mulhsu,
    func_mulhu,
    func_div,
    func_divu,
    func_rem,
    func_remu,
    func_sub,
    func_sra,
    func_lui,
    func_addw,
    func_sllw,
    func_srlw,
    func_mulw,
    func_divw,
    func_divuw,
    func_remw,
    func_remuw,
    func_subw,
    func_sraw,
    func_beq,
    func_bne,
    func_blt,
    func_bge,
    func_bltu,
    func_bgeu,
    func_jalr,
    func_jal,
    func_ecall,
    func_csrrw,
    func_csrrs,
    func_csrrc,
    func_csrrwi,
    func_csrrsi,
    func_csrrci
};

void exec_block_interp(state_t *state) {
    static inst_t inst = {0};
    while(true) {
        u32 data = *(u32 *)TO_HOST(state->pc);
        // printf("%x\n",data);
        inst_decode(&inst, data);

        funcs[inst.type](state, &inst);
        printf("pc:%lx rs1:%x pc:%lx cont:%d\n",state->pc,inst.rs1,state->reenter_pc,inst.cont?1:0);
        state->gpr[zero] = 0;

        if (inst.cont) break;

        state->pc += inst.rvc ? 2 : 4;
    }
}
