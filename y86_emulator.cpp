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
        cpu.dump_state(); // Check if memory loaded correctly? (Wait, this dumps registers)
        // You might want to print a specific memory address here to verify TASK 1.
    } else {
        std::cout << "Failed to load program.\n";
    }

    return 0;
}