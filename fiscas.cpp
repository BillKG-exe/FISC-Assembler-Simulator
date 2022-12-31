#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

/*
   An Instruction is line containing assembly code.
   An assembly code contains labels, comments and operands
   and an instruction mnemonic.
*/
class Instruction {
   private: 
      vector<string> line;
   public:
      Instruction(vector<string> line) {
         this->line = line;
      }

      vector<string> getInstruction() { return line; }

      /* 
         This function returns a string version of an intruction line
         excluding the defined labels. 
      */
      string toString() {
         string s = "";
         for (auto l : line) {
            if(l[l.size()-1] != ':') s += l + " ";
         }
         return s;
      }
};

/* 
   An Assembly Instruction is a word instruction consisting of an 8-bit
   binary numbers. In 2-bit representation of opCode and operands it is
   stored in the following order: Op Rn Rm Rd.
   This Class make use of an unordered map called OPS_REF which holds 
   2-bits binary values representing an instruction mnemonic or operand.
*/
class AssemblyInstruction {
private:
   vector<string> word;
   unordered_map<string, string> OPS_REF = {
      {"add", "00"}, {"and", "01"}, {"not", "10"},
      {"bnz", "11"}, {"r0", "00"}, {"r1", "01"},
      {"r2", "10"}, {"r3", "11"}
   };
public:
   /* This function initializes the word instruction to empty 2-bits */
   AssemblyInstruction() {
      for (int i = 0; i < 4; i++) { word.push_back(""); }
   }

   /*
      The setWord function sets the binary representation of an assembly
      instruction. The six least significants bit is manupulated in 
      accordance of the opCode.
      The "and", "add", and "not" opCodes shift rd to the end. However,
      the "bnz" converts it last six significants bit to a decimal value. 
   */
   void setWord(int start, vector<string> &line, 
      unordered_map<string, int> labels, int lCount) {
      int count = 1, index = 1;
      string opCode = line[start];
      toLower(opCode);
      
      word[0] = OPS_REF[opCode];

      for (int i = start+1; i < line.size(); i++) {
         toLower(line[i]);
            
         if (line[i].size() > 2 && line[i][0] == 'r') line[i].erase(2);

         if (line[i][0] == 'r' && OPS_REF.find(line[i]) != OPS_REF.end()) {
            word[index++] = OPS_REF[line[i]];
         } else if (line[i][0] == 'r') {
            cout << "<Invalid register " << line[i] << ">\n";
            exit(0);
         }
      }

      if (opCode == "not") {
         if (line.size() - start != 3) {
            cout << "<Instructure is missing at least one operand>" << endl;
            exit(0);
         }

         string temp = word[1];
         word[3] = word[1];
         word[1] = word[2];
         word[2] = "00";
      }
      else if (opCode == "bnz") {
         if (line.size() - start != 2) {
            cout << "<Instruction is missing at least one operand>" << endl;
            exit(0);
         }

         unordered_map<string, int>::iterator it = labels.find(line[start+1]);

         if (it == labels.end()) {
            cout << "<Label <" << line[start + 1];
            cout << "> on line <" << lCount;
            cout << "> is undefined.>" << endl;
            exit(0);
         }

         string bnzWord = decToBin(labels[line[start+1]]);

         int index = 1;
         for (int i = 0; i < bnzWord.size(); i += 2) {
            word[index] += bnzWord[i];
            word[index++] += bnzWord[i + 1];
         }
      }
      else {
         if (opCode != "and" && opCode != "add") {
            cout << "<Invalid operand for the opCode>" << endl;
         }

         if (line.size() - start != 4) {
            cout << "<Instruction is missing at least one operand>" << endl;
            exit(0);
         }

         for (int i = 1; i < word.size()-1; i++) {
            string temp = word[i];
            word[i] = word[i + 1];
            word[i + 1] = temp;
         }
      }
   }

   /* returns the vector containing the instruction word */
   vector<string> getWord() { return word; }

   /* Converts a string into lower case */
   void toLower(string &s) {
      for (int i = 0; i < s.size(); i++) {
         s[i] = tolower(s[i]);
      }
   }

   /* This function is used to convert an integer into binary */
   string decToBin(int dec) {
      string bin = "";

      while (dec != 0) {
         bin = to_string(dec % 2) + bin;
         dec /= 2;
      }
      while (bin.size() != 6) { bin = '0' + bin; }
      return bin;
   }
};

/*
   This Assembler class is used to mimic the behavior of an assembler.
   It has a label map which stores the addresses of the different labels,
   a vector of instructions representing the instructions in the input 
   file, a vector of assembly instructions consisting of 8-bit binary 
   instructions, and a vector named hexCode containing the machine code.
*/
class Assembler {
   private:
      unordered_map<string, int> labels;
      vector<Instruction> instructions;
      vector<AssemblyInstruction> computerInstructions;
      vector<string> hexCode;

   public:
      /* 
         This method reads an input file and converts the instructions into
         into binary representation.
      */
      void readData(string path) {
         firstPass(path);
         secondPass();
      }

