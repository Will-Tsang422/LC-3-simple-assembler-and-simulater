/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 14:36:39
 * @LastEditors  : liuly
 * @LastEditTime : 2022-11-15 21:12:51
 * @Description  : header file for small assembler
 */

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cstring>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <bitset>

const int kLC3LineLength = 16;

extern bool gIsErrorLogMode;
extern bool gIsHexMode;

const std::vector<std::string> kLC3Pseudos({
    ".ORIG",
    ".END",
    ".STRINGZ",
    ".FILL",
    ".BLKW",
});

const std::vector<std::string> kLC3Commands({
    "ADD",   // 00: "0001" + reg(line[1]) + reg(line[2]) + op(line[3])
    "AND",   // 01: "0101" + reg(line[1]) + reg(line[2]) + op(line[3])
    "BR",    // 02: "0000111" + pcoffset(line[1],9)
    "BRN",   // 03: "0000100" + pcoffset(line[1],9)
    "BRZ",   // 04: "0000010" + pcoffset(line[1],9)
    "BRP",   // 05: "0000001" + pcoffset(line[1],9)
    "BRNZ",  // 06: "0000110" + pcoffset(line[1],9)
    "BRNP",  // 07: "0000101" + pcoffset(line[1],9)
    "BRZP",  // 08: "0000011" + pcoffset(line[1],9)
    "BRNZP", // 09: "0000111" + pcoffset(line[1],9)
    "JMP",   // 10: "1100000" + reg(line[1]) + "000000"
    "JSR",   // 11: "01001" + pcoffset(line[1],11)
    "JSRR",  // 12: "0100000"+reg(line[1])+"000000"
    "LD",    // 13: "0010" + reg(line[1]) + pcoffset(line[2],9)
    "LDI",   // 14: "1010" + reg(line[1]) + pcoffset(line[2],9)
    "LDR",   // 15: "0110" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "LEA",   // 16: "1110" + reg(line[1]) + pcoffset(line[2],9)
    "NOT",   // 17: "1001" + reg(line[1]) + reg(line[2]) + "111111"
    "RET",   // 18: "1100000111000000"
    "RTI",   // 19: "1000000000000000"
    "ST",    // 20: "0011" + reg(line[1]) + pcoffset(line[2],9)
    "STI",   // 21: "1011" + reg(line[1]) + pcoffset(line[2],9)
    "STR",   // 22: "0111" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "TRAP"   // 23: "11110000" + h2b(line[1],8)
});

const std::vector<std::string> kLC3TrapRoutine({
    "GETC",  // x20
    "OUT",   // x21
    "PUTS",  // x22
    "IN",    // x23
    "PUTSP", // x24
    "HALT"   // x25
});

const std::vector<std::string> kLC3TrapMachineCode({"1111000000100000",
                                                    "1111000000100001",
                                                    "1111000000100010",
                                                    "1111000000100011",
                                                    "1111000000100100",
                                                    "1111000000100101"});

enum CommandType
{
    OPERATION,
    PSEUDO
};

static inline void SetErrorLogMode(bool error)
{
    gIsErrorLogMode = error;
}

static inline void SetHexMode(bool hex)
{
    gIsHexMode = hex;
}

// A warpper class for std::unorderd_map in order to map label to its address
class LabelMapType
{
private:
    std::unordered_map<std::string, unsigned> labels_;

public:
    void AddLabel(const std::string &str, unsigned address);
    unsigned GetAddress(const std::string &str) const;
};

