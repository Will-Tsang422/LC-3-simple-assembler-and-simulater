/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 15:10:31
 * @LastEditors  : liuly
 * @LastEditTime : 2022-11-15 21:10:23
 * @Description  : content for samll assembler
 */

#include "assembler.h"
#include <string>

// add label and its address to symbol table
void LabelMapType::AddLabel(const std::string &str, const unsigned address)
{
    labels_.insert({str, address});
}

unsigned LabelMapType::GetAddress(const std::string &str) const
{
    if (labels_.find(str) == labels_.end())
    {
        // not found
        return -1;
    }
    return labels_.at(str);
}

std::string assembler::TranslateOprand(unsigned int current_address, std::string str, int opcode_length)
{
    // Translate the oprand
    str = Trim(str);
    auto item = label_map.GetAddress(str);
    if (item != -1)
    {
        // str is a label
        // TO BE DONE
        int gap = item - current_address - 1;
        if (opcode_length == 11)
        {
            return std::bitset<11>(gap).to_string();
        }
        else if (opcode_length == 9)
        {
            return std::bitset<9>(gap).to_string();
        }
        else
        {
            return std::bitset<6>(gap).to_string();
        }
    }
    if (str[0] == 'R')
    {
        // str is a register
        // TO BE DONE
        return std::bitset<3>(str[1] - '0').to_string();
    }
    else
    {
        // str is an immediate number
        // TO BE DONE
        if (opcode_length == 5)
        {
            return std::bitset<5>(RecognizeNumberValue(str)).to_string();
        }
        else
        {
            return std::bitset<6>(RecognizeNumberValue(str)).to_string();
        }
    }
}

std::string assembler::LineLabelSplit(const std::string &line, int current_address)
{
    // label?
    auto first_whitespace_position = line.find(' ');
    auto first_token = line.substr(0, first_whitespace_position);

    if (IsLC3Pseudo(first_token) == -1 && IsLC3Command(first_token) == -1 && IsLC3TrapRoutine(first_token) == -1)
    {
        // * This is an label
        // save it in label_map
        // TO BE DONE
        label_map.AddLabel(first_token, current_address);
        // remove label from the line
        if (first_whitespace_position == std::string::npos)
        {
            // nothing else in the line
            return "";
        }
        auto command = line.substr(first_whitespace_position + 1);
        return Trim(command);
    }
    return line;
}

// Scan #1: save commands and labels with their addresses
int assembler::firstPass(std::string &input_filename)
{
    std::string line;
    std::ifstream input_file(input_filename);
    if (!input_file.is_open())
    {
        std::cout << "Unable to open file" << std::endl;
        // @ Input file read error
        return -1;
    }

    int orig_address = -1;
    int current_address = -1;

    while (std::getline(input_file, line))
    {

        line = FormatLine(line);
        if (line.empty())
        {
            continue;
        }

        auto command = LineLabelSplit(line, current_address);
        if (command.empty())
        {
            continue;
        }

        // OPERATION or PSEUDO?
        auto first_whitespace_position = command.find(' ');
        auto first_token = command.substr(0, first_whitespace_position);

        // Special judge .ORIG and .END
        if (first_token == ".ORIG")
        {
            std::string orig_value =
                command.substr(first_whitespace_position + 1);
            orig_address = RecognizeNumberValue(orig_value);
            if (orig_address == std::numeric_limits<int>::max())
            {
                // @ Error address
                return -2;
            }
            current_address = orig_address;
            continue;
        }

        if (orig_address == -1)
        {
            // @ Error Program begins before .ORIG
            return -3;
        }

        if (first_token == ".END")
        {
            break;
        }

        // For LC3 Operation
        if (IsLC3Command(first_token) != -1 || IsLC3TrapRoutine(first_token) != -1)
        {
            commands.push_back({current_address, command, CommandType::OPERATION});
            current_address += 1;
            continue;
        }

        // For Pseudo code
        commands.push_back({current_address, command, CommandType::PSEUDO});
        auto operand = command.substr(first_whitespace_position + 1);
        if (first_token == ".FILL")
        {
            auto num_temp = RecognizeNumberValue(operand);
            if (num_temp == std::numeric_limits<int>::max())
            {
                // @ Error Invalid Number input @ FILL
                return -4;
            }
            if (num_temp > 65535 || num_temp < -65536)
            {
                // @ Error Too large or too small value  @ FILL
                return -5;
            }
            current_address += 1;
        }
        if (first_token == ".BLKW")
        {
            // modify current_address
            // TO BE DONE
            current_address += RecognizeNumberValue(operand);
        }
        if (first_token == ".STRINGZ")
        {
            // modify current_address
            // TO BE DONE
            current_address += operand.size() - 1;
        }
    }
    // OK flag
    return 0;
}

std::string assembler::TranslatePseudo(std::stringstream &command_stream)
{
    std::string pseudo_opcode;
    std::string output_line;
    command_stream >> pseudo_opcode;
    if (pseudo_opcode == ".FILL")
    {
        std::string number_str;
        command_stream >> number_str;
        output_line = NumberToAssemble(number_str);
        if (gIsHexMode)
            output_line = ConvertBin2Hex(output_line);
    }
    else if (pseudo_opcode == ".BLKW")
    {
        // Fill 0 here
        // TO BE DONE
        std::string number_str;
        command_stream >> number_str;
        int number = RecognizeNumberValue(number_str);
        for (int i = 0; i < number; i++)
        {
            std::string s = NumberToAssemble(0);
            if (gIsHexMode)
                s = ConvertBin2Hex(s);
            output_line += s;
            output_line.push_back('\n');
        }
        output_line.erase(output_line.end() - 1);
    }
    else if (pseudo_opcode == ".STRINGZ")
    {
        // Fill string here
        // TO BE DONE
        std::string str;
        command_stream >> str;
        for (auto iter = str.begin() + 1; iter != str.end() - 1; iter++)
        {
            std::string s = NumberToAssemble(int(*iter));
            if (gIsHexMode)
                s = ConvertBin2Hex(s);
            output_line += s;
            output_line.push_back('\n');
        }
        output_line += NumberToAssemble(0);
    }
    return output_line;
}

