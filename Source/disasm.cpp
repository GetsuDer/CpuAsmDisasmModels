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

//! \bried Read beginning of the code file and check header
//! \in [file] Mmaped file
static int
read_header(char *file) 
{
    assert(file);
//TODO: write this function
    int header_size = 0;
    return header_size;
}

static bool
write_register(char command, int fd) {
    switch(command) {
        case RAX:
            write(fd, RAX_STR, sizeof(RAX_STR));
            break;
        case RBX:
            write(fd, RBX_STR, sizeof(RBX_STR));
            break;
        case RCX:
            write(fd, RCX_STR, sizeof(RCX_STR));
            break;
        default:
            return false;
    }
    write(fd, "\n", 1);
    return true;
}

static bool
translate_to_asm(char *commands, int commands_size, int fd)
{
    assert(commands);
    assert(fd > 0);
    assert(commands_size > 0);
    
    char *commands_end = commands + commands_size;
    double tmp_double = 0;
    while (commands < commands_end) {
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
                    fprintf(stderr, "Error: push command without register argument\n");
                    return false;
                }
                if (!write_register(*commands, fd)) {
                    fprintf(stderr, "Error: push commands without valid register\n");
                    return false;
                }
                break; 
            case PUSH_VAL:
                write(fd, PUSH_STR, sizeof(PUSH_STR) - 1);
                commands++;
                if (commands + sizeof(double) > commands_end) {
                    fprintf(stderr, "Error: no value argument\n");
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
                    fprintf(stderr, "Error: no valid register in pop command\n");
                    return false;          
                }
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
                    fprintf(stderr, "Error: no valid register in in command\n");
                    return false;
                }
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
                    fprintf(stderr, "Error: no valid register name in out command\n");
                    return false;
                }
                break;
            default:
                fprintf(stderr, "Error: can not recognise command\n");
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
    int header_size = read_header(commands);
    if (header_size < 0) {
        fprintf(stderr, "Error: Can`t read header\n");
        close(fd_out);
        close(fd_in);
        munmap(commands, file_in_size);
        return false;
    }
    if (!translate_to_asm(commands + header_size, file_in_size, fd_out)) {
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


