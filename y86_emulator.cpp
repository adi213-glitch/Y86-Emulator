#include <iostream>
#include <fstream>
#include <iomanip>
#include "y86_emulator.h"

Y86Emulator::Y86Emulator() {
    // TASK 0: Initialize your hardware.
    // 1. Resize 'memory' to MEM_SIZE and fill with 0.
    memory.resize(MEM_SIZE,0);
    // 2. Set 'pc' to 0.
    pc=0;
    // 3. Set 'status' to AOK.
    status=AOK;
    // 4. Clear all registers to 0.
    for(int i =0 ; i< 16; i++) registers[i]=0;
    // 5. Set cc.zf = true (default), sf = false, of = false.
    cc={1,0,0};
}

// The Loader
bool Y86Emulator::load_program(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {//getline returns true as long as it successfully read something. Once it hits the end of the file, it returns false, and the loop stops.
        // TASK 1: Implement the parsing logic we discussed.

        // 1. Find the address (before the ':')
        int i =0 ;
        std::string address="";
        size_t l_size = line.size();
        while(i < l_size && line[i]!=':'){
            address+= line[i++];
        }
        if(i==l_size) continue;
        i++; // found the ':'

        // 2. Find the data (after the ':')
        std::string data ="";
        while(i<l_size && line[i]!='|'){
            if(line[i]!=' ') {  // ensrue no spaces come in data
                data+=line[i];
            }
            i++;
        }
        if(data.empty()) continue;
        // 3. Convert hex strings to bytes.
        uint64_t mem_index = std::stoul(address, nullptr, 16);
        //Translation: "String TO Unsigned Long". The 16 tells it to read Hexadecimal.

        // 4. Store bytes in 'this->memory[address]'.
        for(size_t j = 0; j <data.length() ; j+=2 ){
            if(j+1 >=data.length()) break;
            std::string byteString = data.substr(j, 2);
            uint8_t val = (uint8_t)std::stoul(byteString, nullptr, 16);
            if(mem_index<MEM_SIZE){
                this->memory[mem_index]=val;
            }
            mem_index++;//prep for next byte
        }
        
    }
    return true;
}
void Y86Emulator::run() {
    // The "Game Loop": Keep running as long as status is AOK
    while (status == AOK) {
        
        //  STAGE 1: FETCH 
        
        // TODO 1: Read the instruction byte from memory at the current PC.
        
        // Safety:  we might want to check if pc < MEM_SIZE first.
        if(pc >= MEM_SIZE) status = ADR;// imem_error
        uint8_t instruction_byte = memory[pc];

        // TODO 2: Extract 'icode' (High 4 bits) and 'ifun' (Low 4 bits)
        // Hint for icode: You need to "shift" the bits to the right.

        // Hint for ifun: You need to "mask" the bits using the & operator.
        // Remember: 0xF is the mask for 1111 (4 bits).
        int icode = (instruction_byte>>4)  & 0xF;
        int ifun  = instruction_byte & 0xF;

        // 2. Control Signal: Does this instruction need a register byte?
        // Look at CS:APP Figure 4.2 (Instruction Encodings).
        // True for: rrmovq, irmovq, rmmovq, mrmovq, OPq, pushq, popq.
        // False for: halt, nop, jXX, call, ret.
        bool need_regids = false;
        switch(icode) {
            case 2: case 3: case 4: case 5: case 6: case 0xA: case 0xB:
                need_regids = true;
                break;
            default:
                need_regids = false;
        }

        // 3. Control Signal: Does this instruction need a constant valC?
        // True for: irmovq, rmmovq, mrmovq, jXX, call.
        bool need_valC =false;
        switch (icode){
        case 3: case 4 : case 5 : case 7 : case 8:
            need_valC=true;
            break;
        
        default:
            need_valC=false;
        }

        // 4. Variables to hold the fetched data
        uint64_t rA = RNONE;
        uint64_t rB = RNONE;
        uint64_t valC = 0;
        
        // Track our position in memory while reading (start 1 byte after PC)
        uint64_t current_offset = pc + 1;

        // 5. Read Register Byte (if needed)
        if (need_regids) {
            // TODO: Read the byte at 'memory[current_offset]'
            // TODO: Split it: High 4 bits -> rA, Low 4 bits -> rB
            uint8_t reg_byte = memory[current_offset];
            rA = (reg_byte>>4)  & 0xF;
            rB = reg_byte & 0xF;
            current_offset++; // Move past the register byte
        }

        // 6. Read Constant valC (if needed)
        if (need_valC) {
            // TODO: Read 8 bytes from 'memory[current_offset]'
            // Warning: Y86 is Little Endian. we must reconstruct the uint64_t.
            // Hint: can use a loop or bitwise shifts (byte | byte<<8 | byte<<16...)
            for(int i =0 ; i<8; i++){
                uint8_t single_byte = memory[current_offset+i];
                uint64_t masked_byte = (uint64_t)single_byte <<56;
                valC >>= 8;
                valC = valC | masked_byte;
            }
            
            current_offset += 8; // Move past the 8 bytes
        }

        // 7. Calculate valP (Address of next instruction)
        // In hardware, valP is literally PC + 1 + (1 if regids) + (8 if valC)
        uint64_t valP = current_offset;


        //----STAGE 2 DECODE-----

        uint64_t srcA =rA ;
        uint64_t srcB = rB;
        // set srcA
        switch (icode){
        case 2 : case 4: case 6 : case 0xA:
            srcA = rA;
            break;
        case 9 : case 0xB:
            srcA = RSP;
            break;
        default:
            srcA = RNONE;
            break;
        }
        // set srcB
        switch (icode){
        case 2 : case 4:case 5:case 6:
            srcB = rB;
            break;
        case 8 : case 9 : case 0xA: case 0xB:
            srcB = RSP;
            break;
        default:
            break;
        }
        //access reg file
        uint64_t valA = registers[srcA];
        uint64_t valB = registers[srcB];


        // -- stage 3 execute --

        uint64_t AluA = valA; 
        uint64_t AluB= valB;
        // set aluA
        switch (icode){
        case 2 :case 6:
            AluA=valA;
            break;
        case 3 : case 4 : case 5:
            AluB= valC;
            break;
        case 8 :case 0xA:
            AluA=-8;
            break;
        case 9: case 0xB:
            AluB= 8;
            break;
        default:
            break;
        }
        //set aluB
        switch (icode){
        case 4 :case 5: case 6: case 8 : case 9 : case 0xA: case 0xB:
            AluB=valB;
            break;
        default:
            AluB=0;
            break;
        }
        uint64_t valE=0;
        //set valE (alu's output)
        if(icode == 6){
            switch(ifun){
                case 0:
                    valE= valA + valB;
                    break;
                case 1 : 
                    valE = valA-valB;
                    break;
                case 2:
                    valE= valA & valB;
                    break;
                case 3:
                    valE = valA ^ valB;
                    break;
                default :
                    break;
            }
        }else{
            valE= valA+valB;
        }
        // do we need to update cc reg
        bool set_cc = (icode ==6);

        
















        // DEBUG: Print what we found so you can verify
        std::cout << "PC: 0x" << std::hex << pc 
                  << " | Byte: " << (int)instruction_byte
                  << " | icode: " << icode 
                  << " ifun: " << ifun << "\n";

        // --- STAGE 6: PC UPDATE (Temporary) ---
        // TODO 3: Increment the PC.
        // For now, just add 1 to it. Later we will calculate the real length.
        pc=valP;

        // --- STOPPING CONDITION ---
        // TODO 4: Check if we hit the Halt instruction.
        // In Y86, Halt is icode 0. If you see it, change 'status' to HLT.
        if (icode == 0) {
            status = HLT;
        }
    }
}
// Debug Helper (I'll give you this one for free so you can test)
void Y86Emulator::dump_state() {
    std::cout << "PC: 0x" << std::hex << pc << std::dec << "\n";
    std::cout << "Stat: " << status << "\n";
    std::cout << "Registers:\n";
    for (int i = 0; i < 15; i++) {
        std::cout << "%r" << i << ": 0x" << std::hex << registers[i] << "\n";
    }
    std::cout << std::dec; // Reset to decimal
}

// Main function to run the whole thing
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./y86 <file.yo>\n";
        return 1;
    }

    Y86Emulator cpu;
    if (cpu.load_program(argv[1])) {
        std::cout << "Program loaded.\n";

        cpu.run();

        cpu.dump_state(); // Check if memory loaded correctly? (Wait, this dumps registers)
        // You might want to print a specific memory address here to verify TASK 1.
    } else {
        std::cout << "Failed to load program.\n";
    }

    return 0;
}