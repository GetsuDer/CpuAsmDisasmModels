#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cmath>

//! This is magic for right preprocessing
#define CAT_IMPL(a) Stack_##a

//! This is magic for right preprocessing
#define Stack(a) CAT_IMPL(a)

//! This is magic for right preprocessing
#define CAT_IMPL2(a) POISON_##a

//! This is magic for right preprocessing
#define POISON(a) CAT_IMPL2(a)

#define CAT_IMPL3(a) Print_##a
#define PRINT(a) CAT_IMPL3(a)

//! Type for guard bird
#define BIRD unsigned int

//! Type for stack size
#define STACK_SIZE int

//! Type for stack capacity
#define STACK_CAPACITY int

//! Type for hash
#define HASH_TYPE unsigned int

//! Stack name
#define STACK_VAR_NAME const char *name


//! \"Template\" for different stacks
struct Stack(TYPE)
{
#ifdef DEBUG_BIRDS
    BIRD bird1;
#endif


// For dumping with variable name
#ifdef DEBUG_CHECK_CORRECTNESS
    STACK_VAR_NAME;
#endif

#ifdef DEBUG_HASH
    HASH_TYPE hash;
    HASH_TYPE data_hash;
    int data_byte_size;
#endif

    STACK_SIZE size;
    STACK_CAPACITY capacity;
    TYPE * data;

#ifdef DEBUG_BIRDS
    BIRD bird2;
#endif
};

int Stack_Err(Stack(TYPE) *thou);
void print_err(int err, Stack(TYPE) *thou);

//! Type of stack size var. May be changed in future
#define STACK_SIZE int

//! Type of stack capacity var. May be changed in future
#define STACK_CAPACITY int

#define CAT_IMPL4(a,b) Stack_Dump_##a (b)
#define Stack_Dump(a,b) CAT_IMPL4(a,b)

//! Short record for debug checks
#ifdef DEBUG_CHECK_CORRECTNESS
#define STACK_CHECK(a,thou) \
    if (Stack_Err(thou)) {\
        Stack_Dump(a,thou);\
        assert(0);\
    }
#else
#define STACK_CHECK(a, thou)
#endif

//! Epsilon for non-integer comparing
extern double POISON_EPS;

//! Knowingly wrong pointer, but not NULL
extern void *errptr;

//! Special wrong stack size value (poison)
extern STACK_SIZE errsize;

//! Special wrong stack capacity value (poison)
extern STACK_CAPACITY errcapacity;

//! Poisoned value for type int
extern int POISON_int;

//! Poisoned value for type double
extern double POISON_double;

//! Poisoned value for type double
extern unsigned POISON_unsigned;

//! Poisoned value for type double
extern float POISON_float;

//! Poisoned value for type double
extern char POISON_char;

//! Basic value for guard bird
extern BIRD bird_prototype;

#ifndef STACK_ERRORS_H
#define STACK_ERRORS_H
//! Values for error codes. 0 - success, not 0 - error.
enum Stack_Errors
{
    OK = 0,
    NEGATIVE_SIZE,
    NEGATIVE_CAPACITY,
    SIZE_GREATER_CAPACITY,
    NULL_DATA,
    POISONED_VALUE,
    WRONG_STACK_POINTER,
    DEAD_DATA_POINTER,
    FIRST_STACK_BIRD_DEAD,
    SECOND_STACK_BIRD_DEAD,
    FIRST_DATA_BIRD_DEAD,
    SECOND_DATA_BIRD_DEAD,
    NULL_DATA_NONULL_CAPACITY,
    WRONG_STACK_HASH,
    WRONG_DATA_HASH,
    NOT_ENOUGH_MEMORY
};
#endif

HASH_TYPE hash_counter(unsigned char *s, STACK_SIZE number);


// Here are different dumps for different data types, which differs only in
// printing data value and stack type, so there are two common defines.

