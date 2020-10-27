#ifndef CPU_H
#define CPU_H
constexpr int REG_NUMBER = 3;

enum CPU_COMMANDS {
    HLT = 0,
    ADD,
    SUB,
    MUL,
    DIV,
    SQRT,
    PUSH_REG = 30,
    PUSH_VAL,
    POP_REG,
    POP_VAL,
    IN = 60,
    IN_REG,
    OUT,
    OUT_REG,
    RAX = 100,
    RBX,
    RCX
};
#endif
