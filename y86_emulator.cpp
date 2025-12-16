#include <iostream>
#include <fstream>
#include <iomanip>
#include "y86_emulator.h"

Y86Emulator::Y86Emulator() {
    // TASK 0: Initialize the hardware.
    // 1. Resize 'memory' to MEM_SIZE and fill with 0.
    memory.resize(MEM_SIZE,0);
    // 2. Set 'program counter' to 0.
    pc=0;
    // 3. Set 'program status' to AOK.
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
        // TASK 1: Implement  parsing logic

        // 1. Find the address (before the ':')
        int i =0 ;
        std::string address="";
        size_t l_size = line.size();
        while(i < l_size && line[i]!=':' && line[i]!='|'){
            if(line[i]!=' ') {  // ensrue no spaces come in data
                address+=line[i];
            }
            i++;
        }
        if(i==l_size || address.empty()) continue;
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
        // "String TO Unsigned Long". The 16 tells it to read Hexadecimal.

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
    // The "main Loop": Keep running as long as status is AOK
    while (status == AOK) {
        
        //  STAGE 1: FETCH 
        
        // TODO 1: Read the instruction byte from memory at the current PC.
        
        // Safety:  we might want to check if pc < MEM_SIZE first.
        if(pc >= MEM_SIZE) {
            status = ADR;// imem_error
            break;
        }
        uint8_t instruction_byte = memory[pc];

        // TODO 2: Extract 'icode' (High 4 bits) and 'ifun' (Low 4 bits)
        // for icode: we need to "shift" the bits to the right.

        // for ifun: we need to "mask" the bits using the & operator.
        // 0xF is the mask for 1111 (4 bits).
        int icode = (instruction_byte>>4)  & 0xF;
        int ifun  = instruction_byte & 0xF;
        if(icode > 0xB || icode <0) {
            status= INS;
            break;
        }
        if (icode == 0) { 
            status = HLT;
            break;
        }
        // 2. Control Signal
        // True for: rrmovq, irmovq, rmmovq, mrmovq, OPq, pushq, popq.
        // False for: halt, nop, jXX, call, ret.
        bool need_regids = false;
        switch(icode) {
            case 2: case 3: case 4: case 5: case 6: case 0xA: case 0xB:
                need_regids = true;
                break;
            default:
                need_regids = false;
                break;
        }

        // 3. Control Signal
        // True for: irmovq, rmmovq, mrmovq, jXX, call.
        bool need_valC =false;
        switch (icode){
        case 3: case 4 : case 5 : case 7 : case 8:
            need_valC=true;
            break;
        
        default:
            need_valC=false;
            break;
        }

        // 4. Variables to hold the fetched data
        uint64_t rA = RNONE;
        uint64_t rB = RNONE;
        uint64_t valC = 0;
        
        // Track our position in memory while reading (start 1 byte after PC)
        uint64_t current_offset = pc + 1;

        // 5. Read Register Byte (if needed)
        if (need_regids) {
            if(current_offset>=MEM_SIZE){// for safety
                status= ADR;
                break;
            }
            // TODO: Read the byte at 'memory[current_offset]'
            // TODO: Split it: High 4 bits -> rA, Low 4 bits -> rB
            uint8_t reg_byte = memory[current_offset];
            rA = (reg_byte>>4)  & 0xF;
            rB = reg_byte & 0xF;
            current_offset++; // Move past the register byte
        }

        // 6. Read Constant valC (if needed)
        if (need_valC) {
            if(current_offset>=MEM_SIZE){// for safety
                status= ADR;
                break;
            }
            // TODO: Read 8 bytes from 'memory[current_offset]'
            // Y86 is Little Endian. we must reconstruct the uint64_t.
            for(int i =0 ; i<8; i++){
                uint8_t single_byte = memory[current_offset+i];
                uint64_t masked_byte = (uint64_t)single_byte <<56;
                valC >>= 8;
                valC = valC | masked_byte;
            }
            
            current_offset += 8; // Move past the 8 bytes
        }

        // 7. Calculate valP (Address of next sequential instruction)
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
            srcB= RNONE;
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
            AluA= valC;
            break;
        case 8 :case 0xA:
            AluA=-8;
            break;
        case 9: case 0xB:
            AluA= 8;
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
                    valE= AluA + AluB;
                    break;
                case 1 : 
                    valE = AluB-AluA;
                    break;
                case 2:
                    valE= AluA & AluB;
                    break;
                case 3:
                    valE = AluA ^ AluB;
                    break;
                default :
                    break;
            }
        }else{
            valE= AluA+AluB;
        }
        
        // Update Condition Codes (Only for OPq)
        if (icode == 6) {
            cc.zf = (valE == 0);
            cc.sf = ((int64_t)valE < 0); // Check the sign bit (easier with cast)

            // signed Overflow Logic
            // For ADD (ifun 0): Overflow if (A>0, B>0, Res<0) OR (A<0, B<0, Res>0)
            // For SUB (ifun 1): Overflow if (A<0, B>0, Res<0) OR (A>0, B<0, Res>0)
            // It's simpler to ask: Did the sign flip unexpectedly?
            
            bool a_neg = ((int64_t)valA < 0);
            bool b_neg = ((int64_t)valB < 0);
            bool e_neg = ((int64_t)valE < 0);

            if (ifun == 0) { // ADD
                cc.of = (a_neg == b_neg) && (a_neg != e_neg);
            } else if (ifun == 1) { // SUB
                cc.of = (a_neg != b_neg) && (a_neg == e_neg);

            } else {
                cc.of = false; // Logic operations (AND/XOR) never overflow
            }
        }
        // test cc logic 
        bool cnd = 0;
        if(icode ==7 || icode ==2){
            switch (ifun) {
                case 0 :
                    cnd =1;
                    break;
                case 1:
                    cnd = (cc.sf ^ cc.of) | cc.zf;
                    break;
                case 2 :
                    cnd = cc.sf ^ cc.of;
                    break;
                case 3:
                    cnd = cc.zf;
                    break;
                case 4:
                    cnd = !cc.zf;
                    break;
                case 5:
                    cnd = !(cc.sf ^ cc.of);
                    break;
                case 6 :
                    cnd = !(cc.sf ^ cc.of) & !cc.zf;
                    break;
                default:
                    break;
            }
        }



        //---stage 4 memory---
        uint64_t mem_addr = valA;
        uint64_t mem_data = valA;
        //set mem_addr
        switch (icode){
        case 4 :case 5:case 8 : case 0xA:
            mem_addr=valE;
            break;
        case 9: case 0xB:
            mem_addr=valA;
            break;
        default:
            break;
        }
        //set mem_data
        switch (icode){
        case 4 :case 0xA:
            mem_data=valA;
            break;
        case 8:
            mem_data=valP;
            break;
        default:
            break;
        }
        bool mem_read =0, mem_write=0;
        switch(icode){
            case 5: case 9 : case 0xB:
                mem_read=1;
                break;
            default: 
                mem_read = 0; 
                break;
        }
        switch(icode){
            case 4: case 8 : case 0xA:
                mem_write=1;
                break;
            default: 
                mem_write= 0; 
                break;
        }

        // Only check memory bounds if we're actually doing memory operations
        if((mem_read || mem_write) && (mem_addr >= MEM_SIZE || mem_addr + 7 >= MEM_SIZE)) {
            status=ADR;
            break;
        }

        uint64_t valM = 0;
        // Memory Read
        if (mem_read) {
            
            for(int i =0 ; i<8; i++){
                uint8_t single_byte = memory[mem_addr+i];
                uint64_t masked_byte = (uint64_t)single_byte <<56;
                valM >>= 8;
                valM = valM | masked_byte;
            }
        }
        // Memory Write
        else if (mem_write) {
            for (int i = 0; i < 8; i++) {
                memory[mem_addr + i] = (mem_data >> (i * 8)) & 0xFF; // Extract byte and write it one by one
            }
        }
        //---stage 5 writeback---

        uint64_t dstE=rB;
        uint64_t dstM=rA; 
        //logic for dste (also handles cmovxx)
        switch(icode){
            case 2 :
                if(cnd){
                    dstE=rB;
                    break;
                }
                dstE = RNONE;
                break;
            case 3: case 6:
                dstE=rB;
                break;
            case 8: case 9:case 0xA: case 0xB:
                dstE=RSP;
                break;
            default:
                dstE= RNONE;
                break;
        }
        //logic for dstm
        switch(icode){
            case 5 : case 0xB:
                dstM= rA;
                break;
            default : 
                dstM = RNONE;
                break;
        }
        if(dstE != RNONE) registers[dstE]= valE;
        if(dstM != RNONE) registers[dstM]=valM;


        //---stage 6 pc update---

        switch(icode){
            case 8: 
                pc = valC;
                break;
            case 7: 
                if(cnd) pc = valC;
                else pc = valP;
                break;
            case 9:
                pc = valM;
                break;
            default:
                pc = valP;
                break;
        }
        
    }
}
// Debug Helper 
void Y86Emulator::dump_state() {
    std::cout << "\n========== CPU State ==========\n";
    std::cout << "PC: 0x" << std::hex << pc << std::dec << "\n";
    
    // Status with name
    std::cout << "Stat: " << status;
    switch(status) {
        case AOK: std::cout << " (AOK - Running)\n"; break;
        case HLT: std::cout << " (HLT - Halted)\n"; break;
        case ADR: std::cout << " (ADR - Address Error)\n"; break;
        case INS: std::cout << " (INS - Invalid Instruction)\n"; break;
        default: std::cout << " (Unknown)\n";
    }
    
    // Condition codes
    std::cout << "Condition Codes: ZF=" << cc.zf 
              << " SF=" << cc.sf 
              << " OF=" << cc.of << "\n";
    
    std::cout << "\nRegisters:\n";
    const char* reg_names[] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
                                "r8 ", "r9 ", "r10", "r11", "r12", "r13", "r14"};
    for (int i = 0; i < 15; i++) {
        std::cout << "  %" << reg_names[i] << ": 0x" 
                  << std::hex << std::setw(16) << std::setfill('0') 
                  << registers[i] << std::dec;
        if (registers[i] != 0) {
            std::cout << " (" << std::dec << (int64_t)registers[i] << ")";
        }
        std::cout << "\n";
    }
    std::cout << "==============================\n\n";
}
void Y86Emulator::dump_memory(uint64_t start, uint64_t end) {
    std::cout << "\n========== Memory Dump ==========\n";
    for (uint64_t addr = start; addr <= end && addr < MEM_SIZE; addr += 8) {
        std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') << addr << ": ";
        for (int i = 0; i < 8 && addr + i < MEM_SIZE; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << (int)memory[addr + i] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::dec << "=================================\n\n";
}


// Main function to run the whole thing

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./y86 <file.yo> [options]\n";
        std::cout << "Options:\n";
        std::cout << "  -m <start> <end>  : Dump memory from start to end address (hex)\n";
        std::cout << "  -m data           : Dump data area (0x000-0x100)\n";
        std::cout << "  -m all            : Dump all modified memory\n";
        std::cout << "\nExample: ./y86 test.yo -m 0x100 0x200\n";
        return 1;
    }

    Y86Emulator cpu;
    if (cpu.load_program(argv[1])) {
        std::cout << "Program loaded.\n";
        
        cpu.run();
        
        cpu.dump_state();
        
        // Parse memory dump options
        if (argc >= 3 && std::string(argv[2]) == "-m") {
            if (argc >= 4) {
                std::string option = argv[3];
                
                if (option == "data") {
                    std::cout << "\n=== Data Area ===\n";
                    cpu.dump_memory(0x000, 0x100);
                }
                else if (option == "all") {
                    std::cout << "\n=== All Memory ===\n";
                    cpu.dump_memory(0x000, 0x1000);
                }
                else if (argc >= 5) {
                    // Custom range: -m 0x100 0x200
                    uint64_t start = std::stoul(argv[3], nullptr, 16);
                    uint64_t end = std::stoul(argv[4], nullptr, 16);
                    std::cout << "\n=== Memory Range 0x" << std::hex << start 
                              << " - 0x" << end << std::dec << " ===\n";
                    cpu.dump_memory(start, end);
                }
            }
        }
        
    } else {
        std::cout << "Failed to load program.\n";
    }
    
    return 0;
}
// ./y86 test.yo                  # No memory dump
// ./y86 test.yo -m data            # Dump data area
// ./y86 test.yo -m all             # Dump all memory
// ./y86 test.yo -m 0x100 0x200     # Custom range

