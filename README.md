Y86-64 Processor Emulator written in C++ 

Overview:
This emulator implements the full Y86-64 ISA, simulating a simplified 64-bit processor architecture. It faithfully executes Y86-64 machine code and provides debugging capabilities to inspect processor state and memory.

Features
Complete Y86-64 ISA Implementation.
All 11 instruction types (halt, nop, moves, ALU ops, jumps, call/ret, push/pop).
Conditional moves (cmovXX) and conditional jumps (jXX).
Full memory addressing with base+displacement.
Accurate Hardware Simulation.
15 general-purpose 64-bit registers.
Condition codes (ZF, SF, OF).
64KB addressable memory space.
Little-endian byte ordering.
Debugging & Inspection Tools.
Register dump with hex and decimal values.
Memory inspection with customizable ranges.
CPU state display (PC, status, condition codes).
Support for .yo object code format.

🔧 Prerequisites
C++ Compiler: g++ (C++11 or later).
Y86 Assembler: yas (provided here in 'sim' folder).
Operating System: Linux, macOS, or Windows (with WSL).

