#include <iostream>
#include <stdlib.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <map>

using namespace std;

/* An intruction memory is an 8-bit representation of an hex machine code */
class InstructionMemory {
private:
   uint8_t im[8] = {};

public:
   /* Initializes the instruction memory using an input binary string */
   InstructionMemory(string inst) {
      int index = 0;

      for (auto i : inst) {
         if (i == '1') {
            im[index] = 1;
         }
         else {
            im[index] = 0;
         }
         index++;
      }
   }

   /* Returns 8-bit array instruction memory */
   uint8_t* getIM() { return im; }

   /* Converts the uint8_t instruction into string */
   string toString() {
      string str = "";

      for (int i = 0; i < 8; i++) {
         str += to_string(im[i]);
      }
      return str;
   }
};

/*
   This object is used to represent assembly intructions. It uses maps to
   map any 2-bit binary input to its opCode or operand.
   The constructor converts an 8-bit binary instruction to an assembly 
   instruction.
*/
class DisassemblerInstructions {
private:
   string word[4];
   map<string, string> opCodeMap = {
      {"00", "add"}, {"01", "and"}, {"10", "not"}, {"11", "bnz"}
   };

   map<string, string> registerMap = {
      {"00", "r0"}, {"01", "r1"}, {"10", "r2"}, {"11", "r3"}
   };

public:
   DisassemblerInstructions(string line) {
      int wordIndex = 0;
      string strAddress = "";

      for (int i = 0; i < line.size(); i+=2) {
         string s =  "";
         s += line[i];
         s += line[i + 1];
         strAddress += s;

         if (i == 0) {
            word[wordIndex] = opCodeMap[s];
         }
         else {
            word[wordIndex] = registerMap[s];
         }
         wordIndex++;
      }

      if (word[0] == "not") {
         string temp = word[1];
         word[1] = word[3];
         word[2] = temp;
         word[3] = "";
      }
      else if (word[0] == "bnz") {
         int address = 0, power = 5;
         strAddress.erase(0, 2);

         for (int i = 0; i < strAddress.size(); i++) {
            if (strAddress[i] == '1') { address += pow(2, power); }
            power--;
         }
         
         word[1] = to_string(address);
         word[2] = "";
         word[3] = "";
      }
      else {
         for (int i = 3; i > 1; i--) {
            string temp = word[i];
            word[i] = word[i - 1];
            word[i - 1] = temp;
         }
      }
   }

   /* Returns a vector containing each assembly word */
   vector<string> getDI() { 
      vector<string> vec;
      for (int i = 0; i < 4; i++) vec.push_back(word[i]);
      return vec;
   }
};

/*
   The simulator object compiles an input file by converting hex machine
   codes into 8-bit binary code, and uses the 8-bit binary instruction
   to convert it into assembly code.
*/
class Simulator{
private:
   char zFlag = '0';
   uint8_t registers[4] = {0, 0, 0, 0};
   vector<InstructionMemory> instructions;
   vector<DisassemblerInstructions> decodes;
   
public:
   void compileFile(string pathname, int numOfCycle, bool show_disassembly) {
      ifstream inputFile;

      inputFile.open(pathname, ios::in);

      if (inputFile) {
         string line;
         int index = 0;

         while (getline(inputFile, line)) {
            if (index == 0 && line != "v2.0 raw") {
               cout << "Invalid header file <"<< line <<">" << endl;
               exit(0);
            }

            if (line != "v2.0 raw") {
               InstructionMemory ins(hexToBin(line[0])+hexToBin(line[1]));
               instructions.push_back(ins);
               DisassemblerInstructions dec(ins.toString());
               decodes.push_back(dec);
            }
            index = 1;
         }
      }
      else {
         cout << "<File <" << pathname << "> not found>" << endl;
      }

      int cycle = 1, PC = 0;
      int loop = 0;

      /* Displays cycles as well as disassembly code */
      while (PC < decodes.size()) {
         if (cycle <= numOfCycle) {
            computeInstruction(decodes[PC], PC, loop);
            displayStates(cycle, PC);
            
            if (show_disassembly) {
               int index = loop > PC? loop : PC - 1;
               displayDisassembly(decodes[index].getDI());
               loop = 0;
            }
         }
         else { break; }
         cycle++;
      }
   }

