#include <sys/types.h>
#include <fcntl.h>

#ifndef ASM_H
#define ASM_H

//! Command string for mul
const char MUL_STR[] = "mul";

//! Command string for div
const char DIV_STR[] = "div";

//! Command string for add
const char ADD_STR[] = "add";

//! Command string for sub
const char SUB_STR[] = "sub";

//! Command string for sqrt
const char SQRT_STR[] = "sqrt";

//! Command string for in
const char IN_STR[] = "in";

//! Command string for out
const char OUT_STR[] = "out";

//! Command string for push
const char PUSH_STR[] = "push";

//! Command string for pop
const char POP_STR[] = "pop";

//! Command string for hlt
const char HLT_STR[] = "hlt";

//! Command string for rax
const char RAX_STR[] = "rax";

//! Command string for rax
const char RBX_STR[] = "rbx";

//! Command string for rax
const char RCX_STR[] = "rcx";

constexpr mode_t out_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
bool in_and_out_from_asm(char *file_in, char *file_out);
#endif