#define Stack_Dump_Part_One(thou) \
    int err = Stack_Err((thou)); \
    if (!err) { \
        fprintf(stderr, " (ok) "); \
    } else { \
        print_err(err, (thou)); \
    } \
    if (err != WRONG_STACK_POINTER) { \
        STACK_NAME(thou);\
        STACK_HASH(thou);\
        fprintf(stderr, "\n [ %p ] ", thou); \
        fprintf(stderr, "[ in file: %s on func: %s on line: %d ]", __FILE__, __func__, __LINE__);  \
        STACK_BIRDS_CHECK(thou);\
        fprintf(stderr, "\n size = %d, capacity = %d ", (thou)->size, (thou)->capacity);\
        if ((thou)->capacity > 0) {\

#define Stack_Dump_Part_Two(thou)\
                fprintf(stderr, " !Poisoned!");\
            }\
        }\
    }\
    fprintf(stderr, "\n================================================================\n");

//! \brief Dumps Stack for type int and its content into stderr (now)
//! \param [in] thou - pointer on stack
#define Stack_Dump_int(thou) \
{ \
    fprintf(stderr, "stack dump [TYPE = int]"); \
    Stack_Dump_Part_One(thou);\
    DATA_BIRDS_CHECK(thou, int);\
    }\
    for (int i = 0; i < (thou)->size; i++) {\
    fprintf(stderr, "\n *[%d] = %d", i, (thou)->data[i]);\
            if ((thou)->data[i] == POISON_int) {\
    Stack_Dump_Part_Two(thou);\
}

//! \brief Dumps Stack for type unsigned and its content into stderr (now)
//! \param [in] thou - pointer on stack
#define Stack_Dump_unsigned(thou) \
{ \
    fprintf(stderr, "stack dump [TYPE = unsigned]"); \
    Stack_Dump_Part_One(thou);\
    DATA_BIRDS_CHECK(thou, unsigned);\
    }\
    for (int i = 0; i < (thou)->size; i++) {\
    fprintf(stderr, "\n *[%d] = %u", i, (thou)->data[i]);\
            if ((thou)->data[i] == POISON_unsigned) {\
    Stack_Dump_Part_Two(thou);\
}

//! \brief Dumps Stack for type double and its content into stderr (now)
//! \param [in] thou - pointer on stack
#define Stack_Dump_double(thou) \
{ \
    fprintf(stderr, "stack dump [TYPE = double]"); \
    Stack_Dump_Part_One(thou);\
    DATA_BIRDS_CHECK(thou, double);\
    }\
    for (int i = 0; i < (thou)->size; i++) {\
    fprintf(stderr, "\n *[%d] = %lf", i, (thou)->data[i]);\
            if (fabs((thou)->data[i] - POISON_double) < EPS) {\
    Stack_Dump_Part_Two(thou);\
}

//! \brief Dumps Stack for type float and its content into stderr (now)
//! \param [in] thou - pointer on stack
#define Stack_Dump_float(thou) \
{ \
    fprintf(stderr, "stack dump [TYPE = float]"); \
    Stack_Dump_Part_One(thou);\
    DATA_BIRDS_CHECK(thou, float);\
    }\
    for (int i = 0; i < (thou)->size; i++) {\
    fprintf(stderr, "\n *[%d] = %f", i, (thou)->data[i]);\
            if (fabs((thou)->data[i] - POISON_float) < EPS) {\
    Stack_Dump_Part_Two(thou);\
}

//! \brief Dumps Stack for type char and its content into stderr (now)
//! \param [in] thou - pointer on stack
#define Stack_Dump_char(thou) \
{ \
    fprintf(stderr, "stack dump [TYPE = char]"); \
    Stack_Dump_Part_One(thou);\
    DATA_BIRDS_CHECK(thou, char);\
    }\
    for (int i = 0; i < (thou)->size; i++) {\
    fprintf(stderr, "\n *[%d] = %c", i, (thou)->data[i]);\
            if ((thou)->data[i] == POISON_char) {\
    Stack_Dump_Part_Two(thou);\
}

#ifdef DEBUG_CHECK_CORRECTNESS
#define STACK_NAME(thou) fprintf(stderr, "\nvar name = %s \n", (thou)->name);
#else
#define STACK_NAME(thou)
#endif

#ifdef DEBUG_BIRDS

#define STACK_BIRDS_CHECK(thou) \
    fprintf(stderr, "\n first bird: 0x%x second bird: 0x%x etalon bird: 0x%x\n", \
            (thou)->bird1, (thou)->bird2, bird_prototype);\
        if ((thou)->bird1 != bird_prototype) {\
            fprintf(stderr, "First bird dead!\n");\
        }\
        if ((thou)->bird2 != bird_prototype) {\
            fprintf(stderr, "Second bird dead!\n");\
        }

#define DATA_BIRDS_CHECK(thou, type) \
    BIRD bird_tmp_1 = *(BIRD *)(((char *)(thou)->data) - sizeof(BIRD));\
    BIRD bird_tmp_2 = *(BIRD *)(((char *)(thou)->data + sizeof(type) * (thou)->capacity));\
    fprintf(stderr, "\n first data bird: 0x%x second data bird: 0x%x\n", bird_tmp_1, bird_tmp_2);\
    if (bird_tmp_1 != bird_prototype) {\
        fprintf(stderr, "First data bird dead!\n");\
    }\
    if (bird_tmp_2 != bird_prototype) {\
        fprintf(stderr, "Second data bird dead!\n");\
    }

#else

#define STACK_BIRDS_CHECK(thou)
#define DATA_BIRDS_CHECK(thou, type)

#endif

#ifdef DEBUG_HASH
#define STACK_HASH(thou) \
    HASH_TYPE saved = (thou)->hash;\
    (thou)->hash = 0;\
    HASH_TYPE new_hash = hash_counter((unsigned char *)thou, sizeof(*(thou)));\
    fprintf(stderr, "\n saved stack hash = 0x%x, calculated hash = 0x%x", saved, new_hash);\
    if (saved != new_hash) {\
        fprintf(stderr, " wrong hash! ");\
    }\
    (thou)->hash = saved;\
    if ((thou)->data) {\
        saved = (thou)->data_hash;\
        new_hash = hash_counter((unsigned char *)((thou)->data), (thou)->data_byte_size);\
        fprintf(stderr, "\n saved stack data hash = 0x%x, calculated data hash = 0x%x", saved, new_hash);\
        if (saved != new_hash) {\
            fprintf(stderr, "wrong hash for data! ");\
        }\
    }

#else
#define STACK_HASH(thou)
#endif

#define STACK_INT(a) Stack_int a; STACK_INIT(a)
#define STACK_UNSIGNED(a) Stack_unsigned a; STACK_INIT(a)
#define STACK_DOUBLE(a) Stack_double a; STACK_INIT(a)
#define STACK_FLOAT(a) Stack_float a; STACK_INIT(a)
#define STACK_CHAR(a) Stack_char a; STACK_INIT(a)

#define STACK_INIT(a) \
   STACK_NAME_INIT(a);\
   a.size = 0;\
   a.capacity = 0;\
   a.data = NULL;\
   STACK_BIRDS_INIT(a);\
   STACK_HASH_INIT(a);

#ifdef DEBUG_CHECK_CORRECTNESS
#define STACK_NAME_INIT(a) a.name = #a;
#else
#define STACK_NAME_INIT(a)
#endif

#ifdef DEBUG_BIRDS
#define STACK_BIRDS_INIT(a) \
    a.bird1 = bird_prototype;\
    a.bird2 = bird_prototype;
#else
#define STACK_BIRDS_INIT(a)
#endif

#ifdef DEBUG_HASH
#define STACK_HASH_INIT(a) \
    a.hash = 0;\
    a.data_hash = 0;\
    a.data_byte_size = 0;\
    a.hash = hash_counter((unsigned char *)&(a), sizeof((a)));
#else
#define STACK_HASH_INIT(a)
#endif

//! \brief Destructor for stack
//! \param [in] thou Valid pointer on stack
//! \return Returns true, if after destruction stack is poisoned (destructed correctly), else false 
bool Stack_Destruct(Stack(TYPE) *thou)
{
    STACK_CHECK(TYPE, thou);
    assert(thou);

    if (thou->data) {
#ifdef CHECK_CORRECTNESS
        for (int i = 0; i < thou->size; i++) {
            thou->data[i] = POISON(TYPE);
        }
        thou->data_hash = 0;
#endif
#ifdef DEBUG_BIRDS
        free((char *)(thou->data) - sizeof(BIRD));
#else
        free(thou->data);
#endif //DEBUG_BIRDS
        thou->data = (TYPE *) errptr;
    }
#ifdef DEBUG_HASH
    thou->hash = 0;
#endif
    thou->size = errsize;
    thou->capacity = errcapacity;
    return !Stack_Err(thou);
}

//! \brief Add elem in stack
//! \param[in] elem Elem to be added
//! \param[in] thou Valid pointer on stack in which elem must be inserted
//! \return Returns 0 (OK) if success, else ErrorCode from Stack_Errors (see global_stack.h)
int Stack_Push(Stack(TYPE) *thou, TYPE elem)
{
    STACK_CHECK(TYPE, thou);
    
    if (thou->capacity == 0) {
#ifdef DEBUG_BIRDS
        BIRD * tmp_data_pointer = (BIRD *) calloc(1, sizeof(TYPE) + 2 * sizeof(BIRD));
        if (!tmp_data_pointer) { // stack wasn`t changed yet
            return NOT_ENOUGH_MEMORY;
        }

        //set first bird
        *tmp_data_pointer = bird_prototype;
        //set second bird
        *(BIRD *) (((char *)tmp_data_pointer) + sizeof(BIRD) + sizeof(TYPE)) = bird_prototype;
        //stack data pointer is shifted
        thou->data = (TYPE *) (((char *)tmp_data_pointer) + sizeof(BIRD));
#else
        TYPE *tmp_data_pointer = (TYPE *) calloc(1, sizeof(TYPE));
#ifdef CHECK_CORRECTNESS
        if (!tmp_data_pointer) {
            // stack wasn`t changed yet
            return NOT_ENOUGH_MEMORY;
        }
#endif //CHECK_CORRECTNESS
        thou->data = tmp_data_pointer;
#endif //DEBUG_BIRDS

        thou->capacity = 1;
#ifdef DEBUG_HASH
        thou->data_byte_size = sizeof(TYPE);
#endif
    }

    if (thou->size == thou->capacity) {
//shadow control begins
#ifdef SHADOW_CONTROL
        TYPE *saved_data = (TYPE *) calloc(thou->capacity, sizeof(TYPE));
        if (saved_data) {
            for (int i = 0; i < thou->capacity; i++) {
                saved_data[i] = thou->data[i];
                thou->data[i] = POISON(TYPE);
            }
        }
#endif //SHADOW_CONTROL
#ifdef DEBUG_BIRDS
        TYPE *tmp_data_pointer = (TYPE *) ((char *)realloc(((char *)thou->data) - sizeof(BIRD), (thou->capacity * 2 + 1) * sizeof(TYPE) + sizeof(BIRD) * 2) + sizeof(BIRD));
        if (!tmp_data_pointer) {
            return NOT_ENOUGH_MEMORY;
        }
        thou->data = tmp_data_pointer;
        *(BIRD *)(((char *)thou->data) + sizeof(TYPE) * (thou->capacity * 2 + 1)) = bird_prototype;
#else
        TYPE *tmp_data_pointer = (TYPE *) realloc(thou->data, (thou->capacity * 2 + 1) * sizeof(TYPE));
#ifdef CHECK_CORRECTNESS
        if (!tmp_data_pointer) {
            return NOT_ENOUGH_MEMORY;
        }
#endif //CHECK_CORRECTNESS
        thou->data = tmp_data_pointer;
#endif //DEBUG_BIRDS

#ifdef SHADOW_CONTROL
        if (saved_data) {
            for (int i = 0; i < thou->capacity; i++) {
                thou->data[i] = saved_data[i];
                saved_data[i] = POISON(TYPE);
            }
            free(saved_data);
        }
#endif //SHADOW_CONTROL

        thou->capacity = thou->capacity * 2 + 1;
#ifdef CHECK_CORRECTNESS
        for (STACK_SIZE i = thou->size; i < thou->capacity; i++) {
            thou->data[i] = POISON(TYPE);
        }
#endif //CHECK_CORRECTNESS

#ifdef DEBUG_HASH
        thou->data_byte_size = thou->capacity * sizeof(TYPE);
#endif //DEBUG_HASH
    }

    thou->data[thou->size] = elem;
    thou->size++;

#ifdef DEBUG_HASH
    thou->data_hash = hash_counter((unsigned char *)thou->data, thou->data_byte_size);
    thou->hash = 0;
    thou->hash = hash_counter((unsigned char *)thou, sizeof(*thou));
#endif //DEBUG_HASH
    STACK_CHECK(TYPE, thou);
    return Stack_Err(thou);
}

//! \brief Pops the stack top elem
//! \param [in] thou Stack to be processed
//! \return Return 0 (OK) if success, error code from Stack_Errors else (see global_stack.h)
int Stack_Pop(Stack(TYPE) *thou)
{
    STACK_CHECK(TYPE, thou);
    if (thou->size < 1) {
        return -1;
    }

    thou->size--;

#ifdef CHECK_CORRECTNESS
    thou->data[thou->size] = POISON(TYPE);
#endif //CHECK_CORRECTNESS

#ifdef DEBUG_HASH
    thou->hash = 0;
    thou->data_hash = hash_counter((unsigned char *)thou->data, thou->data_byte_size); 
    thou->hash = hash_counter((unsigned char *)thou, sizeof(*thou));
#endif //DEBUG_HASH

    STACK_CHECK(TYPE, thou);
    return 0; 
}

//! \brief Take the stack top without deleting
//! \param [in] thou Stack to be processed
//! \return Return value from stack top. If trying to see elem from empty stack, returns POISONED VALUE (see global_stack.h)
TYPE Stack_Top(Stack(TYPE) *thou)
{
    STACK_CHECK(TYPE, thou);
    if (thou->size <= 0) {
        return POISON(TYPE);
    }
    return thou->data[thou->size - 1];
}

//! \brief Check stack
//! \param [in] thou Pointer on stack to be checked
//! \return Returns error code from Stack_Errors (see global_stack.h) or 0 (OK)
int Stack_Err(Stack(TYPE) *thou)
{
    if (!thou) {
        return WRONG_STACK_POINTER;
    }
#ifdef DEBUG_BIRDS
    if (thou->bird1 != bird_prototype) {
        return FIRST_STACK_BIRD_DEAD;
    }
    if (thou->bird2 != bird_prototype) {
        return SECOND_STACK_BIRD_DEAD;
    }
#endif //DEBUG_BIRDS
    if (thou->size < 0) {
        return NEGATIVE_SIZE;
    }
    if (thou->capacity < 0) {
        return NEGATIVE_CAPACITY;
    }
    if (thou->size > thou->capacity) {
        return SIZE_GREATER_CAPACITY;
    }
    if (thou->size != 0 && !thou->data) {
        return NULL_DATA;
    }
    if (thou->data == errptr) {
        return DEAD_DATA_POINTER;
    }
    if (!thou->data && (thou->capacity != 0)) {
        return NULL_DATA_NONULL_CAPACITY;
    }
#ifdef DEBUG_BIRDS
    if (thou->data) {
        BIRD data_bird_1 = *(BIRD *)(((char *)thou->data) - sizeof(BIRD));
        BIRD data_bird_2 = *(BIRD *)(((char *)thou->data) + thou->capacity * sizeof(TYPE));
        if (data_bird_1 != bird_prototype) {
            return FIRST_DATA_BIRD_DEAD;
        }
        if (data_bird_2 != bird_prototype) {
            return SECOND_DATA_BIRD_DEAD;
        }
    }
#endif //DEBUG_BIRDS

#ifdef DEBUG_HASH
    HASH_TYPE saved = thou->hash;
    thou->hash = 0;
    HASH_TYPE new_hash = hash_counter((unsigned char *)thou, sizeof(*thou));
    thou->hash = saved;
    if (saved != new_hash) {
        return WRONG_STACK_HASH;
    }    
    if (thou->data) {
        new_hash = hash_counter((unsigned char *)(thou->data), thou->data_byte_size);
        if (thou->data_hash != new_hash) {
            return WRONG_DATA_HASH;
        }
    }
#endif //DEBUG_HASH
    return OK;
}

//! \brief Stack size
//! \param [in] thou Stack for stack size info
//! \return returns Stack size
STACK_SIZE Stack_Size(Stack(TYPE) *thou)
{
    STACK_CHECK(TYPE, thou);
    assert(thou);
    return thou->size;
}


//! \brief Check on empty stack
//! \param [in] thou Stack to be checked
//! \return Returns true if stack is empty
bool Stack_Empty(Stack(TYPE) *thou)
{
    STACK_CHECK(TYPE, thou);
    assert(thou);
    return !thou->size;
}


//! \brief Error decryption
//! \param [in] err Error code from Stack_Errors (see global_stack.h)
//! \param [in] thou Stack for additional info (if null, just basic decryption)
void print_err(int err, Stack(TYPE) *thou = NULL) 
{
    if (err == NOT_ENOUGH_MEMORY) {
        fprintf(stderr, " Error during memory allocation: not enough memory. ");
        return;
    }
    if (err == WRONG_STACK_POINTER) {
        fprintf(stderr, " Dumping stack on NULL pointer, nothing interesting? ");
        return;
    }
    if (err == NEGATIVE_SIZE) {
        fprintf(stderr, " Stack size is negative");
        if (thou) {
            fprintf(stderr, ": %d ", thou->size);
            if (thou->size == errsize) {
                fprintf(stderr, " [POISONED!] ");
            }
        }
        return;
    }
    if (err == NEGATIVE_CAPACITY) {
        fprintf(stderr, " Stack capacity is negative");
        if (thou) {
            fprintf(stderr, ": %d ", thou->capacity);
            if (thou->size == errcapacity) {
                fprintf(stderr, " [POISONED!] ");
            }
        }
        return;
    }
    if (err == SIZE_GREATER_CAPACITY) {
        fprintf(stderr, " Stack size is > then stack capacity");
        if (thou) {
            fprintf(stderr, ": %d  > %d ", thou->size, thou->capacity);
        }
        return;
    }
    if (err == NULL_DATA) {
        fprintf(stderr, " Stack data pointer is null, but stack size is not zero!");
        if (thou) {
           fprintf(stderr, "Stack size is: %d ", thou->size);
        }
        return;
    }
    if (err == DEAD_DATA_POINTER) {
        fprintf(stderr, " Stack data pointer is poisoned, may be stack was destructed? ");
        return;
    }
    if (err == FIRST_STACK_BIRD_DEAD) {
        fprintf(stderr, " First stack bird is dead ");
        return;
    }
    if (err == SECOND_STACK_BIRD_DEAD) {
        fprintf(stderr, " Second stack bird is dead ");
        return;
    }
    if (err == FIRST_DATA_BIRD_DEAD) {
        fprintf(stderr, " First data bird is dead ");
        return;
    }
    if (err == SECOND_DATA_BIRD_DEAD) {
        fprintf(stderr, " Second data bird is dead ");
        return;
    }
    if (err == NULL_DATA_NONULL_CAPACITY) {
        fprintf(stderr, " Stack capacity is not zero, but data pointer is null! ");
        return;
    }
    if (err == WRONG_STACK_HASH) {
        fprintf(stderr, " Stack hash is wrong ");
        return;
    }
    if (err == WRONG_DATA_HASH) {
        fprintf(stderr, " Data hash is wrong ");
        return;
    }
    fprintf(stderr, " Sorry, i don`t know this mistake :( ");
    return;
}

#ifndef STACK_H
#define STACK_H

#define BIRD unsigned int
#define STACK_SIZE int
#define STACK_CAPACITY int
#define HASH_TYPE unsigned int
void *errptr = (void *)13;
STACK_SIZE errsize = -777;
STACK_CAPACITY errcapacity = -666;
int POISON_int = 0xDEADBEEF;
double POISON_double = 12345.54321;
unsigned POISON_unsigned = 0x0EADBEEF;
float POISON_float = 12345.54321;
char POISON_char = 127;
BIRD bird_prototype = 0xDEADBEEFULL;
double POISON_EPS = 1e-8;
HASH_TYPE hash_counter(unsigned char *s, STACK_SIZE number)
{
    assert(s);
    HASH_TYPE hash = 0;
    while (number > 0) {
        hash += *s;
        hash -= (hash << 13) | (hash >> 19);
        number--;
        s++;
    }
    return hash;
}

#endif
