# Y86-64 Processor Emulator

*A complete instruction-set simulator written in C++*

## Overview:
This emulator implements the full Y86-64 ISA, simulating a simplified 64-bit processor architecture. It faithfully executes Y86-64 machine code and provides debugging capabilities to inspect processor state and memory.
Also look below in this Readme.md for learning a little bit of Y86-64 (Or I should say a very simplistic model of x86 ;) 

## Features

**Complete Y86-64 ISA Implementation**
- All 11 instruction types (halt, nop, moves, ALU ops, jumps, call/ret, push/pop)
- Conditional moves (cmovXX) and conditional jumps (jXX)
- Full memory addressing with base+displacement

**Accurate Hardware Simulation**
- 15 general-purpose 64-bit registers
- Condition codes (ZF, SF, OF)
- 64KB addressable memory space
- Little-endian byte ordering

**Debugging & Inspection Tools**
- Register dump with hex and decimal values
- Memory inspection with customizable ranges
- CPU state display (PC, status, condition codes)
- Support for `.yo` object code format

## Prerequisites
C++ Compiler: g++ (C++11 or later)
Y86 Assembler: yas (provided here)
Operating System: Linux, macOS, or Windows (with WSL)

## Installation
Clone or download the project:


`git clone <https://github.com/adi213-glitch/Y86-Emulator>`
`cd Y86-Emulator`

### Compile the emulator:
`g++ y86_emulator.cpp -o y86`

### Verify installation:
`./y86`

You should see usage instructions.


## Quick Start

### Step 1: Write Y86 Assembly Code
Create a file test.ys:
#### Simple Y86 program - adds two numbers

    .pos 0
    irmovq stack, %rsp      # Initialize stack
    call main
    halt
main:
    irmovq $10, %rax        # rax = 10
    irmovq $20, %rbx        # rbx = 20
    addq %rbx, %rax         # rax = rax + rbx = 30
    ret
    .pos 0x100
stack:


### Step 2: Assemble to Object Code

`yas test.ys`

#### This creates test.yo (object code file).

### Step 3: Run the Emulator

`./y86 test.yo`

#### Output:


Program loaded.

========== CPU State ==========
PC: 0x13
Stat: 2 (HLT - Halted)
Condition Codes: ZF=0 SF=0 OF=0

Registers:
  %rax: 0x000000000000001e (30)
  %rbx: 0x0000000000000014 (20)
  %rsp: 0x0000000000000100 (256)
  ...
==============================


## Usage Guide
### Basic Usage

`./y86 <file.yo> [options]`

### Command-Line Options

| Option | Description | Example |
|--------|-------------|---------|
| No options | Execute program and display final state | `./y86 test.yo` |
| `-m data` | Dump data area (0x000-0x100) | `./y86 test.yo -m data` |
| `-m all` | Dump all memory (0x000-0x1000) | `./y86 test.yo -m all` |
| `-m <start> <end>` | Dump custom memory range (hex) | `./y86 test.yo -m 0x100 0x200` |


## Examples

### Run without memory inspection
`./y86 program.yo`

### Inspect data area after execution
`./y86 program.yo -m data`

### Inspect specific memory range
`./y86 program.yo -m 0x18 0x38`

### Inspect all modified memory
`./y86 program.yo -m all`


## Writing Y86 Assembly Programs
## Instruction Set
### Data Movement

irmovq $V, rB         # Immediate to register
rrmovq rA, rB         # Register to register
mrmovq D(rB), rA      # Memory to register
rmmovq rA, D(rB)      # Register to memory

### Arithmetic & Logic (OPq)

addq rA, rB           # rB = rB + rA
subq rA, rB           # rB = rB - rA
andq rA, rB           # rB = rB & rA
xorq rA, rB           # rB = rB ^ rA

### Conditional Moves

cmovle rA, rB         # Move if <=
cmovl  rA, rB         # Move if <
cmove  rA, rB         # Move if ==
cmovne rA, rB         # Move if !=
cmovge rA, rB         # Move if >=
cmovg  rA, rB         # Move if >

### Jumps

jmp   Dest            # Unconditional
jle   Dest            # Jump if <=
jl    Dest            # Jump if <
je    Dest            # Jump if ==
jne   Dest            # Jump if !=
jge   Dest            # Jump if >=
jg    Dest            # Jump if >

### Stack Operations

pushq rA              # Push register onto stack
popq  rA              # Pop stack into register
call  Dest            # Call procedure
ret                   # Return from procedure

### Control

halt                  # Stop execution
nop                   # No operation

### Registers

| Register | Purpose |
|----------|---------|
| %rax - %r14 | General purpose (15 registers) |
| %rsp | Stack pointer (convention) |
| %rbp | Base pointer (convention) |


### Assembly Directives

.pos <address>        # Set code/data position
.align <bytes>        # Align to byte boundary
.quad <value>         # 8-byte constant


## Complete Example Program

#### Array sum program
    .pos 0
init:
    irmovq stack, %rsp      # Set up stack
    call main
    halt

#### Data: array of 4 elements
    .pos 0x20
    .align 8
array:
    .quad 10
    .quad 20
    .quad 30
    .quad 40

main:
    irmovq array, %rdi      # Array pointer
    irmovq $4, %rsi         # Array length
    call sum
    ret

#### sum(array, length) - returns sum in %rax
sum:
    xorq %rax, %rax         # sum = 0
    irmovq $8, %r8          # Element size
loop:
    andq %rsi, %rsi         # Test length
    je done
    mrmovq (%rdi), %r9      # Load element
    addq %r9, %rax          # Add to sum
    addq %r8, %rdi          # Next element
    irmovq $-1, %r10
    addq %r10, %rsi         # length--
    jmp loop
done:
    ret

    .pos 0x200
stack:

### Assemble and run:

`yas array_sum.ys`
`./y86 array_sum.yo`

### Expected output: %rax = 0x64 (100 in decimal)

##  Status Codes

| Code | Name | Meaning |
|------|------|---------|
| AOK (1) | Normal | Program running normally |
| HLT (2) | Halt | Halt instruction executed |
| ADR (3) | Address Error | Invalid memory address |
| INS (4) | Invalid Instruction | Unknown instruction code |


## Debugging Tips
Check register values: Look at the register dump after execution

Inspect memory: Use -m options to see data/code

Verify object code: Check the .yo file for correct encoding

Trace execution: Add multiple halt points in your code

Start simple: Test each instruction type individually

## References
CS:APP Book: Computer Systems: A Programmer's Perspective (3rd Edition)

Chapter 4: Processor Architecture

Figures 4.2, 4.7, 4.18, 4.23

Y86-64 ISA: CS:APP Section 4.1

Simulator Guide: CS:APP3e Guide to Y86-64 Processor Simulators

## Contributing

### Suggestions and improvements are welcome! Areas for enhancement:

Step-through debugging mode

Instruction count/cycle statistics

GUI visualization

Pipeline simulation

## License
Educational project based on CS:APP materials. For academic use.

## Author
Aditya - IIT Delhi
Systems Programming Project - (September 2025- December 2025 )