   /* Takes care of intruction mnemonic operations */
   void computeInstruction(DisassemblerInstructions instruction,
      int &PC, int &loop ) {
      vector<string> word = instruction.getDI();
      
      if (word[0] == "not") {
         int dest = customCharToInt(word[1][1]);
         int rn = customCharToInt(word[2][1]);

         registers[dest] = ~registers[rn];
         zFlag = registers[dest] == 0? '1' : '0';
      }
      else if(word[0] == "bnz") {
         /* If in a loop, jump to targeted address */
         /* If zFlag is set, break loop and jump to next address */
         if (zFlag == '1') {
            PC = PC + 1;
            loop = false;
         }
         else {
            loop = PC;
            PC = stoi(word[1]);
         }
         return;
      }
      else {
         int dest = customCharToInt(word[1][1]);
         int rn = customCharToInt(word[2][1]);
         int rm = customCharToInt(word[3][1]);

         if (word[0] == "add") {
            registers[dest] = registers[rn] + registers[rm];
         }
         else {
            registers[dest] = registers[rn] & registers[rm];
         }
         zFlag = registers[dest] == 0? '1' : '0';
      }
      PC++;
   }

   /* Displays each cycle with the different states */
   void displayStates(int cycle, int PC) {
      stringstream r3("");
      r3 << hex << (int)registers[3];
      cout << "Cycle:" <<to_string(cycle)<< " States:PC:" << disNum(PC);
      cout << " Z:" << zFlag;
      cout << " R0:" <<disNum(registers[0])<< " R1:" << disNum(registers[1]);
      cout << " R2:" <<disNum(registers[2])<< " R3:" <<hex<< r3.str();
      cout << endl;
   }

   /* Displays a vector of disassemly as a string line */
   void displayDisassembly(vector<string> word) {
      cout << "Disassembly: ";
      for (int i = 0; i < 4; i++) {
         if (word.size() != 0) {
            cout << word[i] << " ";
         }
      }
      cout << endl << endl;
   }

   /* Returns single digits as string with leading 0's, 255 as FF */
   /* and 254 as FE, and any other numbers as string */
   string disNum(uint8_t num) {
      if (num == 255) return "FF";
      if (num == 254) return "FE";

      if (num < 10) {
         return '0' + to_string(num);
      }
      else {
         return to_string(num);
      }
   }

   /* Converts characters 0, 1, 2, 3 to their respective integer values */
   int customCharToInt(char c) {
      switch (c) {
      case '0':
         return 0;
      case '1':
         return 1;
      case '2':
         return 2;
      case '3':
         return 3;
      default:
         return -1;
      }
   }

   /* Converts an hexadecimal to binay */
   string hexToBin(char hex) {
      map<char, int> hexVal = {
         {'A', 10}, {'B', 11}, {'C', 12},
         {'D', 13}, {'E', 14}, {'F', 15}
      };
      string bin = "";
      int val = 0;

      if (hexVal.find(hex) != hexVal.end()) {
         val = hexVal[hex];
      }
      else {
         val = stoi(to_string(hex));
         char* newHex = &hex;
         sscanf(newHex, "%d", &val);
      }

      while (val != 0) {
         bin = to_string(val % 2) + bin;
         val /= 2;
      }
      while (bin.size() < 4) { bin = '0' + bin; }
      return bin;
   }
};

/* Output error message for invalid command inputs */
void errorMessage() {
   cout << "USAGE:  fiscsim  <object file> [cycles] [-l]\n";
   cout << "    -d : print disassembly listing with each cycle\n";
   cout << "    if cycles are unspecified the CPU will run for 20 cycles\n";
   exit(1);
}

/* Check if a string is a number */
bool isNumber(string str) {
   for (auto s : str) {
      if (s < '0' || s > '9') return false;
   }
   return true;
}


int main(int argc, char** argv)
{
   int cycles = 20;
   bool showDisassembly = false;

   if (argc < 2 || argc > 4) {
      errorMessage();
   }

   if (argc == 3) {
      string input = argv[2];
      if (isNumber(input)) {
         cycleccs = stoi(argv[2]);
      }
      else if (input == "-d") {
         showDisassembly = true;
      }
      else errorMessage();
   }
   else if(argc == 4) {
      string input1 = argv[2], input2 = argv[3];
      bool validIn = false;

      if (isNumber(input1) || isNumber(input2)) {
         cycles = isNumber(input1) ? stoi(input1) : stoi(input2);
         validIn = true;
      } 
      
      if (input1 == "-d" || input2 == "-d") {
         showDisassembly = true;
         validIn = true;
      }
      if(!validIn) { errorMessage(); }
   }

   Simulator simu;
   simu.compileFile(argv[1], cycles, showDisassembly);
}
