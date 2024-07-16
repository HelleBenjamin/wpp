#include <iostream> // include all the necessary libraries
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>

#define helpMessage "-s <source file> -o <output file> -h help -v version -c compile -i interpret -? print syntax -I interpret without file"
#define supportedArchitectures "Supported architectures: X86"
#define version "Wuf++ Interpreter Compiler v0.1.0"

#ifdef _WIN32
    bool os = 1;
#elif (__linux__)
    bool os = 0;
#endif

using namespace std;

vector<char> program; // source program
vector<string> compiledProgram; // compiled program
int mode = 0; //0 = compile, 1 = interpret, 2 = interpret without file



/* Wuf++ Interpreter Compiler, WIC
 * Copyright (c) 2024 Pepe57
 * All rights reserved.
 * MIT License
*/
string syntax = {
    "Syntax:\n"
    "+ - increment main register\n"
    "- - decrement main register\n"
    "} - push main register\n"
    "{ - pop main register\n"
   ". - print main register\n"
   ", - read to the main register\n"
   "& - jump to location pointed by pointer\n"
    "[ - pc = pc - cx\n"
   "] - pc = pc + cx\n"
   "! - invert main register\n"
   "> - increment pointer\n"
   "< - decrement pointer\n"
   "$ - print pointer\n"
   "#[char] - load char to the main register\n"
   "( - loop start, decrement pointer and loop until pointer = 0\n"
   ") - loop end\n"
   "\" - swap registers\n"
   "#[char] - compare main register with char, jump if equal to location pointed by pointer\n"
   "= - halt\n"
   "/ - add main register and pointer, bx = bx + cx\n"
   "\\ - sub main register and pointer, bx = bx - cx\n"
   "@ - load 0 to the main register\n"
   "^- swap bl with bh, bh <- bl\n"
};

string filename;
ifstream source_file;
ofstream output_file; // output file

uint16_t bx = 0;
uint16_t cx,dx = 0;
unsigned int pc = 0;
uint8_t stack[1000] = {0};
unsigned int sp = 1000;
bool halt = false;

string source_name, output_name;

string dectohex(int num) {
    stringstream ss;
    ss << hex << num;
    return ss.str();
}

void interpret(string InterpretedProgram) {
    pc = 0;
    while (pc < InterpretedProgram.length()) {
        if(halt) break;
        switch (InterpretedProgram[pc]) {
            case '\n':
                break;
            case ' ':
                break;
            case 'i':
                break;
            case 'o':
                break;
            case '+':
                bx++;
                break;
            case '-':
                bx--;
                break;
            case '}':
                sp--;
                stack[sp] = bx & 0xFF;
                sp --;
                stack[sp] = (bx & 0xFF00) >> 8;
                break;
            case '{':
                bx = (stack[sp] << 8) | stack[sp+1];
                sp += 2;
                break;
            case '.':
                cout << char(bx);
                break;
            case ',':
                bx = cin.get();
                break;
            case '&':
                pc = cx;
                break;
            case '[':
                pc = pc - cx;
                break;
            case ']':
                pc = pc + cx;
                break;
            case '!':
                bx = ~bx;
                break;
            case '>':
                cx++;
                break;
            case '<':
                cx--;
                break;
            case '$':
                cout << dectohex(cx) << endl;
                break;
            case '#':
                bx = InterpretedProgram[pc+1];
                pc++;
                break;
            case '(': 
                dx = pc;
                cx--;
                break;
            case ')':
                if (cx != 0) {
                    cx--;
                    pc = dx;
                    break;
                } if (cx == 0) {
                    break;
                }
            case '"':
                dx = bx;
                bx = cx;
                cx = dx;
                break;
            case '%':
                pc++;
                if (bx == InterpretedProgram[pc]) {
                    pc = cx -1;
                    break;
                }
                break;
            case '=':
                halt = true;
                break;
            case '/':
                bx = bx + cx;
                break;
            case '\\':
                bx = bx - cx;
                break;
            case '@':
                bx = 0;
                break;
            case '^':
                dx = bx;
                bx = (dx & 0xFF00);
                bx = (dx & 0xFF00) >> 8;
                break;
            default:
                cout << "Error: Unknown command: '" << InterpretedProgram[pc] << "' at position: " << pc << "" << endl;
                break;
        }
        pc++;
    }
}