      /*
         The firstPass reads an input file and converts each instruction line
         into an instruction object. It also keep track of labels and their
         addresses.
         It also ouput error such invalid label definitions and use, and out
         of bound memory storage.
      */
      void firstPass(string path) {
         ifstream inputFile;

         inputFile.open(path, ios::in);

         if (inputFile) {
            string line;
            int count = 0;

            while (getline(inputFile, line)) {
               if (count > 63) {
                  cout << "<Output file is larger than system memory>\n";
                  exit(0);
               }

               if (line.empty()) continue;
               if (isComment(line)) continue;

               vector<string> splittedLine = split(line, ' ');
               Instruction i(splittedLine);
               instructions.push_back(i);

               string firstWord = splittedLine[0];

               if (firstWord.size() != 0 && isLabel(firstWord)) {
                  firstWord.erase(firstWord.size() - 1);
                  
                  if (labels.find(firstWord) != labels.end()) {
                     cout << "Label <"<< firstWord << "> on line <";
                     cout << count << "> is already defined." << endl;
                     exit(0);
                  }
                  labels.emplace(firstWord, count);
               }
               count++;
            }
            inputFile.close();
         }
         else {
            cout << "<File <" <<path<< "> was not found>\n";
            exit(0);
         }
      }

      /*
         The second pass converts each instruction object into an assembly
         instruction object, and adds it to the computersInstruction vector.
      */
      void secondPass() {
         for (int i = 0; i < instructions.size(); i++) {
            vector<string> instruction = instructions[i].getInstruction();
            AssemblyInstruction ai;
               
            if (isComment(instruction[0])) {
               continue;
            }
            else if(isLabel(instruction[0])) {
               ai.setWord(1, instruction, labels, i);
            }
            else {
               ai.setWord(0, instruction, labels, i);
            }

            computerInstructions.push_back(ai);
         }
      }

      /*
         WriteData converts each 8-bit instruction word into hexadecimal
         and stores the resulting value into an object file, and the 
         hexCode vector.
      */
      void writeData(string path) {
         ofstream outputFile;

         outputFile.open(path, ios::out);

         if (outputFile) {
            outputFile << "v2.0 raw" << endl;

            for (auto ci : computerInstructions) {
               vector<string> word = ci.getWord();
               string leftHex = binToHex(word[0]+word[1]);
               string rightHex = binToHex(word[2] + word[3]);
               string hexLine = "";
               
               hexLine.append(leftHex);
               hexLine.append(rightHex);
               outputFile << hexLine << endl;
               hexCode.push_back(hexLine);
            }
            outputFile.close();
         }
         else {
            cout << "<Invalid input for object file ";
            cout << "<" << path << " >>" << endl;
            exit(0);
         }
      }

      /* This method return a string with a leading 0 for single digits */
      /* and return a string representation of non single digits  */
      string strDig(int d) {
         if (d <= 9) {
            return "0" + to_string(d);
         }
         else return to_string(d);
      }

      /* Displays the listing for -l command option */
      void displayListing() {
         cout << "*** LABEL LIST***" << endl;
         
         unordered_map<string, int>::iterator it = labels.begin();

         for (it; it != labels.end(); it++) {
            cout << setw(8) << left << it->first;
            cout << setw(4)<< strDig(it->second) << endl;
         }

         cout << "*** MACHINE PROGRAM ***" << endl;
         for (int i = 0; i < hexCode.size(); i++) {
            cout << strDig(i) << ":" <<left << setw(5) << hexCode[i];
            cout <<setw(15)<< instructions[i].toString() << endl;
         }
      }

      /* Converts a binary string into hexadecimal */
      string binToHex(string bin) {
         unordered_map<int, string> hexVal = {
            {10, "A"}, {11, "B"}, {12, "C"},
            {13, "D"}, {14, "E"}, {15, "F"}
         };

         int power = 1;
         int val = 0;

         for (int i = bin.size() - 1; i >= 0; i--) {
            if (bin[i] == '1') { val += power; }
            power *= 2;
         }

         if (val > 9) {
            return hexVal[val];
         } else {
            return to_string(val);
         }
      }

      /*  
         This is a custum split method which splits instruction lines without
         including comments.
      */
      vector<string> split(string str, char sep) {
         vector<string> strList;
         string newStr = "";

         for (int i = 0; i < str.size(); i++) {
            if (str[i] == ';') { return strList; }

            if (str[i] != sep) {
               newStr += str[i];
            }
            else {
               if (newStr.size() != 0) {
                  strList.push_back(newStr);
               }
               newStr = "";
            }
         }
        
         if (newStr.size() != 0) {
            strList.push_back(newStr);
         }
         return strList;
      }
      /* Check if a word is a label */
      bool isLabel(string word) {
         if (word.size() == 0) return false;
         return word[word.size() - 1] == ':' ? true : false;
      }
      /* Checks if a word is beginning of a comment */
      bool isComment(string word) {
         int count = 0;
         if (word.size() == 0) return false;
         for (auto s : word) {
            if (s == ' ') count++;
            else break;
         }
         return word[count] == ';' ? true : false;
      }
};

/* Prints error message for invalid command operstions */
void errorMessage() {
   cout << "USAGE:  fiscas <source file> <object file> [-l]\n";
   cout << "        -l : print listing to standard error" << endl;
   exit(0);
}

int main(int argc, char** argv) {
   bool showListing = false;

   if (argc < 3 || argc > 4) {
      errorMessage();
   }
   else if (argc == 4) {
      string input = argv[3];

      if (input == "-l") {
         showListing = true;
      }
      else {
         errorMessage();
      }
   }

   Assembler assembler;

   assembler.readData(argv[1]);
   assembler.writeData(argv[2]);
   if (showListing) assembler.displayListing();
}