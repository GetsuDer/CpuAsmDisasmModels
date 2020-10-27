#include <stdio.h>

#include "cpu.h"
#include "cpu_main.h"
#include "in_and_out.h"

int
main(int argc, char **argv)
{
    if (argc < ARG_NUM) {
        fprintf(stderr, "Specify input and output files\n");
        return 1;
    }
    
    int commands_size = 0;
    char *commands = mmap_file(argv[FILE_IN], &commands_size);
    if (!commands) {
        fprintf(stderr, "Error: Can`t mmap file %s\n", argv[FILE_IN]);
        return 1;
    }

    struct Cpu work_cpu;
    init(&work_cpu);
    work(commands, commands_size, &work_cpu);

    return 0;
}
