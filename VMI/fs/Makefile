SHELL = /bin/sh

LIB_DIR = $(CURDIR)/lib/
BIN_DIR = $(CURDIR)/bin/
OBJ_DIR = $(CURDIR)/obj/

TOP      = $(CURDIR)/../..
INCLUDES = -I$(CURDIR)/include\
           -I$(CURDIR)/include/linux\
           -I$(CURDIR)/include/configs\
           -I$(CURDIR)/arch/x86/include
#           -I$(CURDIR)/include/configs\
#           -I$(CURDIR)/include/asm-generic/bitops\
#           -I$(CURDIR)/arch/x86/include/asm\
#           -I$(CURDIR)/arch/x86/include/asm/arch-coreboot
MACRO    = #-DDO_DEPS_ONLY -D__KERNEL__ -D__ASSEMBLY__
CFLAG    = -g # -g means add debug inform, -s means remove debug inform
CFLAGS   = $(INCLUDES) $(CFLAG) $(MACRO)
LDFLAGS  = -L$(TOP)/VMI/fs/lib -lfs
CC       = cc
AR       = ar r
RM       = rm -rf

$(shell mkdir -p $(LIB_DIR))
$(shell mkdir -p $(BIN_DIR))
$(shell mkdir -p $(OBJ_DIR))

SOURCES = $(wildcard *.c) $(wildcard ext4/*.c) $(wildcard disk/*.c)

#specific bin source file
BIN_SOURCES = $(notdir $(CURDIR))-op.c
BIN_OBJECTS = $(BIN_SOURCES:%.c=%.o)

LIB_SOURCES = $(filter-out $(BIN_SOURCES), $(SOURCES))
#LIB_OBJECTS = $(filter-out $(BIN_OBJECTS), $(wildcard *.o))

LIB = lib$(notdir $(CURDIR)).a
BIN = $(BIN_SOURCES:%.c=%)

.PHONY : all
all : OBJS LIBS MOVES

# -c means only generate object files
OBJS:
	$(CC) $(CFLAGS) -c $(LIB_SOURCES)

LIBS:
	$(AR) $(LIB) *.o

MOVES:
	mv *.o $(OBJ_DIR)
	mv $(LIB) $(LIB_DIR)
#	mv $(BIN) $(BIN_DIR)

test:
	$(CC) $(CFLAGS) -o $(BIN) $(BIN_SOURCES) $(LDFLAGS)
	mv $(BIN) $(BIN_DIR)

.PHONY : clean
clean :
	$(RM) $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR) *~ *.o *.a
