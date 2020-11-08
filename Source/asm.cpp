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

#define TYPE int
#include "Stack.h"

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

static void
init_sym_tab(struct Symtab *sym_tab)
{
    assert(sym_tab);
    sym_tab->size = 0;
    sym_tab->symbols = NULL;
}

//! \brief Find symbol in symbolic table
//! \param [in] sym_tab pointer to sym_tab
//! \param [in] name Symbol name
//! \param [in] name_size Symbol name size without \0
//! \return Return index in symbolic table, or -1, if symbol wasn`t found
int
find_symbol(struct Symtab *sym_tab, char *name, int name_size)
{
    assert(sym_tab);
    assert(name);
    for (int i = 0; i < sym_tab->size; i++) {
        if (sym_tab->symbols[i].name_size - 1 == name_size) {
            if (!strncmp(sym_tab->symbols[i].name, name, name_size)) {
        //founded
                return i;
            }
        }
    }
    return -1;
}


//! \brief Insert symbol into symbol table
//! \param[in] sym_tab Symbol table
//! \param[in] name Symbol name without \0
//! \param[in] name_size Symbol name size (without \0)
bool
add_symbol(struct Symtab *sym_tab, char *name, int name_size)
{
    assert(sym_tab);
    assert(name);
    if (sym_tab->size > 0) {
        struct Symbol *tmp = (struct Symbol *)realloc(sym_tab->symbols, sizeof(struct Symbol) * (sym_tab->size + 1));
        if (!tmp) {
            return false;
        }
        sym_tab->symbols = tmp;
    } else {
        sym_tab->symbols = (struct Symbol *)calloc(1, sizeof(struct Symbol));
        if (!sym_tab->symbols) {
            return false;
        }
    }
    sym_tab->symbols[sym_tab->size].name = (char *)calloc(1, name_size + 1);
    strncpy(sym_tab->symbols[sym_tab->size].name, name, name_size);
    sym_tab->symbols[sym_tab->size].name[name_size] = 0;
    sym_tab->symbols[sym_tab->size].name_size = name_size + 1;
    sym_tab->size++;
    sym_tab->symbols[sym_tab->size - 1].address = -1;
    return true;
}


//! \brief Insert address value for symbol into symtab
//! \param [in] sym_tab Pointer on symbol table
//! \param [in] name Symbol name
//! \param [in] name_size Symbol name size
//! \param [in] address Address for symbol
bool
add_symbol_address(struct Symtab *sym_tab, char *name, int name_size, int address)
{
    assert(sym_tab);
    assert(name);
    assert(name_size > 0);

    int id = find_symbol(sym_tab, name, name_size);
    if (id < 0) {
        return false;
    }
    sym_tab->symbols[id].address = address;
    return true;
}

//! \brief Recognise alone ssembler command and translate it to 'machine' code
//! \param [in] coms Commands to be recognised
//! \param [in] coms_end Pointer after last valid commands position
//! \param [in] fd File descriptor to write command
//! \param [out] sh_coms Shift after processing command (if founded)
//! \param [out] sh_add Size of machine code of written command
//! \param [in] com_size Size of the command str (with \0 symbol)
//! \param [in] com Command to be written
//! \return Returns true if the command was recognised 
bool
process_alone_command(char *coms, char *coms_end, int fd, const char *com_str, int com_size, int com, char **sh_coms, int *sh_add) {
    if (coms + com_size > coms_end || 
            strncmp(coms, com_str, com_size - 1)) {
        return false;
    }
    if (coms + com_size == coms_end || 
        isspace(*(coms + com_size))) {
        *sh_coms += com_size;
        *sh_add += 1;
        write_to_file(fd, com);
        return true;
    }
    return false;
}


//! \brief Recognise assembler command with register parameter and translate it to 'machine' code
//! \param [in] coms Commands to be recognised
//! \param [in] coms_end Pointer after last valid commands position
//! \param [in] fd File descriptor to write command
//! \param [out] sh_coms Shift after processing command (if founded)
//! \param [out] sh_add Size of machine code of written command
//! \param [in] com_size Size of the command str (with \0 symbol)
//! \param [in] com Command to be written
//! \return Returns true if the command was recognised 
bool
process_register_command(char *coms, char *coms_end, int fd, const char *com_str, int com_size, int com, char **sh_coms, int *sh_add) {
    if (coms + com_size >= coms_end || 
            strncmp(coms, com_str, com_size - 1) || !isspace(*(coms + com_size))) {
        return false;
    }
    char *old_coms = coms;
    coms += com_size + 1; //command + space after command
    skip_nonimportant_symbols(&coms, coms_end);
    if (coms >= coms_end) {
        //there is no register here
        return false;
    }

    int reg = write_register_to_file(coms);
    if (!reg) {
        //there is no register here
        return false;
    }
    write_to_file(fd, com);
    write_to_file(fd, reg);
    *sh_coms += (coms + sizeof(RAX_STR) - old_coms);
    *sh_add += 2; //command and register;
    return true;
}

