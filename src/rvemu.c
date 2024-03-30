#include "rvemu.h"

int main(int argc, char *argv[]){
    // printf("Hello, World\n");
    assert(argc > 1);

    machine_t machine = {0};
    machine_load_program(&machine, argv[1]);
    machine_setup(&machine, argc, argv);

    printf("entry: %lx\n",machine.mmu.entry);
    printf("host entry: %llx\n",TO_HOST(machine.mmu.entry));
    printf("host alloc: %lx\n",machine.mmu.host_alloc);
    printf("machine address: 0x%lx\n", (u64)&machine);

    while (true) {
        enum exit_reason_t reason = machine_step(&machine);
        assert(reason == ecall);

        //handle ecall
        // printf("\n");
        // fatal("syscall!!");
        u64 syscall = machine_get_gpr(&machine, a7);
        // fatalf("syscall :%ld",syscall);
        u64 ret = do_syscall(&machine, syscall);
        machine_set_gpr(&machine, a0, ret);
    }
    return 0;
}