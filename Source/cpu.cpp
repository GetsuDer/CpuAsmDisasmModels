#include <string.h>
#include <cstdint>

#define TYPE double
#include "Stack.h"

#include "cpu.h"

//! \brief Init cpu into void state (OFF)
//! \param [in] cpu CPU to be inited
void
init(struct Cpu *cpu)
{
    assert(cpu);
    cpu->state = OFF;
    cpu->cpu_stack = (Stack_double *)calloc(1, sizeof(*cpu->cpu_stack));
    cpu->rax = 0;
    cpu->rbx = 0;
    cpu->rcx = 0;
}

//! \brief Change CPU state and initialize stack, if necessary
//! \param[in] cpu Pointer to CPU
//! \return True if success, False else
bool
turn_cpu_on(struct Cpu *cpu)
{
    switch (cpu->state) {
        case WAIT:
        case ON:
            return true;
        case OFF:

            STACK_INIT((*cpu->cpu_stack));
            cpu->state = ON;
            return true;

        default:
            fprintf(stderr, "Unknown CPU state - %d\n", cpu->state);
            return false;
    }
    return false;
};



//! \brief Often need to check, if cpu stack has enough arguments
//! \param[in] cpu Pointer to cpu to check
//! \param[in] argn Necessary number of arguments on stack
//! \return True if cpu has enough values on stack, false else
static bool
check_arg_num(struct Cpu *cpu, int argn)
{
    if (Stack_Size(cpu->cpu_stack) < argn) {
        fprintf(stderr, "CPU error: not enough arguments on stack\n");
        cpu->state = WAIT;
        return false;
    }
    return true;
}

//! \brief Function to hide register swicth
//! \param [in] cpu Cpu to choose register
//! \param [in] command Command, which specifies register
//! \return Returns pointer to necessary register or NULL
static double*
find_register(struct Cpu *cpu, char *command)
{
    switch (*command) {
        case RAX:
            return &(cpu->rax);
        case RBX:
            return &(cpu->rbx);
        case RCX:
            return &(cpu->rcx);
        default:
            return NULL;
    }
    return NULL;
}

//! \brief Take one or two top values from cpu stack
//! \param [in] cpu Cpu to work with
//! \param [out] tmp1 First value
//! \param [out] tmp2 Second value
static void
take_from_cpu_stack(struct Cpu *cpu, double *tmp1, double *tmp2)
{
    *tmp1 = Stack_Top(cpu->cpu_stack);
    Stack_Pop(cpu->cpu_stack);
    if (tmp2) {
        *tmp2 = Stack_Top(cpu->cpu_stack);
        Stack_Pop(cpu->cpu_stack);
    }
    return;
}