//! \brief Recognise assembler command with value parameter and translate it to 'machine' code
//! \param [in] coms Commands to be recognised
//! \param [in] coms_end Pointer after last valid commands position
//! \param [in] fd File descriptor to write command
//! \param [out] sh_coms Shift after processing command (if founded)
//! \param [out] sh_add Size of machine code of written command
//! \param [in] com_size Size of the command str (with \0 symbol)
//! \param [in] com Command to be written
//! \return Returns true if the command was recognised 
bool
process_value_command(char *coms, char *coms_end, int fd, const char *com_str, int com_size, int com, char **sh_coms, int *sh_add) {
    if (coms + com_size >= coms_end || 
            strncmp(coms, com_str, com_size - 1) || !isspace(*(coms + com_size))) {
        return false;
    }
    coms += com_size;
    skip_nonimportant_symbols(&coms, coms_end);
    char *endptr = NULL;
    errno = 0;
    double tmp_double = strtod(coms, &endptr);
    if (errno || endptr == coms) {
        return false;
    }
     
    write_to_file(fd, com);
    write(fd, &tmp_double, sizeof(double));
    *sh_add += 1 + sizeof(double);
    *sh_coms = endptr; //this is not an error!!! (i hope) 
    return true;
}


//! \brief Skip comment and space symbols
//! \param [in,out] Assembler commands
//! \param [in] End of assembler commands
//! \return Moves commands to right position
void
skip_nonimportant_symbols(char **commands, char *commands_end)
{
    char *tmp_commands = *commands;
    while (true) {
        while (tmp_commands < commands_end && isspace(*tmp_commands)) tmp_commands++;
        if (tmp_commands >= commands_end) {
            *commands = commands_end;
            return;
        }
        if (*tmp_commands == '#') { //comment
            tmp_commands++;
            while (tmp_commands < commands_end && !(*tmp_commands == '#')) tmp_commands++;
            if (tmp_commands >= commands_end) {
                fprintf(stderr, "Not ended comment: %20s\n", *commands);
                *commands = tmp_commands;
                return;
            }
            tmp_commands++;
            continue;
        } else {
            *commands = tmp_commands;
            return; //all comment skipped
        }
    }
    return;
}

