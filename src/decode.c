#include "rvemu.h"

#define QUADRANT(data) (((data) >> 0) & 0x3) // 取最低两位 ->指令象限 ->判断是否是压缩指令


void inst_decode(inst_t *inst, u32 data) {
    u32 quadrant = QUADRANT(data);
    switch (quadrant) {
        case 0x0:fatal("unimplemented");
        case 0x1:fatal("unimplemented");
        case 0x2:fatal("unimplemented");
        case 0x3:fatal("unimplemented");
        default:unreachable();
    }
}