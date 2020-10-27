#include <cstdio>


#include "asm.h"
#include "asm_main.h"

int
main(int argc, char **argv)
{
    if (argc < ARG_NUM) {
        fprintf(stderr, "Please, specify in and out files\n");
        return 1;
    }
    if (!in_and_out_from_asm(argv[FILE_IN], argv[FILE_OUT])) {
        fprintf(stderr, "File %s can not be translated to asm", argv[FILE_IN]);
        fprintf(stderr, " or result can not be written into file %s\n", argv[FILE_OUT]);
        return 1;
    }
    return 0;
}