void interpret_without_file() {
    cout << "--Wuf++ interpreter--" << endl;
    int i = 0;
    char programI[0xff] = {0};
    loop:
    for (i = 0; i < 0xff; i++) programI[i] = 0;
    i = 0;
    bx, cx, dx = 0;
    sp = 1000;
    halt = false;
    cout << "> ";
    while(1) {
        cin.get(programI[i]);
        if (programI[i] == '\n') break;
        i++;
    }
    programI[i] = '=';
    interpret(programI);
    cout << '\n';
    goto loop;
}

void Interpreter() {
    char c;
    while(source_file.get(c)) {
        program.push_back(c);
    }
    source_file.close();
    interpret(program.data());
}

void compileX86() {
    char c;
    int pc = 0;
    int loopLabel = 0;
    while(source_file.get(c)) {
        program.push_back(c);
    }

    compiledProgram.push_back("global _start");
    compiledProgram.push_back(";wic v0.1.0");
    compiledProgram.push_back("section .text");

    compiledProgram.push_back("jp_cx:");
    compiledProgram.push_back("     jmp edx");
    if (program[pc] == 'i') {
        compiledProgram.push_back("readc:");
        compiledProgram.push_back("     mov edi, ecx");
        compiledProgram.push_back("     mov eax, 0x3");
        compiledProgram.push_back("     mov ebx, 0x0");
        compiledProgram.push_back("     mov ecx, ebx");
        compiledProgram.push_back("     mov edx, 1");
        compiledProgram.push_back("     int 0x80");
        compiledProgram.push_back("     mov ecx, edi");
        compiledProgram.push_back("     ret");     
        pc++;
    }
    if (program[pc] == 'o') {
        compiledProgram.push_back("printc:");
        compiledProgram.push_back("     mov edi, ecx");
        compiledProgram.push_back("     push ebx");   
        compiledProgram.push_back("     mov eax, 0x4");
        compiledProgram.push_back("     mov ebx, 0x1");
        compiledProgram.push_back("     mov ecx, esp");
        compiledProgram.push_back("     mov edx, 1");
        compiledProgram.push_back("     int 0x80");
        compiledProgram.push_back("     pop ebx");
        compiledProgram.push_back("     mov ecx, edi");
        compiledProgram.push_back("     ret");
        pc++;
    }

    compiledProgram.push_back("_start:");
    compiledProgram.push_back("     mov ebx, 0");
    compiledProgram.push_back("     mov ecx, 0");
    compiledProgram.push_back("     mov edx, 0");
    compiledProgram.push_back("main:");
    int get_pc = 0;
    for(int i = pc; i < program.size(); i++) {
        switch (program[i]) {
            case '\n':
                break;
            case ' ':
                break;
            case '+':
                compiledProgram.push_back("     inc ebx");
                break;
            case '-':
                compiledProgram.push_back("     dec ebx");
                break;
            case '}':
                compiledProgram.push_back("     push ebx");
                break;
            case '{':
                compiledProgram.push_back("     pop ebx");
                break;
            case '.':
                compiledProgram.push_back("     call printc");
                break;
            case ',':
                compiledProgram.push_back("     call readc");
                break;
            case '[':
                compiledProgram.push_back("     sub epc, ecx");
                break;
            case ']':
                compiledProgram.push_back("     call .get_pc" + std::to_string(get_pc));
                compiledProgram.push_back("     .get_pc" + std::to_string(get_pc) + ": pop edx");
                compiledProgram.push_back("     add edx, ecx");
                compiledProgram.push_back("     jmp edx");
                get_pc++;
                break;
            case '!':
                compiledProgram.push_back("     not ebx");
                break;
            case '>':
                compiledProgram.push_back("     inc ecx");
                break;
            case '<':
                compiledProgram.push_back("     dec ecx");
                break;
            case '$':
                compiledProgram.push_back("     mov dx, bx");
                compiledProgram.push_back("     mov bx, cx");
                compiledProgram.push_back("     call printc");
                compiledProgram.push_back("     mov bx, dx");
                break;
            case '#':
                compiledProgram.push_back("     mov bx, '" + std::string(1, program[i+1]) + "'");
                i++;
                break;
            case '(': 
                compiledProgram.push_back("loop" + std::to_string(loopLabel) + ":");
                loopLabel++;
                break;
            case ')':
                compiledProgram.push_back("     cmp ecx, 0");
                compiledProgram.push_back("     dec ecx");
                compiledProgram.push_back("     jne loop" + std::to_string(loopLabel-1));
                compiledProgram.push_back("     .loop" + std::to_string(loopLabel-1) + "_end:");
                break;
            case '"':
                compiledProgram.push_back("     xchg ebx, ecx");
                break;
            case '%':
                compiledProgram.push_back("     cmp ebx, " + std::to_string(program[i+1]));
                compiledProgram.push_back("     lea edx, [ecx + main]");
                compiledProgram.push_back("     je jp_cx");
                i++;
                break;
            case '=':
                compiledProgram.push_back("     mov eax, 1");
                compiledProgram.push_back("     mov ebx, 0");
                compiledProgram.push_back("     int 0x80");
                break;
            case '/':
                compiledProgram.push_back("     add ebx, ecx");
                break;
            case '\\':
                compiledProgram.push_back("     sub ebx, ecx");
                break;
            case '@':
                compiledProgram.push_back("     mov ebx, 0");
                break;
            case '^':
                compiledProgram.push_back("     xchg bl, bh");
                break;
            default:
                cout << "Error: " << program[i] << " at position " << i << endl;
                break;
        }
    }
    for(int i = 0; i < compiledProgram.size(); i++) {
        output_file << compiledProgram[i] << endl;
        //cout << compiledProgram[i] << endl;
    }
    cout << "Compiled" << endl;
}

