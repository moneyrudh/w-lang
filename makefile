# Compiler and flags
CC = gcc
CFLAGS = -I./include

# Source files directly
SRCS = src/lexer.c src/ast.c src/gen.c src/parser.c src/symbol_table.c src/operator_utils.c src/main.c \
       src/data_structures/map.c src/transpiler/type_registry.c src/transpiler/token_registry.c \
       src/codegen/formatters.c src/codegen/control_flow.c src/parser/control_flow.c \
       src/runtime/wlang_runtime.c

# Target executable
TARGET = transpiler

# Default target
all: $(TARGET)

# Direct compilation without intermediate object files
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# Tests (see tests/run.sh)
test: $(TARGET)
	./tests/run.sh

# Same tests with the transpiler built under AddressSanitizer + UBSan
ASAN_TARGET = transpiler_asan

$(ASAN_TARGET): $(SRCS)
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined $(SRCS) -o $(ASAN_TARGET)

test-asan: $(ASAN_TARGET)
	ASAN_OPTIONS=detect_leaks=0:exitcode=99 UBSAN_OPTIONS=halt_on_error=1:exitcode=99 TRANSPILER=./$(ASAN_TARGET) ./tests/run.sh

# Clean
clean:
	rm -f $(TARGET) $(ASAN_TARGET)

.PHONY: all clean test test-asan