static inline int IsLC3Pseudo(const std::string &str)
{
    int index = 0;
    for (const auto &command : kLC3Pseudos)
    {
        if (str == command)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int IsLC3Command(const std::string &str)
{
    int index = 0;
    for (const auto &command : kLC3Commands)
    {
        if (str == command)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int IsLC3TrapRoutine(const std::string &str)
{
    int index = 0;
    for (const auto &trap : kLC3TrapRoutine)
    {
        if (str == trap)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

static inline int CharToDec(const char &ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }
    return -1;
}

static inline char DecToChar(const int &num)
{
    if (num <= 9)
    {
        return num + '0';
    }
    return num - 10 + 'A';
}

// trim string from both left & right
static inline std::string &Trim(std::string &s)
{
    // TO BE DONE
    for (auto iter = s.begin(); isblank(*iter); iter++)
    {
        s.erase(iter--);
    }
    for (auto iter = s.end() - 1; isblank(*iter); iter--)
    {
        s.erase(iter);
    }
    return s;
}
// Format one line from asm file, do the following:
// 1. remove comments
// 2. convert the line into uppercase
// 3. replace all commas with whitespace (for splitting)
// 4. replace all "\t\n\r\f\v" with whitespace
// 5. remove the leading and trailing whitespace chars
// Note: please implement function Trim first
static std::string FormatLine(const std::string &line)
{
    // TO BE DONE
    std::string s = line;
    if (s.find(';') != s.npos)
    {
        s.erase(s.begin() + s.find(';'), s.end());
    }
    s = Trim(s);
    for (auto iter = s.begin(); iter != s.end(); iter++)
    {
        if (*iter == ',')
            *iter = ' ';
        if (*iter >= 'a' && *iter <= 'z')
            *iter = *iter + 'A' - 'a';
    }
    return s;
}

static int RecognizeNumberValue(const std::string &str)
{
    // Convert string `str` into a number and return it
    // TO BE DONE
    if (str[0] == '#')
    {
        return atoi(str.substr(1).c_str());
    }
    else
    {
        int number = 0, index = 1;
        for (auto iter = str.end() - 1; iter != str.begin(); iter--)
        {
            if (*iter >= '0' && *iter <= '9')
            {
                number += index * (*iter - '0');
            }
            else
            {
                number += index * (*iter - 'A' + 10);
            }
            index *= 16;
        }
        return number;
    }
}

static std::string NumberToAssemble(const int &number)
{
    // Convert `number` into a 16 bit binary string
    // TO BE DONE
    return std::bitset<16>(number).to_string();
}

static std::string NumberToAssemble(const std::string &number)
{
    // Convert `number` into a 16 bit binary string
    // You might use `RecognizeNumberValue` in this function
    // TO BE DONE
    return std::bitset<16>(RecognizeNumberValue(number)).to_string();
}

static std::string ConvertBin2Hex(const std::string &bin)
{
    // Convert the binary string `bin` into a hex string
    // TO BE DONE
    std::string s;
    for (int i = 0; i < bin.size(); i += 4)
    {
        std::string tmp = bin.substr(i, 4);
        if (!tmp.compare("0000"))
            s.push_back('0');
        else if (!tmp.compare("0001"))
            s.push_back('1');
        else if (!tmp.compare("0010"))
            s.push_back('2');
        else if (!tmp.compare("0011"))
            s.push_back('3');
        else if (!tmp.compare("0100"))
            s.push_back('4');
        else if (!tmp.compare("0101"))
            s.push_back('5');
        else if (!tmp.compare("0110"))
            s.push_back('6');
        else if (!tmp.compare("0111"))
            s.push_back('7');
        else if (!tmp.compare("1000"))
            s.push_back('8');
        else if (!tmp.compare("1001"))
            s.push_back('9');
        else if (!tmp.compare("1010"))
            s.push_back('A');
        else if (!tmp.compare("1011"))
            s.push_back('B');
        else if (!tmp.compare("1100"))
            s.push_back('C');
        else if (!tmp.compare("1101"))
            s.push_back('D');
        else if (!tmp.compare("1110"))
            s.push_back('E');
        else
            s.push_back('F');
    }
    return s;
}

class assembler
{
    using Commands = std::vector<std::tuple<unsigned, std::string, CommandType>>;

private:
    LabelMapType label_map;
    Commands commands;

    static std::string TranslatePseudo(std::stringstream &command_stream);
    std::string TranslateCommand(std::stringstream &command_stream, unsigned int current_address);
    std::string TranslateOprand(unsigned int current_address, std::string str, int opcode_length = 3);
    std::string LineLabelSplit(const std::string &line, int current_address);
    int firstPass(std::string &input_filename);
    int secondPass(std::string &output_filename);

public:
    int assemble(std::string &input_filename, std::string &output_filename);
};