int main(int argc, char *argv[]) {
    for(int i = 0; i < argc; i++) {
        if (argc == 1) {
            cout << "Error: No arguments" << endl;
            cout << "Usage: ";
            cout << helpMessage << endl;
            return 1;
        }   
        if(string(argv[i]) == "-h") {
            cout << helpMessage << endl;
        }
        if((string(argv[i])).find("-s") == 0) {
            filename = argv[i+1];
            int len = filename.length();
            source_name = filename;
            source_name.erase(len-4, 4); // remove .wpp
            source_file.open(filename);
        }
        if((string(argv[i])).find("-o") == 0) {
            filename = argv[i+1];
            output_name = filename;
            filename = filename + ".asm";
            output_file.open(filename);
        }
        if(string(argv[i]) == "-v") {
            cout << version << endl;
            cout << supportedArchitectures << endl;
        }
        if(string(argv[i]) == "-c") {
            mode = 0;
        }
        if(string(argv[i]) == "-i") {
            mode = 1;
        }
        if (string(argv[i]) == "-?") {
            cout << syntax << endl;
        }
        if (string(argv[i]) == "-I") {
            mode = 2;
        }
    }
    if (source_file.is_open() && mode == 0 && output_file.is_open()) {
        compileX86();
        if (os == 0){ //linux
            system(("nasm -f elf32 -o " + output_name + ".o " + source_name + ".asm").c_str());
            system(("ld -m elf_i386 -o " + output_name + " -static -nostdlib " + output_name + ".o").c_str());
            system(("rm " + output_name + ".o").c_str());
        } else if (os == 1){ //windows
            system(("nasm -f win32 -o " + output_name + ".o " + source_name + ".asm").c_str());
            system(("ld -m i386pe -o " + output_name + " -static -nostdlib " + output_name + ".o").c_str());
            system(("del " + output_name + ".o").c_str());
            
        }
        //system(("./wic -s " + source_name + " -o " + output_name + ".asm").c_str());
    } if (mode == 1 && source_file.is_open()) {
        Interpreter();
    } if (mode == 2){
        interpret_without_file();
    }
    source_file.close();
    output_file.close();
    return 0;
}