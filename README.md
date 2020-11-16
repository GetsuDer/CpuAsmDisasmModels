# CPU+

## Description
### Assembler commands and syntax
    '# this is comment. #'
    Comment can include '\n' symbol, but comment can not be inside of the commands (m#comment#mul is incorrect, for example)
    push #comment# rax is correct, at the same time.
    command_name [{address, register}] - access to memory
    $address - absolute address in program being executed

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
#### JMP operations
    jmp LABEL - jmp to label LABEL (it can be defined later)
    jmp $address - jmp to absolute address
    jmpl {LABEL, $address} - jmp if for last two values in stack a, b (push a, push b) a < b
    jmpg {LABEL, $address} - - - - - - jmp if a > b
    jmp{l, g} POP LAST TWO VALUES FROM STACK, be careful
    LABEL: - set a label with name LABEL
#### Functions
    call func_name - jmp to func_name with saving return point
    ret - try to return from function (if no function is runned, error is raised)
    func_name must be correct label. So, if you want, you can jump to func_name as on label.
    Parameters for functions are passed through stack and are NOT removed by function. And stack after returning
    from function must be at the same state, as before. Return value is in rax register.
#### Memory
    write REGISTER_NAME [ADDRESS] - write content of register into memory
    write REGISTER_NAME [REGISTER_NAME] write content of register into memory pointed by another register
    read [ADDRESS] REGISTER_NAME - read from memory into register
    read [REGISTER_NAME] REGISTER_NAME read from memory pointed by register into register

LABEL is an arbirtrary consecuence of non-space symbols, but it should not begins from '$' symbol

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
