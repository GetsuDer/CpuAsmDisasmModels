#ifndef MEMORY_H
#define MEMORY_H
struct Memory
{
    int size;
    double *memory;
};

struct Memory_Controller
{
    int memory_pieces_num;
    struct Memory **memory;
};

int init_memory(struct Memory*, int size);
int init_memory_controller(struct Memory_Controller*);
int add_memory(struct Memory_Controller*, struct Memory*);
int write_into_memory(struct Memory_Controller*, int address, double value);
int get_from_memory(struct Memory_Controller*, int address, double*); 
int get_memory_size(struct Memory_Controller*);

enum Memory_Errors {
    NULL_MEM = 1,
    NEGATIVE_MEM,
    TOO_BIG_ADDRESS,
    READ_UNINITIALIZED_MEM,
    ALLOCATE_ERROR
};

//! Delay for writing into operative memory, in nanoseconds
#define WRITE_DELAY (1000 * 1000 * 100 * 2)
//! Delay for reading from operative memory, in nanoseconds
#define READ_DELAY (1000 * 1000 * 100)
#endif
