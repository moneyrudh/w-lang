# Compiler and flags
CC = gcc
CFLAGS = -I./include

# Source files directly
SRCS = src/lexer.c src/ast.c src/gen.c src/parser.c src/symbol_table.c src/operator_utils.c src/main.c \
       src/data_structures/map.c src/transpiler/type_registry.c src/transpiler/token_registry.c \
       src/codegen/formatters.c src/runtime/wlang_runtime.c

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