TESTDIR = Testing/
SRCDIR = Source/
OBJDIR = ObjectFiles/
INCDIR = Include/
CC = g++
DEBUG = YES
CFLAGS = -Wall -Wextra -Wformat -std=c++14 -g -IInclude -ISource 

ifeq ($(DEBUG), YES)
	CFLAGS += -g
endif

.PHONY: all clean asm disasm cpu test
all: asm disasm cpu
	
test_all: test_asm

test_asm: asm $(TESTDIR)test_asm
	cd $(TESTDIR); ./test_asm; cd ..

asm: $(OBJDIR)asm.o $(OBJDIR)asm_main.o
	$(CC) $(OBJDIR)asm_main.o $(OBJDIR)asm.o -o asm $(CFLAGS)

$(OBJDIR)asm.o: $(SRCDIR)asm.cpp $(INCDIR)asm.h $(INCDIR)cpu.h $(OBJDIR)
	$(CC) -o $(OBJDIR)asm.o -c $(SRCDIR)asm.cpp $(CFLAGS)

$(OBJDIR)asm_main.o: $(SRCDIR)asm_main.cpp $(INCDIR)asm.h $(OBJDIR)
	$(CC) -o $(OBJDIR)asm_main.o -c $(SRCDIR)asm_main.cpp $(CFLAGS)

disasm: $(OBJDIR)disasm.o $(OBJDIR)disasm_main.o
	$(CC) $(OBJDIR)disasm_main.o $(OBJDIR)disasm.o -o disasm $(CFLAGS)

$(OBJDIR)disasm.o: $(SRCDIR)disasm.cpp $(INCDIR)disasm.h $(INCDIR)asm.h $(INCDIR)cpu.h $(OBJDIR)
	$(CC) -o $(OBJDIR)disasm.o -c $(SRCDIR)disasm.cpp $(CFLAGS)

$(OBJDIR)disasm_main.o: $(SRCDIR)disasm_main.cpp $(INCDIR)disasm.h $(OBJDIR)
	$(CC) -o $(OBJDIR)disasm_main.o -c $(SRCDIR)disasm_main.cpp $(CFLAGS)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf *.o ObjectFiles asm disasm cpu
