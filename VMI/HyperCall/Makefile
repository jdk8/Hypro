SHELL = /bin/sh

LIB_DIR = $(CURDIR)/lib/
BIN_DIR = $(CURDIR)/bin/
OBJ_DIR = $(CURDIR)/obj/

TOP      = $(CURDIR)/../..
INCLUDES = -I$(TOP)/tools/common -I$(TOP)
CFLAG    = -g # -g means add debug inform, -s means remove debug inform
CFLAGS   = $(INCLUDES) $(CFLAG)
LDFLAGS  = -L$(TOP)/tools/common/lib -lCallVmm
CC       = cc
AR       = ar r
RM       = rm -rf

$(shell mkdir -p $(LIB_DIR))
$(shell mkdir -p $(BIN_DIR))
$(shell mkdir -p $(OBJ_DIR))

SOURCES = $(wildcard *.c)

#specific bin source file
BIN_SOURCES = $(notdir $(CURDIR))Test.c
BIN_OBJECTS = $(BIN_SOURCES:%.c=%.o)

LIB_SOURCES = $(filter-out $(BIN_SOURCES), $(SOURCES))
LIB_OBJECTS = $(filter-out $(BIN_OBJECTS), $(SOURCES:%.c=%.o))

LIB = lib$(notdir $(CURDIR)).a
BIN = $(BIN_SOURCES:%.c=%)

.PHONY : all
all : OBJS LIBS BINS MOVES

# -c means only generate object files
OBJS:
	$(CC) $(CFLAGS) -c $(SOURCES)

LIBS:
	$(AR) $(LIB) $(LIB_OBJECTS)

BINS:
	$(CC) $(CFLAGS) -o $(BIN) $(BIN_OBJECTS) $(LIB) $(LDFLAGS)

MOVES:
	mv $(BIN_OBJECTS) $(LIB_OBJECTS) $(OBJ_DIR)
	mv $(LIB) $(LIB_DIR)
	mv $(BIN) $(BIN_DIR)

.PHONY : clean
clean :
	$(RM) $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR) *~ *.o *.a
