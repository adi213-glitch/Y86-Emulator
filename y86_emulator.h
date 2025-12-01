#ifndef Y86_EMULATOR_H
#define Y86_EMULATOR_H

#include <vector>
#include <cstdint> // <--- This library gives us the specific integer types we need
#include <string>


const int MEM_SIZE = 0x10000;

// --- SYSTEM TYPES  ---

// 1. Register IDs
// 'enum' gives English names to the numbers 0-14.
// can write 'reg[RAX]' instead of 'reg[0]'.

enum RegID {
    RAX=0, RCX =1, RDX =2, RBX =3, RSP=4,RBP=5,RSI=6,RDI=7,R8=8,R9=9,R10=10,R11=11,R12=12,R13=13,R14=14,RNONE=0xF
};
// 2. Status Codes
enum Stat{
    AOK = 1, // All OK 
    HLT = 2, // Halt instruction hlt
    ADR = 3, // Invalid Address 
    INS = 4  // Invalid Instruction
};
// 3. Condition Codes
// bundle the 3 flags into a simple struct.
struct ConditionCodes {
    bool zf; // Zero Flag
    bool sf; // Sign Flag
    bool of; // Overflow Flag
};

// --- THE EMULATOR CLASS ---
class Y86Emulator {
private:
    // == HARDWARE STATE ==
    
    // Memory: A vector of bytes. 
    // uint8_t = "Unsigned Integer 8-bit" (exactly 1 byte).
    // could also use vector<char>, but uint8_t is more precise for hardware.
    std::vector<uint8_t> memory; 

    // Register File: Array of 16 values.
    // uint64_t = "Unsigned Integer 64-bit".
    // We need 64 bits because Y86-64 registers hold 64-bit values.
    uint64_t registers[16]; 

    // Program Counter
    uint64_t pc;

    // Processor Status
    Stat status;

    // Condition Flags
    ConditionCodes cc;

public:
    // Constructor: Initializes the machine (clears memory, resets PC)
    Y86Emulator();

    // == THE LOADER  ==
    // Reads a .yo file and fills the 'memory' vector.
    // Returns true if successful, false if file error.
    bool load_program(const std::string& filename);

    // == THE ENGINE  ==
    // Runs the processor loop until status is not AOK.
    void run();
    
    // Debug helper: Print current state of registers and memory
    void dump_state();

    void dump_memory(uint64_t start, uint64_t end);

};

#endif