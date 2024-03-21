#include "rvemu.h"

static void func_empty(state_t *state, inst_t *inst) {}

typedef void (func_t)(state_t *, inst_t *);

static func_t *funcs[] = {
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty,
    func_empty
};

void exec_block_interp(state_t *state) {
    static inst_t inst = {0};
    while(true) {
        u32 data = *(u32 *)TO_HOST(state->pc);
        printf("%x\n",data);
        inst_decode(&inst, data);

        funcs[inst.type](state, &inst);
        state->gpr[zero] = 0;

        if (inst.cont) break;

        state->pc += inst.rvc ? 2 : 4;
    }
}
