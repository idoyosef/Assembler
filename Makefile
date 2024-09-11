# Define the source and object files
SRC_DIR := src
OBJ_DIR := obj
INCLUDE_DIR := include
FILES_SOURCE := main.c passes.c tokens.c macros.c
OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(FILES_SOURCE))

# Define the compiler and flags
CC := gcc
CFLAGS := -g -Wall -ansi -pedantic -I $(INCLUDE_DIR)

# Target to build the final executable
all: assembler

# Rule to link the object files into the final executable
assembler: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Pattern rule to compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean rule to remove generated files
clean:
	rm -f assembler $(OBJ_DIR)/*.o *.ent *.ext *.ob *.am

.PHONY: all clean
