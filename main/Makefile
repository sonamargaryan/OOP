# ════════════════════════════════════════════════════════════
#  Makefile — MyLang Compiler
#  RISC-V RV32I 32-bit Classic Architecture
# ════════════════════════════════════════════════════════════

CXX      = g++
CXXFLAGS = -std=c++14 -Wall -Wextra

TARGET   = compiler

SRCS     = main.cpp \
           Token.cpp \
           SymbolTable.cpp \
           ASTNode.cpp \
           Lexer.cpp \
           Parser.cpp \
           ProgramParser.cpp \
           TACInstruction.cpp \
           IRGenerator.cpp \
           Optimizer.cpp \
           CodeGenerator.cpp \
           FileCreator.cpp \
           Memory.cpp \
           Loader.cpp \
           Processor.cpp \
           VM.cpp

OBJS     = $(SRCS:.cpp=.o)

# ── Default target ─────────────────────────────────────────
all: $(TARGET)

# ── Link ───────────────────────────────────────────────────
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "✓ Build successful → ./$(TARGET)"

# ── Compile each .cpp → .o ─────────────────────────────────
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ── Run compiler only ──────────────────────────────────────
run: $(TARGET)
	./$(TARGET) program.mylang

# ── Run compiler + VM ──────────────────────────────────────
run-vm: $(TARGET)
	./$(TARGET) program.mylang output --run

# ── Clean ──────────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(TARGET) output output.s output.o
	@echo "✓ Cleaned"

.PHONY: all run run-vm clean