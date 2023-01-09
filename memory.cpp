/*
 * @Author       : Chivier Humber
 * @Date         : 2021-09-15 21:15:24
 * @LastEditors  : Chivier Humber
 * @LastEditTime : 2021-11-23 16:08:51
 * @Description  : file content
 */
#include "common.h"
#include "memory.h"

namespace virtual_machine_nsp {
    void memory_tp::ReadMemoryFromFile(std::string filename, int beginning_address) {
        // Read from the file
        // TO BE DONE
        std::ifstream input_file(filename);
        if (!input_file.is_open())
        {
            std::cout << "Unable to open file" << std::endl;
            // @ Input file read error
            exit(EXIT_FAILURE);
        }
        std::string line;
        while (getline(input_file, line))
        {
            int16_t num = 0;
            for (int i = 0; i < 16; i++)
            {
                num += (line[i] - '0') * pow(2, 15 - i);
            }
            memory_tp::memory[beginning_address] = num;
            beginning_address++;
        }
    }

    int16_t memory_tp::GetContent(int address) const {
        // get the content
        // TO BE DONE
        return memory_tp::memory[address];
    }

    int16_t& memory_tp::operator[](int address) {
        // get the content
        // TO BE DONE
        return memory_tp::memory[address];
    }    
}; // virtual machine namespace