std::string assembler::TranslateCommand(std::stringstream &command_stream, unsigned int current_address)
{
    std::string opcode;
    command_stream >> opcode;
    auto command_tag = IsLC3Command(opcode);

    std::vector<std::string> operand_list;
    std::string operand;
    while (command_stream >> operand)
    {
        operand_list.push_back(operand);
    }
    auto operand_list_size = operand_list.size();

    std::string output_line;

    if (command_tag == -1)
    {
        // This is a trap routine
        command_tag = IsLC3TrapRoutine(opcode);
        output_line = kLC3TrapMachineCode[command_tag];
    }
    else
    {
        // This is a LC3 command
        switch (command_tag)
        {
        case 0:
            // "ADD"
            output_line += "0001";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            if (operand_list[2][0] == 'R')
            {
                // The third operand is a register
                output_line += "000";
                output_line +=
                    TranslateOprand(current_address, operand_list[2]);
            }
            else
            {
                // The third operand is an immediate number
                output_line += "1";
                output_line +=
                    TranslateOprand(current_address, operand_list[2], 5);
            }
            break;
        case 1:
            output_line += "0101";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            if (operand_list[2][0] == 'R')
            {
                // The third operand is a register
                output_line += "000";
                output_line += TranslateOprand(current_address, operand_list[2]);
            }
            else
            {
                // The third operand is an immediate number
                output_line += "1";
                output_line += TranslateOprand(current_address, operand_list[2], 5);
            }
            break;
            // "AND"
            // TO BE DONE
        case 2:
            output_line += "0000111";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
            // "BR"
            // TO BE DONE
        case 3:
            output_line += "0000100";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
            // "BRN"
            // TO BE DONE
        case 4:
            output_line += "0000010";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
            // "BRZ"
            // TO BE DONE
        case 5:
            output_line += "0000001";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
            // "BRP"
            // TO BE DONE
        case 6:
            output_line += "0000110";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
            // "BRNZ"
            // TO BE DONE
        case 7:
            output_line += "0000101";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
            // "BRNP"
            // TO BE DONE
        case 8:
            output_line += "0000011";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
            // "BRZP"
            // TO BE DONE
        case 9:
            // "BRNZP"
            output_line += "0000111";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 10:
            output_line += "1100000";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += "000000";
            break;
            // "JMP"
            // TO BE DONE
        case 11:
            output_line += "01001";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 11);
            break;
            // "JSR"
            // TO BE DONE
        case 12:
            output_line += "0100000";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += "000000";
            break;
            // "JSRR"
            // TO BE DONE
        case 13:
            output_line += "0010";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
            // "LD"
            // TO BE DONE
        case 14:
            output_line += "1010";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
            // "LDI"
            // TO BE DONE
        case 15:
            output_line += "0110";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += TranslateOprand(current_address, operand_list[2], 6);
            break;
            // "LDR"
            // TO BE DONE
        case 16:
            output_line += "1110";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
            // "LEA"
            // TO BE DONE
        case 17:
            output_line += "1001";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            break;
            // "NOT"
            // TO BE DONE
        case 18:
            // RET
            output_line += "1100000111000000";
            if (operand_list_size != 0)
            {
                // @ Error operand numbers
                exit(-30);
            }
            break;
        case 19:
            output_line += "1000000000000000";
            if (operand_list_size != 0)
            {
                // @ Error operand numbers
                exit(-30);
            }
            break;
            // RTI
            // TO BE DONE
        case 20:
            // ST
            output_line += "0011";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 21:
            output_line += "1011";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
            // STI
            // TO BE DONE
        case 22:
            output_line += "0111";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += TranslateOprand(current_address, operand_list[2], 6);
            break;
            // STR
            // TO BE DONE
        case 23:
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            command_tag = operand_list[0][2] - '0';
            output_line = kLC3TrapMachineCode[command_tag];
            break;
            // TRAP
            // TO BE DONE
        default:
            // Unknown opcode
            // @ Error
            break;
        }
    }

    if (gIsHexMode)
        output_line = ConvertBin2Hex(output_line);

    return output_line;
}

int assembler::secondPass(std::string &output_filename)
{
    // Scan #2:
    // Translate
    std::ofstream output_file;
    // Create the output file
    output_file.open(output_filename);
    if (!output_file)
    {
        // @ Error at output file
        return -20;
    }

    for (const auto &command : commands)
    {
        const unsigned address = std::get<0>(command);
        const std::string command_content = std::get<1>(command);
        const CommandType command_type = std::get<2>(command);
        auto command_stream = std::stringstream(command_content);

        if (command_type == CommandType::PSEUDO)
        {
            // Pseudo
            output_file << TranslatePseudo(command_stream) << std::endl;
        }
        else
        {
            // LC3 command
            output_file << TranslateCommand(command_stream, address) << std::endl;
        }
    }

    // Close the output file
    output_file.close();
    // OK flag
    return 0;
}

// assemble main function
int assembler::assemble(std::string &input_filename, std::string &output_filename)
{
    auto first_scan_status = firstPass(input_filename);
    if (first_scan_status != 0)
    {
        return first_scan_status;
    }
    auto second_scan_status = secondPass(output_filename);
    if (second_scan_status != 0)
    {
        return second_scan_status;
    }
    // OK flag
    return 0;
}
