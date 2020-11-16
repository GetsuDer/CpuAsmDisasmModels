#ifndef CPU_MAIN_H
#define CPU_MAIN_H
constexpr int ARG_NUM = 2;
constexpr int FILE_IN = 1;
bool work(char *commands, int commands_size, Cpu *cpu, Memory_Controller *mc);
#endif
