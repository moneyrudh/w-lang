# Everything generated (objects, the flex lexer, binaries, transpiled output) goes in build/
CFLAGS = -I./include -I./src
OPTFLAGS = -O2
# our own code must build without warnings; flex's generated scanner gets one exemption
WARNFLAGS = -Wall -Wextra -Werror
LEXER_WARNFLAGS = $(WARNFLAGS) -Wno-sign-compare
BUILD = build

# make install: $(DESTDIR)$(PREFIX)/bin/wlang (Homebrew passes PREFIX; packagers use DESTDIR)
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
OBJ = $(BUILD)/obj

# Sources, by stage
SRCS = src/main.c \
       src/driver/pipeline.c src/driver/toolchain.c \
       src/core/ast.c src/core/diagnostics.c src/core/user_types.c \
       src/language/token_registry.c src/language/type_registry.c src/language/operators.c \
       src/lexer/layout.c \
       src/parser/parser.c src/parser/expressions.c src/parser/statements.c \
       src/parser/functions.c src/parser/control_flow.c \
       src/checker/checker.c src/checker/expressions.c src/checker/statements.c \
       src/checker/functions.c src/checker/control_flow.c src/checker/scopes.c src/checker/types.c src/checker/pods.c \
       src/codegen/codegen.c src/codegen/expressions.c src/codegen/statements.c \
       src/codegen/functions.c src/codegen/control_flow.c src/codegen/formatters.c src/codegen/strings.c src/codegen/arithmetic.c src/codegen/pods.c \
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
	$(CC) $(CFLAGS) $(OPTFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

$(OBJ)/lexer.o: $(LEXER_C)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OPTFLAGS) $(LEXER_WARNFLAGS) -MMD -MP -c $< -o $@

$(LEXER_C): src/lexer/lexer.l
	@mkdir -p $(dir $@)
	flex -o $@ $<

-include $(OBJS:.o=.d)

HEADERS = $(shell find include src -name '*.h')

# Tests: the language (tests/run.sh), unit tests of pure rules (tests/unit/), then the wlang
# command itself (tests/driver/test.sh)
UNIT_TESTS = $(BUILD)/unit/type_rules

UNIT_SRCS = src/checker/types.c src/language/operators.c src/core/user_types.c

$(BUILD)/unit/type_rules: tests/unit/type_rules.c $(UNIT_SRCS) $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(WARNFLAGS) tests/unit/type_rules.c $(UNIT_SRCS) -o $@

unit: $(UNIT_TESTS)
	@for t in $(UNIT_TESTS); do ./$$t || exit 1; done

test: $(TARGET) unit
	./tests/run.sh
	./tests/driver/test.sh

# Same tests with wlang built under AddressSanitizer + UBSan (one-shot build)

$(ASAN_TARGET): $(SRCS) $(LEXER_C) $(HEADERS)
	$(CC) $(CFLAGS) -g -fsanitize=address,undefined $(SRCS) $(LEXER_C) -o $@

# leak checking needs LeakSanitizer, which macOS on Apple Silicon lacks: CI turns it on (ASAN_LEAKS=1) on Linux
ASAN_LEAKS = 0
ASAN_ENV = ASAN_OPTIONS=detect_leaks=$(ASAN_LEAKS):exitcode=99 UBSAN_OPTIONS=halt_on_error=1:exitcode=99

test-asan: $(ASAN_TARGET)
	$(ASAN_ENV) WLANG=./$(ASAN_TARGET) PROGRAM_CFLAGS="-g -fsanitize=address,undefined" ./tests/run.sh
	$(ASAN_ENV) WLANG=./$(ASAN_TARGET) ./tests/driver/test.sh

# clang's static analyzer over our own sources (not flex's output); any finding fails
ANALYZER = clang
analyze:
	@failed=0; for f in $(SRCS); do \
		$(ANALYZER) --analyze $(CFLAGS) -Xclang -analyzer-werror -Xclang -analyzer-output=text -o /dev/null $$f || failed=1; \
	done; \
	if [ $$failed = 1 ]; then echo "analyze: findings above"; exit 1; fi; echo "analyze: no findings"

# mutation fuzzer (tests/fuzz/fuzz.py) against the sanitizer build; FUZZ_RUNS / FUZZ_SEED to tune
FUZZ_RUNS = 2000
fuzz: $(ASAN_TARGET)
	$(ASAN_ENV) python3 tests/fuzz/fuzz.py --wlang ./$(ASAN_TARGET) --runs $(FUZZ_RUNS) $(if $(FUZZ_SEED),--seed $(FUZZ_SEED))

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/wlang

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/wlang

clean:
	rm -rf $(BUILD)

.PHONY: all clean test test-asan unit analyze fuzz install uninstall
