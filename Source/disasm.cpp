#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>

#include "asm.h"
#include "cpu.h"
#include "disasm.h"
#include "in_and_out.h"


//! \brief Small func to make code looks better. Write specified register.
//! \param [in] command Command which specifies register
//! \param [in] fd File descriptor to write result (rax, rbx or rcx)
//! \return Returns true if register command was valid and successfully written
static bool
write_register(char command, int fd) {
    switch(command) {
        case RAX:
            if (write(fd, RAX_STR, sizeof(RAX_STR) - 1) == -1) return false;
            break;
        case RBX:
            if (write(fd, RBX_STR, sizeof(RBX_STR) - 1) == -1) return false;
            break;
        case RCX:
            if (write(fd, RCX_STR, sizeof(RCX_STR) - 1) == -1) return false;
            break;
        default:
            return false;
    }
    write(fd, "\n", 1);
    return true;
}

//! \brief Main disassembler function. Translates command bytes into assembler commands.
//! \param [in] commands Command bytes
//! \param [in] commands_size Command bytes len
//! \param [in] fd File descriptor to write result in
//! \return Returns true if no problems during execution were.
static bool
translate_to_asm(char *commands, int commands_size, int fd)
{
    assert(commands);
    assert(fd > 0);
    assert(commands_size > 0);
#ifdef DEBUG_NUMERATION    
    char *commands_begin = commands;
#endif
    char *commands_end = commands + commands_size;
    double tmp_double = 0;
    int addr = 0;
    while (commands < commands_end) {
#ifdef DEBUG_NUMERATION
        dprintf(fd, "%ld : ", commands - commands_begin);
#endif
        switch (*commands) {
            case HLT:
                write(fd, HLT_STR, sizeof(HLT_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case MUL:
                write(fd, MUL_STR, sizeof(MUL_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case DIV:
                write(fd, DIV_STR, sizeof(DIV_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case ADD:
                write(fd, ADD_STR, sizeof(ADD_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case SUB:
                write(fd, SUB_STR, sizeof(SUB_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case SQRT:
                write(fd, SQRT_STR, sizeof(SQRT_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case PUSH_REG:
                write(fd, PUSH_STR, sizeof(PUSH_STR) - 1);
                write(fd, " ", 1);
                commands++;
                if (commands >= commands_end) {
                    fprintf(stderr, "Error: push command without register argument at the end of the file\n");
                    return false;
                }
                if (!write_register(*commands, fd)) {
                    fprintf(stderr, "Error: push commands without valid register in %10s\n", commands);
                    return false;
                }
                commands++;
                break; 
            case PUSH_VAL:
                write(fd, PUSH_STR, sizeof(PUSH_STR) - 1);
                commands++;
                if (commands + sizeof(double) > commands_end) {
                    fprintf(stderr, "Error: no value argument for push commands %10s\n", commands);
                    return false;
                }
                tmp_double = 0;
                memcpy(&tmp_double, commands, sizeof(double));
                dprintf(fd, " %lf\n", tmp_double);
                commands += sizeof(double);
                break;
            case POP_VAL:
                write(fd, POP_STR, sizeof(POP_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case POP_REG:
                write(fd, POP_STR, sizeof(POP_STR) - 1);
                write(fd, " ", 1);
                commands++;
                if (commands >= commands_end) {
                    fprintf(stderr, "Error: no register in pop command\n");
                    return false;
                }
                if (!write_register(*commands, fd)) {
                    fprintf(stderr, "Error: no valid register in pop command %10s\n", commands);
                    return false;          
                }
                commands++;
                break;
            case IN:
                write(fd, IN_STR, sizeof(IN_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case IN_REG:
                write(fd, IN_STR, sizeof(IN_STR) - 1);
                write(fd, " ", 1);
                commands++;
                if (commands >= commands_end) {
                    fprintf(stderr, "Error: no register in in command\n");
                    return false;
                }
                if (!write_register(*commands, fd)) {
                    fprintf(stderr, "Error: no valid register in in command %10s\n", commands);
                    return false;
                }
                commands++;
                break;
            case OUT:
                write(fd, OUT_STR, sizeof(OUT_STR) - 1);
                write(fd, "\n", 1);
                commands++;
                break;
            case OUT_REG:
                write(fd, OUT_STR, sizeof(OUT_STR) - 1);
                write(fd, " ", 1);
                commands++;
                if (commands >= commands_end) {
                    fprintf(stderr, "Error: no register name in out command\n");
                    return false;
                }
                if (!write_register(*commands, fd)) {
                    fprintf(stderr, "Error: no valid register name in out command %10s\n", commands);
                    return false;
                }
                commands++;
                break;
            case JMP:
                write(fd, JMP_STR, sizeof(JMP_STR) - 1);
                commands++;
                addr = *commands;
                commands += sizeof(addr);
                write(fd, " ", 1);
                dprintf(fd, "%d\n", addr);
                break; 
            default:
                fprintf(stderr, "Error: can not recognise command %10s\n", commands);
                commands++;
                break;
        }
    }
    return true;
}

//! \brief Read 'binary' code and write result of the translation to files
//! \param [in] file_in File to read 'binary' code
//! \param [out] file_out File to write asm commands
//! \return Returns true if success, false else
bool
in_and_out_from_binary(char *file_in, char *file_out) 
{
    assert(file_in);
    assert(file_out);

    int file_in_size = 0;
    char *commands = mmap_file(file_in, &file_in_size);
    
    int fd_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, out_mode);
    if (fd_out < 0) {
        fprintf(stderr, "Error: Can`t open out file %s\n", file_out);
        munmap(commands, file_in_size);
        return false;
    }
    if (!translate_to_asm(commands, file_in_size, fd_out)) {
        fprintf(stderr, "Error: Can`t translate to asm\n");
        close(fd_out);
        munmap(commands, file_in_size);
        return false;
    }

    //Don`t forget to unmap input file after using
    munmap(commands, file_in_size);
    // And close file descriptors (no error checking, because the main result is
    // Already gotten
    close(fd_out);
    return true;
}


