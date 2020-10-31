#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <cassert>
#include <stdlib.h>

#include "asm.h"
#include "cpu.h"
#include "in_and_out.h"


//! \brief Just a small func to make code more readable when need to write byte into file
//! \param [in] fd File to write in
//! \param [in] value Value to write into file
//! \return Returns true if byte was successfully written
static bool
write_to_file(int fd, int value) {
    char val_char = value;
    return write(fd, &val_char, 1) == 1;
}

static int
write_register_to_file(char *command) {
    if (!strncmp(command, RAX_STR, sizeof(RAX_STR) - 1)) {
        return RAX;
    }
    if (!strncmp(command, RBX_STR, sizeof(RBX_STR) - 1)) {
        return RBX;
    }
    if (!strncmp(command, RCX_STR, sizeof(RCX_STR) - 1)) {
        return RCX;
    }
    return 0;
}
//! \brief Main assembler function. Translates assembler commands to 'binary' code
//! \param [in] commands Assembler commands to translate
//! \param [in] commands_size Size of commands in bytes
//! \param [in] fd File descriptor to write result in
//! \return Returns true if no problems during translation appeared
static bool
translate_to_machine_code(char *commands, ssize_t commands_size, int fd) {
    assert(commands);
    assert(fd >= 0);
    assert(commands_size > 0);

    char *commands_end = commands + commands_size;
    int tmp_value = 0;
    while (commands < commands_end) {
        while (commands < commands_end && isspace((int)*commands)) commands++;
        if (commands >= commands_end) {
            return true; // EOF
        } 
        if (commands + sizeof(MUL_STR) <= commands_end && 
            !strncmp(commands, MUL_STR, sizeof(MUL_STR) - 1)) {
            write_to_file(fd, MUL);
            commands += sizeof(MUL_STR) - 1;
            continue;
        }
        if (commands + sizeof(DIV_STR) <= commands_end && 
            !strncmp(commands, DIV_STR, sizeof(DIV_STR) - 1)) {
            write_to_file(fd, DIV);
            commands += sizeof(DIV_STR) - 1;
            continue;
        }
        if (commands + sizeof(ADD_STR) <= commands_end && 
            !strncmp(commands, ADD_STR, sizeof(ADD_STR) - 1)) {
            write_to_file(fd, ADD);
            commands += sizeof(ADD_STR) - 1;
            continue;
        }
        if (commands + sizeof(SUB_STR) <= commands_end &&
            !strncmp(commands, SUB_STR, sizeof(SUB_STR) - 1)) {
            write_to_file(fd, SUB);
            commands += sizeof(SUB_STR) - 1;
            continue;
        }
        if (commands + sizeof(SQRT_STR) <= commands_end &&
            !strncmp(commands, SQRT_STR, sizeof(SQRT_STR) - 1)) {
            write_to_file(fd, SQRT);
            commands += sizeof(SQRT_STR) - 1;
            continue;
        }
        if (commands + sizeof(HLT_STR) <= commands_end &&
            !strncmp(commands, HLT_STR, sizeof(HLT_STR) - 1)) {
            write_to_file(fd, HLT);
            commands += sizeof(HLT_STR) - 1;
            continue;
        } 
        if (commands + sizeof(IN_STR) <= commands_end &&
            !strncmp(commands, IN_STR, sizeof(IN_STR) - 1)) {
            commands += sizeof(IN_STR) - 1;
            while (commands < commands_end && isspace((int)*commands)) commands++;
            if (commands >= commands_end) {
                write_to_file(fd, IN);
                return true;
            }
            if (commands + sizeof(RAX_STR) <= commands_end) {
                tmp_value = write_register_to_file(commands);
                if (tmp_value) {
                    write_to_file(fd, IN_REG);
                    write_to_file(fd, tmp_value);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }                
            }
            write_to_file(fd, IN);
            continue;
        }
        if (commands + sizeof(OUT_STR) <= commands_end &&
            !strncmp(commands, OUT_STR, sizeof(OUT_STR) - 1)) {
            commands += sizeof(OUT_STR) - 1;
            while (commands < commands_end && isspace((int)*commands)) commands++;
            if (commands >= commands_end) {
                write_to_file(fd, OUT);
                return true;
            }
            if (commands + sizeof(RAX_STR) <= commands_end) {
                tmp_value = write_register_to_file(commands); 
                if (tmp_value) {
                    write_to_file(fd, OUT_REG);
                    write_to_file(fd, tmp_value);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }
            }
            write_to_file(fd, OUT);
            continue;
        }
        if (commands + sizeof(PUSH_STR) <= commands_end &&
            !strncmp(commands, PUSH_STR, sizeof(PUSH_STR) - 1)) {
            commands += sizeof(PUSH_STR) - 1;
            while (commands < commands_end && isspace((int)*commands)) commands++;
            if (commands >= commands_end) {
                return false; // What we should push?
            }
            if (commands + sizeof(RAX_STR) <= commands_end) {
                tmp_value = write_register_to_file(commands);
                if (tmp_value) {
                    write_to_file(fd, PUSH_REG);
                    write_to_file(fd, tmp_value);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }
            }
            //if here, try to read double from commands
            //space symbols are processed by strtod, don`t need to update commands before
            char *endptr = NULL;
            errno = 0;
            double tmp_double = strtod(commands, &endptr);
            if (endptr == commands) {
            //double was not found
                fprintf(stderr, "Error in push commands: you need to push value or register\n");
                return false;
            }
            write_to_file(fd, PUSH_VAL);
            write(fd, &tmp_double, sizeof(double));
            commands = endptr;
            continue;
        }     
        if (commands + sizeof(POP_STR) <= commands_end &&
            !strncmp(commands, POP_STR, sizeof(POP_STR) - 1)) {
            commands += sizeof(POP_STR) - 1;
            while (commands < commands_end && isspace((int)*commands)) commands++;
            if (commands >= commands_end) {
                write_to_file(fd, POP_VAL);
                return true; //EOF
            }
            if (commands + sizeof(RAX_STR) <= commands_end) {
                tmp_value = write_register_to_file(commands);
                if (tmp_value) {
                    write_to_file(fd, POP_REG);
                    write_to_file(fd, tmp_value);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }
            }
            write_to_file(fd, POP_VAL);
        }
        return false; //Can not recognise commands
    }
    return true;
}

//! \brief Read commands and write result of the translation to files
//! \param [in] file_in File to read commands
//! \param [out] file_out File to write asm commands
//! \return Returns true if success, false else
bool
in_and_out_from_asm(char *file_in, char *file_out) {
    assert(file_in);
    assert(file_out);
    
    int file_in_size = 0;
    char *commands = mmap_file(file_in, &file_in_size);
    if (!commands) {
        return false;
    }
    int fd_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, out_mode); 
    if (fd_out < 0) {
        fprintf(stderr, "Error: Can`t open out file %s\n", file_out);
        munmap(commands, file_in_size);
        return false;
    }
    if (!translate_to_machine_code(commands, file_in_size, fd_out)) {
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
