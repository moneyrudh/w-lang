# Compiler and flags
CC = gcc
CFLAGS = -I./include

# Source files directly
SRCS = src/lexer.c src/ast.c src/gen.c src/parser.c src/symbol_table.c src/main.c

# Target executable
TARGET = transpiler

# Default target
all: $(TARGET)

# Direct compilation without intermediate object files
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Clean
clean:
	rm -f $(TARGET)

.PHONY: all clean