//! \brief Find jmp command variant, if exists
//! \param [in,out] commands - Place to find commands. If command founded, shifts to next free position
//! \param [in] commands_end End of commands
//! \return Returns jmp command if jmp command founded, zero else
static int
choose_jmp(char **commands, char *commands_end)
{
    char *tmp_commands = *commands;
    int command = 0;
    int command_size = 0;
    if (tmp_commands + sizeof(JMP_STR) - 1 < commands_end &&
            !strncmp(tmp_commands, JMP_STR, sizeof(JMP_STR) - 1)) {
        command = JMP;
        command_size = sizeof(JMP_STR);
    } 

    if (tmp_commands + sizeof(JMPL_STR) - 1 < commands_end &&
            !strncmp(tmp_commands, JMPL_STR, sizeof(JMPL_STR) - 1)) {
        command = JMPL;
        command_size = sizeof(JMPL_STR);
    }
    if (tmp_commands + sizeof(JMPG_STR) - 1 < commands_end &&
            !strncmp(tmp_commands, JMPG_STR, sizeof(JMPG_STR) - 1)) {
        command = JMPG;
        command_size = sizeof(JMPG_STR);
    }
    *commands = tmp_commands + command_size;
    return command;
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
    
    struct Symtab sym_tab;
    init_sym_tab(&sym_tab);

    int address = 0;
    
    char *commands_end = commands + commands_size;
    Stack_int *jmps = (Stack_int *)calloc(1, sizeof(Stack_int));
    STACK_INIT((*jmps));
    while (commands < commands_end) {
        skip_nonimportant_symbols(&commands, commands_end);
        if (commands >= commands_end) {
            break; // EOF
        }
       //arithmetic commads 
        if (process_alone_command(commands, commands_end, fd, MUL_STR, sizeof(MUL_STR) - 1, MUL, &commands, &address)) continue;
        if (process_alone_command(commands, commands_end, fd, DIV_STR, sizeof(DIV_STR) - 1, DIV, &commands, &address)) continue;
        if (process_alone_command(commands, commands_end, fd, ADD_STR, sizeof(ADD_STR) - 1, ADD, &commands, &address)) continue;
        if (process_alone_command(commands, commands_end, fd, SUB_STR, sizeof(SUB_STR) - 1,  SUB, &commands, &address)) continue;
        if (process_alone_command(commands, commands_end, fd, SQRT_STR, sizeof(SQRT_STR) - 1, SQRT, &commands, &address)) continue;
        if (process_alone_command(commands, commands_end, fd, HLT_STR, sizeof(HLT_STR) - 1, HLT, &commands, &address)) continue;
        //it is not.
        //register commands
        
        if (process_register_command(commands, commands_end, fd, IN_STR, sizeof(IN_STR) - 1, IN_REG, &commands, &address)) continue;
        if (process_register_command(commands, commands_end, fd, OUT_STR, sizeof(OUT_STR) - 1, OUT_REG, &commands, &address)) continue;
        if (process_register_command(commands, commands_end, fd, PUSH_STR, sizeof(PUSH_STR) - 1, PUSH_REG, &commands, &address)) continue;
        if (process_register_command(commands, commands_end, fd, POP_STR, sizeof(POP_STR) - 1, POP_REG, &commands, &address)) continue;
        
        // alone commands with possible register version (processed above) 
        if (process_alone_command(commands, commands_end, fd, IN_STR, sizeof(IN_STR) - 1, IN, &commands, &address)) continue;
        if (process_alone_command(commands, commands_end, fd, OUT_STR, sizeof(OUT_STR) - 1, OUT, &commands, &address)) continue; 
        if (process_alone_command(commands, commands_end, fd, POP_STR, sizeof(POP_STR) - 1, POP_VAL, &commands, &address)) continue;
        
        if (process_value_command(commands, commands_end, fd, PUSH_STR, sizeof(PUSH_STR) - 1, PUSH_VAL, &commands, &address)) continue;
        
        //process jmp command 
        int jmp_type = choose_jmp(&commands, commands_end);
        if (jmp_type) {
            write_to_file(fd, jmp_type);
            skip_nonimportant_symbols(&commands, commands_end);
            
            if (commands >=commands_end) {
                fprintf(stderr, "No label after JMP command at the end of the file\n");
                return false;
            }
            char *label = commands;
            if (*label == '$') {
                char *endptr = NULL;
                errno = 0;
                int jmp_address = strtol(label + 1, &endptr, 10);
                if (errno || endptr == commands) {
                    fprintf(stderr, "Wrong jmp value: %10s\n", commands);
                    return false;
                }
                write(fd, &jmp_address, sizeof(int));
                commands = endptr;
                continue;
            }
            while (label < commands_end && !isspace((int)*label)) label++;
            if (label - commands < 1) {
            fprintf(stderr, "JMP command without label: %s\n", commands);
                return false;
            }
            int ind = find_symbol(&sym_tab, commands, label - commands);
            if (ind == -1) {
                add_symbol(&sym_tab, commands, label - commands);
                Stack_Push(jmps, address + 1);
                ind = find_symbol(&sym_tab, commands, label - commands);
                write(fd, &ind, sizeof(ind)); //here must be jmp address
            } else {
                if (sym_tab.symbols[ind].address == -1) {
                    write(fd, &ind, sizeof(ind)); //here must be jmp address
                    Stack_Push(jmps, address + 1);
                } else {
                    write(fd, &(sym_tab.symbols[ind].address), sizeof(ind));
                }
            }
            address += 1 + sizeof(ind);
            commands = label;
            continue;
        }

        //if here, time to work with label
        //first - count label size
        char *label = commands;
        while (label < commands_end && *label != ':') label++;
        if (label >= commands_end) {
        // Well, it was not label, it is just a wrong command
            fprintf(stderr, "Unknown assembler command: %.10s", commands);
            return false;
        } 
        int ind = find_symbol(&sym_tab, commands, label - commands);
        if (ind == -1) {
            add_symbol(&sym_tab, commands, label - commands);
        }
        add_symbol_address(&sym_tab, commands, label - commands, address);
        commands = label + 1;
    }
    //now all unsolved jmp labels must be solved
    int jmps_num = Stack_Size(jmps);
    for (int i = 0; i < jmps_num; i++) {
        int work_address = Stack_Top(jmps);
        Stack_Pop(jmps);
        lseek(fd, work_address, SEEK_SET);
        int ind = 0;
        read(fd, &ind, sizeof(ind));
        lseek(fd, work_address, SEEK_SET);
        write(fd, &(sym_tab.symbols[ind].address), sizeof(int));
    }
    Stack_Destruct(jmps);
    for (int i = 0; i < sym_tab.size; i++) {
        free(sym_tab.symbols[i].name);
    }
    free(sym_tab.symbols);
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
        fprintf(stderr, "Can not mmap file %s\n", file_in);
        return false;
    }
    int fd_out = open(file_out, O_RDWR | O_CREAT | O_TRUNC, out_mode); 
    if (fd_out < 0) {
        fprintf(stderr, "Error: Can`t open out file %s\n", file_out);
        munmap(commands, file_in_size);
        return false;
    }
    if (!translate_to_machine_code(commands, file_in_size, fd_out)) {
        fprintf(stderr, "Error: Can`t translate to asm from file %s\n", file_out);
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
