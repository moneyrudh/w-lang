# Everything generated (objects, the flex lexer, binaries, transpiled output) goes in build/
CC = gcc
CFLAGS = -I./include -I./src
OPTFLAGS = -O2
BUILD = build

# make install: $(DESTDIR)$(PREFIX)/bin/wlang (Homebrew passes PREFIX; packagers use DESTDIR)
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
OBJ = $(BUILD)/obj

# Sources, by stage
SRCS = src/main.c \
       src/driver/pipeline.c src/driver/toolchain.c \
       src/core/ast.c src/core/diagnostics.c \
       src/language/token_registry.c src/language/type_registry.c src/language/operators.c \
       src/lexer/layout.c \
       src/parser/parser.c src/parser/expressions.c src/parser/statements.c \
       src/parser/functions.c src/parser/control_flow.c \
       src/checker/checker.c src/checker/expressions.c src/checker/statements.c \
       src/checker/functions.c src/checker/control_flow.c src/checker/scopes.c src/checker/types.c \
       src/codegen/codegen.c src/codegen/expressions.c src/codegen/statements.c \
       src/codegen/functions.c src/codegen/control_flow.c src/codegen/formatters.c \
       src/util/map.c \
       src/runtime/wlang_runtime.c

# flex output, generated from src/lexer/lexer.l
LEXER_C = $(BUILD)/lexer.c

OBJS = $(SRCS:src/%.c=$(OBJ)/%.o) $(OBJ)/lexer.o

TARGET = $(BUILD)/wlang
ASAN_TARGET = $(BUILD)/wlang_asan

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OPTFLAGS) $(OBJS) -o $@

# -MMD writes a .d file per object so header changes trigger rebuilds
$(OBJ)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@

$(OBJ)/lexer.o: $(LEXER_C)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -MP -c $< -o $@

$(LEXER_C): src/lexer/lexer.l
	@mkdir -p $(dir $@)
	flex -o $@ $<

-include $(OBJS:.o=.d)

# Tests: the language (tests/run.sh), then the wlang command itself (tests/driver/test.sh)
test: $(TARGET)
	./tests/run.sh
	./tests/driver/test.sh

# Same tests with wlang built under AddressSanitizer + UBSan (one-shot build)
HEADERS = $(shell find include src -name '*.h')

$(ASAN_TARGET): $(SRCS) $(LEXER_C) $(HEADERS)
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined $(SRCS) $(LEXER_C) -o $@

test-asan: $(ASAN_TARGET)
	ASAN_OPTIONS=detect_leaks=0:exitcode=99 UBSAN_OPTIONS=halt_on_error=1:exitcode=99 WLANG=./$(ASAN_TARGET) ./tests/run.sh
	ASAN_OPTIONS=detect_leaks=0:exitcode=99 UBSAN_OPTIONS=halt_on_error=1:exitcode=99 WLANG=./$(ASAN_TARGET) ./tests/driver/test.sh

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/wlang

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/wlang

clean:
	rm -rf $(BUILD)

.PHONY: all clean test test-asan install uninstall
