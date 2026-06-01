# ════════════════════════════════════════════════════════════
#  Makefile — MyLang Compiler + VM
#  RISC-V RV32I 32-bit Classic Architecture
# ════════════════════════════════════════════════════════════

CXX      = g++
CXXFLAGS = -std=c++14 -Wall -Wextra

COMMON_SRCS = Token.cpp \
              Symboltable.cpp \
              Astnode.cpp \
              Lexer.cpp \
              Parser.cpp \
              Programparser.cpp \
              Tacinstruction.cpp \
              Irgenerator.cpp \
              Optimizer.cpp

COMPILER_SRCS = $(COMMON_SRCS) \
                Binaryencoder.cpp \
                Codegenerator.cpp \
                Filecreator.cpp \
                main_compiler.cpp

VM_SRCS = $(COMMON_SRCS) \
          Memory.cpp \
          Loader.cpp \
          Processor.cpp \
          Vm.cpp \
          main_vm.cpp

COMPILER_OBJS = $(COMPILER_SRCS:.cpp=.o)
VM_OBJS       = $(VM_SRCS:.cpp=.o)

ALL_OBJS = $(sort $(COMPILER_OBJS) $(VM_OBJS))

# ── Default: build both ────────────────────────────────────
all: compiler vm

compiler: $(COMPILER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✓ compiler built → ./compiler"

vm: $(VM_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✓ vm built → ./vm"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ── Run compiler ───────────────────────────────────────────
run-compiler: compiler
	./compiler program.mylang output

# ── Run VM ─────────────────────────────────────────────────
run-vm: vm
	./vm program.mylang --verbose

# ── Clean ──────────────────────────────────────────────────
clean:
	rm -f $(ALL_OBJS) compiler vm output output.s output.o output.bin
	@echo "✓ Cleaned"

.PHONY: all compiler vm run-compiler run-vm clean