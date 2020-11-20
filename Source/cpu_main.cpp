#include <stdio.h>

#include "cpu.h"
#include "in_and_out.h"
#include "memory.h"
#include "cpu_main.h"

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

    Memory mem1;
    Memory mem2;
    init_memory(&mem1, 10);
    init_memory(&mem2, 5);

    Memory_Controller mc;
    init_memory_controller(&mc);

    add_memory(&mc, &mem1);
    add_memory(&mc, &mem2);

    work(commands, commands_size, &work_cpu, &mc);

    return 0;
}
