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

//! structure for context translation in supporting functions
struct Env
{
    char *commands;
    char *commands_end;
    int fd;
    int address;
};

//! \brief Symbol. At the moment only for labels. Name is saved with \0 symbol.
struct Symbol {
    char *name;
    int name_size;
    int address;
    //in future may be more fields;
};

//! \brief Symtab. Contains many symbols.
struct Symtab {
    struct Symbol *symbols;
    int size;
};

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
    assert(command);
    
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
static int
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
static bool
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
static bool
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
static bool
process_alone_command(struct Env *env, const char *com_str, int com_size, int com) {
    assert(env);
    assert(com_str);
    
    if (env->commands + com_size > env->commands_end || 
            strncmp(env->commands, com_str, com_size)) {
        return false;
    }
    if (env->commands + com_size == env->commands_end || 
        isspace(*(env->commands + com_size))) {
        env->commands += com_size;
        env->address += 1;
        write_to_file(env->fd, com);
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
static bool
process_register_command(struct Env *env, const char *com_str, int com_size, int com) {
    assert(env);
    assert(com_str);
    
    if (env->commands + com_size >= env->commands_end || 
            strncmp(env->commands, com_str, com_size) || !isspace(*(env->commands + com_size))) {
        return false;
    }
    char *old_coms = env->commands;
    env->commands += com_size + 1; //command + space after command
    skip_nonimportant_symbols(&(env->commands), env->commands_end);
    if (env->commands >= env->commands_end) {
        //there is no register here
        env->commands = old_coms;
        return false;
    }

    int reg = write_register_to_file(env->commands);
    if (!reg) {
        //there is no register here
        env->commands = old_coms;
        return false;
    }
    write_to_file(env->fd, com);
    write_to_file(env->fd, reg);
    env->commands += sizeof(RAX_STR);
    env->address += 2; //command and register;
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
static bool
process_value_command(struct Env *env, const char *com_str, int com_size, int com) {
    assert(env);
    assert(com_str);
    
    if (env->commands + com_size >= env->commands_end || 
            strncmp(env->commands, com_str, com_size) || !isspace(*(env->commands + com_size))) {
        return false;
    }
    char *old_coms = env->commands;
    env->commands += com_size;
    skip_nonimportant_symbols(&(env->commands), env->commands_end);
    char *endptr = NULL;
    errno = 0;
    double tmp_double = strtod(env->commands, &endptr);
    if (errno || endptr == env->commands) {
        env->commands = old_coms;
        return false;
    }
     
    write_to_file(env->fd, com);
    write(env->fd, &tmp_double, sizeof(double));
    env->address += 1 + sizeof(double);
    env->commands = endptr; //this is not an error!!! (i hope) 
    return true;
}

static bool
process_write_command(struct Env *env) {
    assert(env);

    int com_size = sizeof(WRITE_STR) - 1;
    if (env->commands + com_size >= env->commands_end ||
        strncmp(env->commands, WRITE_STR, com_size) || !isspace(*(env->commands + com_size))) {
        return false;
    }

    char *old_coms = env->commands;
    env->commands += com_size;
    skip_nonimportant_symbols(&(env->commands), env->commands_end);
    
    if (env->commands >= env->commands_end) {
        env->commands = old_coms;
        return false;
    }
    int tmp_reg1 = write_register_to_file(env->commands);
    if (!tmp_reg1) {
        env->commands = old_coms;
        return false;
    }
    env->commands += sizeof(RAX_STR) - 1;
    skip_nonimportant_symbols(&(env->commands), env->commands_end);
    if (env->commands >= env->commands_end || *(env->commands) != '[') {
        env->commands = old_coms;
        return false;
    }
    env->commands++;

    int tmp_reg2 = write_register_to_file(env->commands);
    if (tmp_reg2) {
     //register
        env->commands += sizeof(RAX_STR) - 1;
        skip_nonimportant_symbols(&(env->commands), env->commands_end);
        if (env->commands >= env->commands_end || *(env->commands) != ']') {
            env->commands_end = old_coms;
            return false;
        }
        env->commands++;
        env->address += 3;
        write_to_file(env->fd, WRITE_REG);
        write_to_file(env->fd, tmp_reg1);
        write_to_file(env->fd, tmp_reg2);
        return true;
    }
    // address
    char *endptr = NULL;
    errno = 0;
    int tmp = strtol(env->commands, &endptr, 10);
    if (errno || endptr == env->commands) {
        env->commands = old_coms;
        return false;
    }
    env->commands = endptr;
    if (env->commands >= env->commands_end || *(env->commands) != ']') {
        env->commands = old_coms;
        return false;
    }
    env->commands++;
    write_to_file(env->fd, WRITE_ADDR);
    write_to_file(env->fd, tmp_reg1);
    write(env->fd, &tmp, sizeof(int));
    env->address += 2 + sizeof(int);
    return true;    
}

static bool
process_read_command(struct Env *env) {
    assert(env);

    int com_size = sizeof(READ_STR) - 1;
    if (env->commands + com_size >= env->commands_end ||
        strncmp(env->commands, READ_STR, com_size) || !isspace(*(env->commands + com_size))) {
        return false;
    }
    char *old_coms = env->commands;
    env->commands += sizeof(READ_STR);
    skip_nonimportant_symbols(&(env->commands), env->commands_end);
    if (env->commands >= env->commands_end || *(env->commands) != '[') {
        env->commands = old_coms;
        return false;
    }
    
    env->commands++;
    skip_nonimportant_symbols(&(env->commands), env->commands_end);
    int tmp_reg2 = write_register_to_file(env->commands);
    if (tmp_reg2) {
     //register
        env->commands += sizeof(RAX_STR) - 1;
        skip_nonimportant_symbols(&(env->commands), env->commands_end);
        if (env->commands >= env->commands_end || *(env->commands) != ']') {
            env->commands_end = old_coms;
            return false;
        }
        env->commands++;
        skip_nonimportant_symbols(&(env->commands), env->commands_end);
        int tmp_reg1 = write_register_to_file(env->commands);
        if (!tmp_reg1) {
            env->commands_end = old_coms;
            return false;
        }
        env->address += 3;
        env->commands += sizeof(RAX_STR) - 1;
        write_to_file(env->fd, READ_REG);
        write_to_file(env->fd, tmp_reg2);
        write_to_file(env->fd, tmp_reg1);
        return true;
    }
    // address
    char *endptr = NULL;
    errno = 0;
    int tmp = strtol(env->commands, &endptr, 10);
    if (errno || endptr == env->commands) {
        env->commands = old_coms;
        return false;
    }
    env->commands = endptr;
    if (env->commands >= env->commands_end || *(env->commands) != ']') {
        env->commands = old_coms;
        return false;
    }
    env->commands++;
    skip_nonimportant_symbols(&(env->commands), env->commands_end);
    int tmp_reg1 = write_register_to_file(env->commands);
    if (!tmp_reg1) {
        env->commands = old_coms;
        return false;
    }
    write_to_file(env->fd, READ_ADDR);
    write(env->fd, &tmp, sizeof(tmp));
    write_to_file(env->fd, tmp_reg1);
    env->address += 2 + sizeof(int);
    env->commands += sizeof(RAX_STR);
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
    if (tmp_commands + sizeof(CALL_STR) - 1 < commands_end &&
            !strncmp(tmp_commands, CALL_STR, sizeof(CALL_STR) - 1)) {
        command = CALL;
        command_size = sizeof(CALL_STR);
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
    
    Stack_int *jmps = (Stack_int *)calloc(1, sizeof(Stack_int));
    STACK_INIT((*jmps));
    
    struct Env *env = (struct Env *)calloc(1, sizeof(*env));
    if (!env) {
        fprintf(stderr, "Memory error\n");
        Stack_Destruct(jmps);
        return false;
    }
    env->commands = commands;
    env->commands_end = commands + commands_size;
    env->fd = fd;
    env->address = 0;

    while (env->commands < env->commands_end) {
        skip_nonimportant_symbols(&(env->commands), env->commands_end);
        if (env->commands >= env->commands_end) {
            break; // EOF
        }
       //arithmetic commads  + ret
        if (process_alone_command(env, MUL_STR, sizeof(MUL_STR) - 1, MUL)) continue;
        if (process_alone_command(env, DIV_STR, sizeof(DIV_STR) - 1, DIV)) continue;
        if (process_alone_command(env, ADD_STR, sizeof(ADD_STR) - 1, ADD)) continue;
        if (process_alone_command(env, SUB_STR, sizeof(SUB_STR) - 1, SUB)) continue;
        if (process_alone_command(env, SQRT_STR, sizeof(SQRT_STR) - 1, SQRT)) continue;
        if (process_alone_command(env, HLT_STR, sizeof(HLT_STR) - 1, HLT)) continue;
        if (process_alone_command(env, RET_STR, sizeof(RET_STR) - 1, RET)) continue;
        //it is not.
        //register commands
        
        if (process_register_command(env, IN_STR, sizeof(IN_STR) - 1, IN_REG)) continue;
        if (process_register_command(env, OUT_STR, sizeof(OUT_STR) - 1, OUT_REG)) continue;
        if (process_register_command(env, PUSH_STR, sizeof(PUSH_STR) - 1, PUSH_REG)) continue;
        if (process_register_command(env, POP_STR, sizeof(POP_STR) - 1, POP_REG)) continue;
        
        // alone commands with possible register version (processed above) 
        if (process_alone_command(env, IN_STR, sizeof(IN_STR) - 1, IN)) continue;
        if (process_alone_command(env, OUT_STR, sizeof(OUT_STR) - 1, OUT)) continue; 
        if (process_alone_command(env, POP_STR, sizeof(POP_STR) - 1, POP_VAL)) continue;
        
        if (process_value_command(env, PUSH_STR, sizeof(PUSH_STR) - 1, PUSH_VAL)) continue;
        
        if (process_write_command(env)) continue;
        if (process_read_command(env)) continue; 
        //process jmp command 
        int jmp_type = choose_jmp(&(env->commands), env->commands_end);
        if (jmp_type) {
            write_to_file(env->fd, jmp_type);
            skip_nonimportant_symbols(&(env->commands), env->commands_end);
            
            if (env->commands >= env->commands_end) {
                fprintf(stderr, "No label after JMP command at the end of the file\n");
                return false;
            }
            char *label = env->commands;
            if (*label == '$') {
                char *endptr = NULL;
                errno = 0;
                int jmp_address = strtol(label + 1, &endptr, 10);
                if (errno || endptr == env->commands) {
                    fprintf(stderr, "Wrong jmp value: %10s\n", env->commands);
                    return false;
                }
                write(env->fd, &jmp_address, sizeof(int));
                env->commands = endptr;
                continue;
            }
            while (label < env->commands_end && !isspace((int)*label)) label++;
            if (label - env->commands < 1) {
                fprintf(stderr, "JMP command without label: %s\n", env->commands);
                return false;
            }
            int ind = find_symbol(&sym_tab, env->commands, label - env->commands);
            if (ind == -1) {
                add_symbol(&sym_tab, env->commands, label - env->commands);
                Stack_Push(jmps, env->address + 1);
                ind = find_symbol(&sym_tab, env->commands, label - env->commands);
                write(env->fd, &ind, sizeof(ind)); //here must be jmp address
            } else {
                if (sym_tab.symbols[ind].address == -1) {
                    write(env->fd, &ind, sizeof(ind)); //here must be jmp address
                    Stack_Push(jmps, env->address + 1);
                } else {
                    write(env->fd, &(sym_tab.symbols[ind].address), sizeof(ind));
                }
            }
            env->address += 1 + sizeof(ind);
            env->commands = label;
            continue;
        }

        //if here, time to work with label
        //first - count label size
        char *label = env->commands;
        while (label < env->commands_end && *label != ':') label++;
        if (label >= env->commands_end) {
        // Well, it was not label, it is just a wrong command
            fprintf(stderr, "Unknown assembler command: %.10s", env->commands);
            return false;
        } 
        int ind = find_symbol(&sym_tab, env->commands, label - env->commands);
        if (ind == -1) {
            add_symbol(&sym_tab, env->commands, label - env->commands);
        }
        add_symbol_address(&sym_tab, env->commands, label - env->commands, env->address);
        env->commands = label + 1;
    }
    //now all unsolved jmp labels must be solved
    int jmps_num = Stack_Size(jmps);
    for (int i = 0; i < jmps_num; i++) {
        int work_address = Stack_Top(jmps);
        Stack_Pop(jmps);
        lseek(env->fd, work_address, SEEK_SET);
        int ind = 0;
        read(env->fd, &ind, sizeof(ind));
        lseek(env->fd, work_address, SEEK_SET);
        write(env->fd, &(sym_tab.symbols[ind].address), sizeof(int));
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
