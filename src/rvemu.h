#include <assert.h> // 引入assert
#include <errno.h>  // 
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "types.h"
#include "elfdef.h"
#include "reg.h"

#define todo(msg) (fprintf(stderr, "warning: %s:%d [TODO] %s\n", __FILE__, __LINE__, msg))
#define fatalf(fmt, ...) (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), exit(1))
#define fatal(msg) fatalf("%s", msg)
#define unreachable() (fatal("unreachable"), __builtin_unreachable())//用于提示编译器有一些分支不可达，让编译器可以做一些优化

#define ROUNDDOWN(x, k) ((x) & -(k)) // 返回最接近vaddr 且小于等于vaddr的page_size的倍数的地址
#define ROUNDUP(x, k)   (((x) + (k)-1) & -(k))
#define MIN(x, y)       ((y) > (x) ? (x) : (y))
#define MAX(x, y)       ((y) < (x) ? (x) : (y))

#define ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))


#define GUEST_MEMORY_OFFSET 0x088800000000ULL

#define TO_HOST(addr)  (addr + GUEST_MEMORY_OFFSET)
#define TO_GUEST(addr) (addr - GUEST_MEMORY_OFFSET)

enum inst_type_t {
    inst_lb, inst_lh, inst_lw, inst_ld, inst_lbu, inst_lhu, inst_lwu,
    inst_fence, inst_fence_i,
    inst_addi, inst_slli, inst_slti, inst_sltiu, inst_xori, inst_srli, inst_srai, inst_ori, inst_andi, inst_auipc, inst_addiw, inst_slliw, inst_srliw, inst_sraiw,
    inst_sb, inst_sh, inst_sw, inst_sd,
    inst_add, inst_sll, inst_slt, inst_sltu, inst_xor, inst_srl, inst_or, inst_and,
    inst_mul, inst_mulh, inst_mulhsu, inst_mulhu, inst_div, inst_divu, inst_rem, inst_remu,
    inst_sub, inst_sra, inst_lui,
    inst_addw, inst_sllw, inst_srlw, inst_mulw, inst_divw, inst_divuw, inst_remw, inst_remuw, inst_subw, inst_sraw,
    inst_beq, inst_bne, inst_blt, inst_bge, inst_bltu, inst_bgeu,
    inst_jalr, inst_jal, inst_ecall,
    inst_csrrc, inst_csrrci, inst_csrrs, inst_csrrsi, inst_csrrw, inst_csrrwi,
    // inst_flw, inst_fsw,
    // inst_fmadd_s, inst_fmsub_s, inst_fnmsub_s, inst_fnmadd_s, inst_fadd_s, inst_fsub_s, inst_fmul_s, inst_fdiv_s, inst_fsqrt_s,
    // inst_fsgnj_s, inst_fsgnjn_s, inst_fsgnjx_s,
    // inst_fmin_s, inst_fmax_s,
    // inst_fcvt_w_s, inst_fcvt_wu_s, inst_fmv_x_w,
    // inst_feq_s, inst_flt_s, inst_fle_s, inst_fclass_s,
    // inst_fcvt_s_w, inst_fcvt_s_wu, inst_fmv_w_x, inst_fcvt_l_s, inst_fcvt_lu_s,
    // inst_fcvt_s_l, inst_fcvt_s_lu,
    // inst_fld, inst_fsd,
    // inst_fmadd_d, inst_fmsub_d, inst_fnmsub_d, inst_fnmadd_d,
    // inst_fadd_d, inst_fsub_d, inst_fmul_d, inst_fdiv_d, inst_fsqrt_d,
    // inst_fsgnj_d, inst_fsgnjn_d, inst_fsgnjx_d,
    // inst_fmin_d, inst_fmax_d,
    // inst_fcvt_s_d, inst_fcvt_d_s,
    // inst_feq_d, inst_flt_d, inst_fle_d, inst_fclass_d,
    // inst_fcvt_w_d, inst_fcvt_wu_d, inst_fcvt_d_w, inst_fcvt_d_wu,
    // inst_fcvt_l_d, inst_fcvt_lu_d,
    // inst_fmv_x_d, inst_fcvt_d_l, inst_fcvt_d_lu, inst_fmv_d_x,
    num_insts,
};

typedef struct {
    i8 rd;
    i8 rs1;
    i8 rs2;
    i8 rs3;
    i32 imm;
    i16 csr;
    enum inst_type_t type;
    bool rvc;
    bool cont; // continue
} inst_t;


/* 
mmu.c
 */
typedef struct {
    u64 entry;
    u64 host_alloc;
    u64 alloc;
    u64 base;
} mmu_t;

void mmu_load_elf(mmu_t *, int );


/* 
state.c
 */
enum exit_reason_t {
    none,
    direct_branch,
    indirect_branch,
    ecall,
};

typedef struct {
    enum exit_reason_t exit_reason;
    u64 gpr[32];
    u64 pc;
} state_t;


/* 
machine.c
 */
typedef struct {
    state_t state;
    mmu_t mmu;
} machine_t;

enum exit_reason_t machine_step(machine_t *);
void machine_load_program(machine_t *, char *);

/* 
interp.c
 */
void exec_block_interp(state_t *);

/* 
decode.c
 */

void inst_decode(inst_t *, u32);