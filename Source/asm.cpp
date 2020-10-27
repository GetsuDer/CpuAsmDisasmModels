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


static bool
write_header(int fd) {
// TODO: Write this function
    assert(fd >= 0);
    return true;
}

static bool
write_to_file(int fd, int value) {
    char val_char = value;
    return write(fd, &val_char, 1) == 1;
}

static bool
translate_to_machine_code(char *commands, ssize_t commands_size, int fd) {
    assert(commands);
    assert(fd >= 0);
    assert(commands_size > 0);

    char *commands_end = commands + commands_size;
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
                if (!strncmp(commands, RAX_STR, sizeof(RAX_STR) - 1)) {
                    write_to_file(fd, IN_REG);
                    write_to_file(fd, RAX);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RBX_STR, sizeof(RBX_STR) - 1)) {
                    write_to_file(fd, IN_REG);
                    write_to_file(fd, RBX);
                    commands += sizeof(RBX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RCX_STR, sizeof(RCX_STR) - 1)) {
                    write_to_file(fd, IN_REG);
                    write_to_file(fd, RCX);
                    commands += sizeof(RCX_STR) - 1;
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
                if (!strncmp(commands, RAX_STR, sizeof(RAX_STR) - 1)) {
                    write_to_file(fd, OUT_REG);
                    write_to_file(fd, RAX);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RBX_STR, sizeof(RBX_STR) - 1)) {
                    write_to_file(fd, OUT_REG);
                    write_to_file(fd, RBX);
                    commands += sizeof(RBX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RCX_STR, sizeof(RCX_STR) - 1)) {
                    write_to_file(fd, OUT_REG);
                    write_to_file(fd, RCX);
                    commands += sizeof(RCX_STR) - 1;
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
                if (!strncmp(commands, RAX_STR, sizeof(RAX_STR) - 1)) {
                    write_to_file(fd, PUSH_REG);
                    write_to_file(fd, RAX);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RBX_STR, sizeof(RBX_STR) - 1)) {
                    write_to_file(fd, PUSH_REG);
                    write_to_file(fd, RBX);
                    commands += sizeof(RBX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RCX_STR, sizeof(RCX_STR) - 1)) {
                    write_to_file(fd, PUSH_REG);
                    write_to_file(fd, RCX);
                    commands += sizeof(RCX_STR) - 1;
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
                if (!strncmp(commands, RAX_STR, sizeof(RAX_STR) - 1)) {
                    write_to_file(fd, POP_REG);
                    write_to_file(fd, RAX);
                    commands += sizeof(RAX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RBX_STR, sizeof(RBX_STR) - 1)) {
                    write_to_file(fd, POP_REG);
                    write_to_file(fd, RBX);
                    commands += sizeof(RBX_STR) - 1;
                    continue;
                }
                if (!strncmp(commands, RCX_STR, sizeof(RCX_STR) - 1)) {
                    write_to_file(fd, POP_REG);
                    write_to_file(fd, RBX);
                    commands += sizeof(RCX_STR) - 1;
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

    //read the file size
    struct stat file_stat;
    errno = 0;
    if (stat(file_in, &file_stat)) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return false;
    }
    ssize_t file_in_size = file_stat.st_size;
    if (file_in_size <= 0) {
        fprintf(stderr, "Error: file %s has no data\n", file_in);
        return false;
    }
    // open file with source commands
    int fd_in = open(file_in, 0);
    if (fd_in < 0) {
        fprintf(stderr, "Error: Can`t open file %s\n", file_in);
        return false;
    }

    // mmap file
    char *commands = (char *)mmap(NULL, file_in_size, PROT_READ, MAP_SHARED, fd_in, 0);
    if (!commands) {
        fprintf(stderr, "Error: Can`t mmap file %s\n", file_in);
        return false;
    }
    int fd_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, out_mode); 
    if (fd_out < 0) {
        fprintf(stderr, "Error: Can`t open out file %s\n", file_out);
        close(fd_in);
        munmap(commands, file_in_size);
        return false;
    }
    if (!write_header(fd_out)) {
        fprintf(stderr, "Error: Can`t write header\n");
        close(fd_out);
        close(fd_in);
        munmap(commands, file_in_size);
        return false;
    }
    if (!translate_to_machine_code(commands, file_in_size, fd_out)) {
        fprintf(stderr, "Error: Can`t translate to asm\n");
        close(fd_out);
        close(fd_in);
        munmap(commands, file_in_size);
        return false;
    }
    
    //Don`t forget to unmap input file after using
    munmap(commands, file_in_size);
    // And close file descriptors (no error checking, because the main result is
    // Already gotten
    close(fd_out);
    close(fd_in);
    return true;
}
