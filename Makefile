TESTDIR = Testing/
SRCDIR = Source/
OBJDIR = ObjectFiles/
INCDIR = Include/
CC = g++
DEBUG = NO
CFLAGS = -Wall -Wextra -Wformat -std=c++14 -IInclude -ISource 
TEST_LOG_DISASM = disasm_test_log
TEST_LOG_ASM = asm_test_log
TEST_LOG_CPU = cpu_test_log

ifeq ($(DEBUG), YES)
	CFLAGS += -g -DDEBUG_NUMERATION
endif

.PHONY: all clean asm disasm cpu test_all test_disasm test_asm

all: asm disasm cpu
	
test_all: test_asm test_disasm test_cpu

test_cpu: cpu $(TESTDIR)test_cpu
	cd $(TESTDIR); ./test_cpu > ../$(TEST_LOG_CPU); cd ..

test_disasm: disasm $(TESTDIR)test_disasm
	cd $(TESTDIR); ./test_disasm > ../$(TEST_LOG_DISASM); cd ..

test_asm: asm $(TESTDIR)test_asm
	cd $(TESTDIR); ./test_asm > ../$(TEST_LOG_ASM); cd ..

cpu: $(OBJDIR)cpu.o $(OBJDIR)cpu_main.o $(OBJDIR)in_and_out.o $(OBJDIR)memory.o
	$(CC) $(OBJDIR)cpu_main.o $(OBJDIR)cpu.o $(OBJDIR)in_and_out.o $(OBJDIR)memory.o -o cpu $(CFLAGS)

asm: $(OBJDIR)asm.o $(OBJDIR)asm_main.o $(OBJDIR)in_and_out.o
	$(CC) $(OBJDIR)asm_main.o $(OBJDIR)asm.o $(OBJDIR)in_and_out.o -o asm $(CFLAGS)

disasm: $(OBJDIR)disasm.o $(OBJDIR)disasm_main.o $(OBJDIR)in_and_out.o
	$(CC) $(OBJDIR)disasm_main.o $(OBJDIR)disasm.o $(OBJDIR)in_and_out.o -o disasm $(CFLAGS)

$(OBJDIR)in_and_out.o: $(SRCDIR)in_and_out.cpp $(INCDIR)in_and_out.h
	$(CC) -o $(OBJDIR)in_and_out.o -c $(SRCDIR)in_and_out.cpp $(CFLAGS)

$(OBJDIR)cpu.o: $(SRCDIR)cpu.cpp $(INCDIR)cpu.h $(INCDIR)in_and_out.h $(OBJDIR)
	$(CC) -o $(OBJDIR)cpu.o -c $(SRCDIR)cpu.cpp $(CFLAGS)

$(OBJDIR)cpu_main.o: $(SRCDIR)cpu_main.cpp $(INCDIR)cpu.h $(OBJDIR)
	$(CC) -o $(OBJDIR)cpu_main.o -c $(SRCDIR)cpu_main.cpp $(CFLAGS)

$(OBJDIR)asm.o: $(SRCDIR)asm.cpp $(INCDIR)in_and_out.h $(INCDIR)asm.h $(INCDIR)cpu.h $(OBJDIR)
	$(CC) -o $(OBJDIR)asm.o -c $(SRCDIR)asm.cpp $(CFLAGS)

$(OBJDIR)asm_main.o: $(SRCDIR)asm_main.cpp $(INCDIR)asm.h $(OBJDIR)
	$(CC) -o $(OBJDIR)asm_main.o -c $(SRCDIR)asm_main.cpp $(CFLAGS)

$(OBJDIR)disasm.o: $(SRCDIR)disasm.cpp $(INCDIR)in_and_out.h $(INCDIR)disasm.h $(INCDIR)asm.h $(INCDIR)cpu.h $(OBJDIR)
	$(CC) -o $(OBJDIR)disasm.o -c $(SRCDIR)disasm.cpp $(CFLAGS)

$(OBJDIR)disasm_main.o: $(SRCDIR)disasm_main.cpp $(INCDIR)disasm.h $(OBJDIR)
	$(CC) -o $(OBJDIR)disasm_main.o -c $(SRCDIR)disasm_main.cpp $(CFLAGS)

$(OBJDIR)memory.o: $(SRCDIR)memory.cpp $(INCDIR)memory.h $(OBJDIR)
	$(CC) -o $(OBJDIR)memory.o -c $(SRCDIR)memory.cpp $(CFLAGS)

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -rf *.o ObjectFiles asm disasm cpu *_test_log
