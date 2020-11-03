#ifndef CPU_H
#define CPU_H
struct Cpu
{
    int state;
    struct Stack_double *cpu_stack;
    double rax;
    double rbx;
    double rcx;
};

constexpr int REG_NUMBER = 3;
constexpr double ZERO_EPS = 1e-6;
enum CPU_STATES {
    OFF = 0,
    ON,
    WAIT
};

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
    RCX,
    JMP
};

bool turn_cpu_on(Cpu *cpu);
bool work(char *commands, int commands_size, Cpu *cpu);
void init(Cpu *cpu);
#endif
