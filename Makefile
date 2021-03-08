# Makefile for 6502 assembler

# accept default flags
CFLAGS?=
CPPFLAGS?=
LDFLAGS?=
LIBS?=

# set final objects
ASSEMBLER=6502-as
DEMO_PRG_FILE=HELLO.PRG

ASSEMBLER_OBJS=\
	asm/assembler.o \
	asm/ltokenizer.o \
	asm/main.o

DEMO_FILES="demo/"

.PHONY: all demo assembler clean clean-demo clean-assembler

all: demo assembler

demo: $(DEMO_PRG_FILE)
$(DEMO_PRG_FILE): $(DEMO_FILES)
	./$(ASSEMBLER) $< -o $@

assembler: $(ASSEMBLER)
$(ASSEMBLER): $(ASSEMBLER_OBJS)
	$(CXX) -o $@ $(ASSEMBLER_OBJS) $(LDFLAGS)

.cpp.o:
	$(CXX) -c $< -o $@ $(CFLAGS) $(CPPFLAGS) 

clean: clean-demo clean-assembler

clean-demo:
	rm -f $(DEMO_PRG_FILE)

clean-assembler:
	rm -f $(ASSEMBLER_OBJS)
	rm -f $(ASSEMBLER)