//! \brief Proccess comands from buffer
//! \param[in] commands Buffer with commands
//! \param[in] commands_size Commands buffer size
//! \param[in] cpu Pointer to cpu which will process commands
//! \return Return true, if no errors during execution
bool
work(char *commands, int commands_size, struct Cpu *cpu)
{
    assert(commands);
    assert(commands_size > 0);
    assert(cpu);
    
    if (cpu->state != ON) {
        turn_cpu_on(cpu);
    }
    char *commands_begin = commands;
    char *commands_end = commands + commands_size;
    double tmp_double1 = 0, tmp_double2 = 0;
    double *tmp_register = NULL;
    int address = 0;
    while (commands < commands_end) {
        switch(*commands) {
            case HLT:
                commands++;
                cpu->state = WAIT;
                //CPU was stopped. Just stop working on commands
                return true;
            case ADD:
                if (!check_arg_num(cpu, 2)) return false;
                commands++;
                take_from_cpu_stack(cpu, &tmp_double1, &tmp_double2);
                Stack_Push(cpu->cpu_stack, tmp_double1 + tmp_double2);
                break;
            case SUB: 
                if (!check_arg_num(cpu, 2)) return false;
                commands++;
                take_from_cpu_stack(cpu, &tmp_double1, &tmp_double2);
                Stack_Push(cpu->cpu_stack, tmp_double2 - tmp_double1);
                break;
            case MUL:
                if (!check_arg_num(cpu, 2)) return false;
                commands++;
                take_from_cpu_stack(cpu, &tmp_double1, &tmp_double2);
                Stack_Push(cpu->cpu_stack, tmp_double1 * tmp_double2);
                break;
            case DIV:
                if (!check_arg_num(cpu, 2)) return false;
                commands++;
                take_from_cpu_stack(cpu, &tmp_double1, &tmp_double2);
                if (fabs(tmp_double2) < ZERO_EPS) {
                    fprintf(stderr, "CPU error: zero division\n");
                    cpu->state = WAIT;
                    return false;
                }
                Stack_Push(cpu->cpu_stack, tmp_double2 / tmp_double1);
                break;
            case SQRT:
                if (!check_arg_num(cpu, 1)) return false;
                commands++;
                take_from_cpu_stack(cpu, &tmp_double1, NULL);
                if (tmp_double1 < 0) {
                    fprintf(stderr, "CPU error: sqrt from negative value\n");
                    cpu->state = WAIT;
                    return false;
                }
                Stack_Push(cpu->cpu_stack, sqrt(tmp_double1));
                break;
            case PUSH_REG:
                commands++;
                if (commands >= commands_end) {
                    fprintf(stderr, "CPU error: no register\n");
                    cpu->state = WAIT;
                    return false;
                }
                tmp_register = find_register(cpu, commands);
                if (!tmp_register) {
                    cpu->state = WAIT;
                    fprintf(stderr, "No valid register in push command\n");
                    return false;
                }
                Stack_Push(cpu->cpu_stack, *tmp_register);
                commands++;
                break;
            case PUSH_VAL:
                commands++;
                if (commands + sizeof(double) >= commands_end) {
                    cpu->state = WAIT;
                    fprintf(stderr, "CPU error: no valid argument\n");
                    return false;
                }
                memcpy(&tmp_double1, commands, sizeof(double));
                commands += sizeof(double);
                Stack_Push(cpu->cpu_stack, tmp_double1);
                break;
            case POP_VAL:
                commands++;
                if (!check_arg_num(cpu, 1)) {
                    cpu->state = WAIT;
                    fprintf(stderr, "CPU error: pop from empty stack\n");
                    return false;
                }
                Stack_Pop(cpu->cpu_stack);
                break;
            case POP_REG:
                commands++;
                if (!check_arg_num(cpu, 1)) {
                    cpu->state = WAIT;
                    fprintf(stderr, "CPU error: pop from empty stack\n");
                    return false;
                }
                take_from_cpu_stack(cpu, &tmp_double1, NULL);
                if (commands >= commands_end) {
                    cpu->state = WAIT;
                    fprintf(stderr, "CPU error: no register name\n");
                    return false;
                }
                tmp_register = find_register(cpu, commands);
                if (!tmp_register) {
                    cpu->state = WAIT;
                    fprintf(stderr, "Unknown register\n");
                    return false;
                }
                *tmp_register = tmp_double1;
                commands++;
                break;
            case IN:
                if (fscanf(stdin, "%lf", &tmp_double1) != 1) {
                    fprintf(stderr, "Input error: can not get value\n");
                    cpu->state = WAIT;
                    return false;
                }
                Stack_Push(cpu->cpu_stack, tmp_double1);
                commands++;
                break;
            case IN_REG:
                if (fscanf(stdin, "%lf", &tmp_double1) != 1) {
                    fprintf(stderr, "Input error: can not get value\n");
                    cpu->state = WAIT;
                    return false;
                }
                commands++;
                if (commands >= commands_end) {
                    fprintf(stderr, "CPU error: no input register\n");
                    cpu->state = WAIT;
                    return false;
                }
                tmp_register = find_register(cpu, commands);
                if (!tmp_register) {
                    fprintf(stderr, "Invalid register name\n");
                    cpu->state = WAIT;
                    return false;
                }
                *tmp_register = tmp_double1;
                commands++;
                break;
            case OUT:
                if (!check_arg_num(cpu, 1)) {
                    fprintf(stderr, "CPU error: empty stack\n");
                    cpu->state = WAIT;
                    return false;
                }
                fprintf(stdout, "%lf\n", Stack_Top(cpu->cpu_stack));
                Stack_Pop(cpu->cpu_stack);
                commands++;
                break;
            case OUT_REG:
                commands++;
                if (commands >= commands_end) {
                    fprintf(stderr, "CPU error: no register name\n");
                    cpu->state = WAIT;
                    return false;
                }
                tmp_register = find_register(cpu, commands);
                if (!tmp_register) {
                    fprintf(stderr, "Invalid register name\n");
                    cpu->state = WAIT;
                    return false;
                }
                fprintf(stdout, "%lf\n", *tmp_register);
                commands++;
                break;
            case JMP:
                commands++;
                if (commands == commands_end) {
                    cpu->state = WAIT;
                    return false;
                }
                address = *(int *)commands;
                commands = commands_begin + address;
                break;
            case JMPL:
                commands++;
                if (!check_arg_num(cpu, 2)) {
                    fprintf(stderr, "jmpl command when less then 2 elements in stack!");
                    cpu->state = WAIT;
                    return false;
                }
                take_from_cpu_stack(cpu, &tmp_double1, &tmp_double2);
                if (tmp_double2 < tmp_double1) { //jmp
                    address = *(int *)commands;
                    commands = commands_begin + address;
                } else {
                    commands += sizeof(address);
                }
                break;

            case JMPG:
                commands++;
                if (!check_arg_num(cpu, 2)) {
                    fprintf(stderr, "jmpg command when less then 2 elements in stack!\n");
                    cpu->state = WAIT;
                    return false;
                }
                take_from_cpu_stack(cpu, &tmp_double1, &tmp_double2);
                if (tmp_double2 > tmp_double1) { //jmp
                    address = *(int *)commands;
                    commands = commands_begin + address;
                } else {
                    commands += sizeof(address);
                }
                break;
            default:
                fprintf(stderr, "CPU error: wrong commands\n");
                cpu->state = WAIT;
                return false;
        }
    }
    return true;
}

