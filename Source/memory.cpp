#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "memory.h"

//! \brief Initialize memory bar
//! \param [in] mem Memory to init
//! \param [in] size Memory size to allocate
//! \return Returns 0 if success, ERROR from Memory_Errors else
int
init_memory(struct Memory *mem, int size)
{
    assert(mem);
    assert(size > 0);

    if (!mem) {
        fprintf(stderr, "Initialization of NULL memory bar\n");
        return NULL_MEM;
    }

    if (size <= 0) {
        fprintf(stderr, "Wrong memory size: %d\n", size);
        return NEGATIVE_MEM;
    }

    double *tmp = (double *)calloc(size, sizeof(double));
    if (!tmp) {
        fprintf(stderr, "Can not allocate memory for %d double\n", size);
        return ALLOCATE_ERROR;
    }
    mem->memory = tmp;
    mem->size = size;
    return 0;
}

//! \brief Initializing memory controller
//! \param [in] mc Memory controller to initialize
//! \return Returns 0 if success, Error number else
int
init_memory_controller(struct Memory_Controller *mc)
{
    assert(mc);
    if (!mc) {
        return NULL_MEM;
    }
    mc->memory_pieces_num = 0;
    mc->memory = NULL;
    return 0;
}
//! \brief Add memory into memory controller
//! \param [in] mc Memory controller
//! \param [in] mem Memory
//! \return Returns 0 in success, ERROR else
int
add_memory(struct Memory_Controller *mc, struct Memory *mem)
{
    assert(mc);
    assert(mem);

    if (!mc) {
        fprintf(stderr, "Add_memory into null memory controller \n");
        return NULL_MEM;
    }

    if (!mem) {
        fprintf(stderr, "Add null memory into memory controller \n");
        return NULL_MEM;
    }

    if (mc->memory_pieces_num == 0) {
        mc->memory = (struct Memory **)calloc(1, sizeof(mem));
        if (!mc->memory) {
            fprintf(stderr, "Can not allocate memory\n");
            return ALLOCATE_ERROR;
        }
        mc->memory[0] = mem;
        mc->memory_pieces_num = 1;
        return 0;
    }

    assert(mc->memory);
    struct Memory **tmp = (struct Memory **)realloc(mc->memory, (mc->memory_pieces_num + 1) * sizeof(struct Memory *));
    if (!tmp) {
        fprintf(stderr, "Can not allocate memory\n");
        return ALLOCATE_ERROR;
    }
    mc->memory = tmp;
    mc->memory[mc->memory_pieces_num] = mem;
    mc->memory_pieces_num++;
    return 0;
}
//! \brief Find right memory part for memory controller
//! \param [in] mc Memory Controller
//! \param [in,out] address Pointer to address
//! \return Returns pointer to right memory part or NULL if unsuccess. If success, shifts address to be right value in memory part
static struct Memory *
find_address(struct Memory_Controller *mc, int *address)
{
  int max_address = 0;
  for (int i = 0; i < mc->memory_pieces_num; i++) {
      if (max_address + mc->memory[i]->size <= *address) {
          max_address += mc->memory[i]->size;
      } else {
          *address -= max_address;
          return mc->memory[i];
      }
  }
  return NULL;
}

//! \brief Imitate delay during memory operations
//! \param [in] delay Delay in nanoseconds
static void
wait(int delay) {
    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = delay;
    nanosleep(&req, NULL);
    return;
}

//! \brief Find right memory bar by memory controller and write into it
//! \param [in] mc Memory Controller
//! \param [in] address Address
//! \param [in] value Value to write
//! \return Returns 0 in success, ERROR number else
int
write_into_memory(struct Memory_Controller *mc, int address, double value)
{
    assert(mc);
    assert(address >= 0);
    
    wait(WRITE_DELAY);

    struct Memory *right_mem = find_address(mc, &address);
    if (!right_mem) {
        fprintf(stderr, "Can not write into memory %d\n", address);
        return TOO_BIG_ADDRESS;
    }
    right_mem->memory[address] = value;
    return 0;
}
//! \brief Get value from memory
//! \param [in] mc Memory Controller
//! \param [in] address Address
//! \param [out] value Pointer to place to write value
int
get_from_memory(struct Memory_Controller *mc, int address, double *value)
{
    assert(mc);
    assert(value);
    assert(address >= 0);
    
    if (address < 0) {
        fprintf(stderr, "Get memory on negative address %d\n", address);
        return NEGATIVE_MEM;
    }

    wait(READ_DELAY);

    struct Memory *right_memory = find_address(mc, &address);
    if (!right_memory) {
        fprintf(stderr, "Can not get memory on address %d\n", address);
        return TOO_BIG_ADDRESS;
    }
    *value = right_memory->memory[address];
    return 0;
}

//! \brief Get whole available memory
//! \param [in] mc Memory Controller
//! \return Returns memory size
int
get_memory_size(struct Memory_Controller *mc)
{
    assert(mc);

    if (!mc) {
        fprintf(stderr, "Get memory size from null pointer\n");
        return NULL_MEM;
    }
    int res = 0;
    for (int i = 0; i < mc->memory_pieces_num; i++) {
        res += mc->memory[i]->size;
    }
    return res;
}
