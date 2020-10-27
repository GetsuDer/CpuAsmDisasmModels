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
Where REGISTER_NAME is in {rax, rbx, rcx}, and VALUE can be presented as double

## Starting

### Dependences
    Linux, g++, make

### Run

## Documentation
To see the whole documentation, download source code and run 'doxywizard Documentation/Config'
(may be you should also update in dozywizard 'directory in which run doxywizard')
