# CPU+

## Description
### Assembler commands and syntax
#### Arithmetic operations
    mul - top value from cpu stack is multiplicated on second top
    div - second top value from cpu stack is divided on top.
    add - sum top two values from cpu stack
    sub - sub top value from stack from second top stack value
    sqrt - calculate sqrt from stack top
All these commands put their result on the stack again.
#### In and out operations
    in REGISTER_NAME - read value from stdin and put it into register
    in - read value from stdin and put it into stack
    out REGISTER_NAME - print value from register to stdout
    out - print value from stack without popping
#### Other stack operations
    push REGISTER_NAME - take value from register and push it into stack
    push VALUE - take value VALUE and push it into stack
    pop REGISTER_NAME - pop value from stack and put it into register
    pop - pop value from stack
#### CPU operations
    hlt - stop working
    jmp LABEL - jmp to label LABEL (it can be defined later)
    LABEL: - set a label with name LABEL
LABEL is an arbirtrary consecuence of non-space symbols

Where REGISTER_NAME is in {rax, rbx, rcx}, and VALUE can be presented as double

## Starting
    run 'make all' to get cpu, asm and disasm programs (see description in documentation)
    'make cpu' to get cpu
    'make asm' to get asm
    'make disasm' to get disasm
## Debug
    To turn debug on run make command with 'DEBUG=YES'
    It turns on -g option and numeration of disassemled code (Be careful, with this option 
            you can not send disassembler output back to assembler)
## Testing
    Directory 'Testing' consists of subdirectories with test in next format
####
    test_name.in (may be also test_name.stdin for cpu)  - input for program
    test_name.out (or test_name.stdout for cpu) - expected output
##
    To run tests run 'make test_all' to test all three subprograms (cpu, asm, disasm),
    or 'make test_asm', 'make test_disasm', 'make test_cpu' to cpecify test target.

### Dependences
    Linux, g++, make

## Documentation
To see the whole documentation, download source code and run 'doxywizard Documentation/Config'
(may be you should also update in dozywizard 'directory in which run doxywizard')
