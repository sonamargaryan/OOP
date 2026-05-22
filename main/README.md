# MyLang Compiler

A compiler for a C-like language targeting **RISC-V RV32I 32-bit Classic Architecture**.

---

## Pipeline

```
program.mylang
      │
      ▼
   Lexer          → Tokens
      │
      ▼
   Parser         → Expression AST
      │
      ▼
 ProgramParser    → Full Program AST
      │
      ▼
 IRGenerator      → Three Address Code (TAC)
      │
      ▼
  Optimizer       → Optimized TAC
  (Logical Code)    - Constant Folding
                    - Copy Propagation
                    - Dead Code Elimination
      │
      ▼
CodeGenerator     → RISC-V Assembly (.s)
      │
      ▼
 FileCreator      → Object file (.o) → Executable (ELF)
      │
      ▼
     VM           → Loader → Memory → Processor
                    (File→Code/Data/Stack→Fetch/Decode/Execute)
```

---

## Input Language

C-like syntax with the following features.

```c
// Variable declarations
int x = 10;
static int count = 0;

// Functions
int add(int a, int b) {
    int result = a + b;
    return result;
}

// Control flow
if (x < y) {
    x = x + 1;
} else {
    y = y - 1;
}

while (count < 5) {
    count = count + 1;
}

do {
    x = x - 1;
} while (x > 0);

// Entry point
int main() {
    int a = 10;
    int b = 20;
    int s = add(a, b);
    return 0;
}
```

### Supported Types
| Type | Description |
|------|-------------|
| `int` | 32-bit integer |
| `void` | No return value |

### Supported Operators
| Operator | Description |
|----------|-------------|
| `+ - * /` | Arithmetic |
| `== != < > <= >=` | Comparison |
| `=` | Assignment |

---

## Project Structure

```
compiler/
├── Token.h / Token.cpp               ← Token types
├── SymbolTable.h / SymbolTable.cpp   ← Variable tracking
├── ASTNode.h / ASTNode.cpp           ← AST node classes
├── Lexer.h / Lexer.cpp               ← Source → Tokens
├── Parser.h / Parser.cpp             ← Expression parser
├── ProgramParser.h / ProgramParser.cpp ← Full program parser
├── TACInstruction.h / TACInstruction.cpp ← TAC instruction
├── IRGenerator.h / IRGenerator.cpp   ← AST → TAC
├── Optimizer.h / Optimizer.cpp       ← TAC → Optimized TAC
├── CodeGenerator.h / CodeGenerator.cpp ← TAC → RISC-V
├── FileCreator.h / FileCreator.cpp   ← Assembly → ELF
├── Memory.h / Memory.cpp             ← Code/Data/Stack sections
├── Loader.h / Loader.cpp             ← File → Memory
├── Processor.h / Processor.cpp       ← Fetch/Decode/Execute
├── VM.h / VM.cpp                     ← VM orchestrator
├── main.cpp                          ← Entry point
├── program.mylang                    ← Example program
└── Makefile
```

---

## Build

```bash
make
```

---

## Usage

```bash
# Compile only
./compiler program.mylang

# Compile + run in VM
./compiler program.mylang output --run

# Custom output name
./compiler program.mylang myprogram --run
```

---

## Output Files

| File | Description |
|------|-------------|
| `output.s` | RISC-V Assembly |
| `output.o` | Object file |
| `output` | ELF Executable |

---

## VM Architecture

```
File → Loader → Memory → Processor
                  │
          ┌───────┼───────┐
          │       │       │
        Code    Data   Stack
        Section Section Section
       (TAC)  (globals) (frames)
                  │
              Processor
           Fetch / Decode / Execute
              IP → IP + 1
```

---

## Requirements

```bash
# Compiler
sudo apt install g++

# RISC-V toolchain (for FileCreator)
sudo apt install gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf
```

---

## Clean

```bash
make clean
```