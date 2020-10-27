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

    char *commands_end = commands + commands_size;
    double tmp_double1 = 0, tmp_double2 = 0;
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
                tmp_double1 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                tmp_double2 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                Stack_Push(cpu->cpu_stack, tmp_double1 + tmp_double2);
                break;
            case SUB: 
                if (!check_arg_num(cpu, 2)) return false;
                commands++;
                tmp_double1 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                tmp_double2 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                Stack_Push(cpu->cpu_stack, tmp_double2 - tmp_double1);
                break;
            case MUL:
                if (!check_arg_num(cpu, 2)) return false;
                commands++;
                tmp_double1 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                tmp_double2 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                Stack_Push(cpu->cpu_stack, tmp_double1 * tmp_double2);
                break;
            case DIV:
                if (!check_arg_num(cpu, 2)) return false;
                commands++;
                tmp_double1 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                tmp_double2 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
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
                tmp_double1 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
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
                switch(*commands) {
                    case RAX:
                        Stack_Push(cpu->cpu_stack, cpu->rax);
                        break;
                    case RBX:
                        Stack_Push(cpu->cpu_stack, cpu->rbx);
                        break;
                    case RCX:
                        Stack_Push(cpu->cpu_stack, cpu->rcx);
                        break;
                    default:
                        cpu->state = WAIT;
                        fprintf(stderr, "CPU error: no valid register\n");
                        return false;
                }
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
                tmp_double1 = Stack_Top(cpu->cpu_stack);
                Stack_Pop(cpu->cpu_stack);
                if (commands >= commands_end) {
                    cpu->state = WAIT;
                    fprintf(stderr, "CPU error: no register name\n");
                    return false;
                }
                switch (*commands) {
                    case RAX:
                        cpu->rax = tmp_double1;
                        break;
                    case RBX: 
                        cpu->rbx = tmp_double1;
                        break;
                    case RCX:
                        cpu->rcx = tmp_double1;
                        break;
                    default:
                        cpu->state = WAIT;
                        fprintf(stderr, "CPU error: unknown register\n");
                        return false;
                }
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
                switch (*commands) {
                    case RAX:
                        cpu->rax = tmp_double1;
                        break;
                    case RBX:
                        cpu->rbx = tmp_double1;
                        break;
                    case RCX:
                        cpu->rcx = tmp_double1;
                        break;
                    default:
                        fprintf(stderr, "CPU error: invalid register name\n");
                        cpu->state = WAIT;
                        return false;
                }
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
                switch (*commands) {
                    case RAX:
                        fprintf(stdout, "%lf", cpu->rax);
                        break;
                    case RBX:
                        fprintf(stdout, "%lf", cpu->rbx);
                        break;
                    case RCX:
                        fprintf(stdout, "%lf", cpu->rcx);
                        break;
                    default:
                        fprintf(stderr, "CPU error: invalid register name\n");
                        cpu->state = WAIT;
                        return false;
                }
                commands++;
                break;
            default:
                fprintf(stderr, "CPU error: wrong commands\n");
                cpu->state = WAIT;
                return false;
        }
    }
    return true;
}

