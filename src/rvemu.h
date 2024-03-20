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
    inst_addi,
    num_insts,
};

typedef struct {
    i8 rd;
    i8 rs1;
    i8 rs2;
    i32 imm;
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