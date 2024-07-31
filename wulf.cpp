#include <iostream> // include all the necessary libraries
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include <map>
#include <unordered_map>

#define helpMessage "-s <source file> -o <output file> -h help -v version-i interpret -? print syntax -I interpret without file"
#define version "Wulf Interpreter Compiler v0.1.0"

using namespace std;

vector<string> WULFprogram;

/* WULF Programming Language
 * Copyright (c) 2024 Benjamin H.
 * All rights reserved.
 * MIT License

Syntax:
 - local <type> <name>// local variable in function
 - global <type> <name>// global variable
 - int <name>// 32 bit signed integer
 - char <name>// 8 bit signed integer
 - function <name> //function name
 - end //end function
 - return //return from function call
 - ifc <condition>//if char condition
 - ifi <condition>//if int condition
 - endif //end if
 - add <dest> <op1> <op2> //add
 - sub <dest> <op1> <op2> //sub
 - mul <dest> <op1> <op2> //mul
 - div <dest> <op1> <op2> //div
 - in <var> //read input to variable
 - out <var> //print variable
 - call <function> //call function
 - main //main function, program starts here
 - endProgram //end program
 - assignc <var> <value> //assign value to char variable
 - assigni <var> <value> //assign value to int variable
 - # //comment


Example code:
function add
    int num1
    assign num1, 10
    int num2
    assign num2, 20
    int sum
    add sum, num1, num2
    out sum
    return
end

main
    call add
    endProgram
*/

int call_stack[1000] = {0};
int call_stack_pointer = 0;
int line = 0;
bool halt = 0;

string filename;
ifstream source_file;
ofstream output_file; // output file
string source_name, output_name;

vector<string> fullyCompiledProgram; //final output

void compile(vector<string> program, int len) {
    unsigned int endifCounter = 0;
    vector<string> compiledProgram; //supported architectures: X86 on linux, windows not supported
    vector<string> compiledVariables; //bss section
    compiledProgram.push_back("bits 32");
    compiledProgram.push_back("global _start");
    compiledProgram.push_back(";wulf interpreter compiler v0.1.0");
    compiledProgram.push_back("section .text");

    compiledProgram.push_back("readc:"); //read character
    compiledProgram.push_back("    mov eax, 0x3");
    compiledProgram.push_back("    mov ebx, 0x0");
    compiledProgram.push_back("    mov ecx, ebx");
    compiledProgram.push_back("    mov edx, 1");
    compiledProgram.push_back("    int 0x80");
    compiledProgram.push_back("    mov ecx, edi");
    compiledProgram.push_back("    ret");

    compiledProgram.push_back("printc:");
    compiledProgram.push_back("    push ebx");   
    compiledProgram.push_back("    mov eax, 0x4");
    compiledProgram.push_back("    mov ebx, 0x1");
    compiledProgram.push_back("    mov ecx, esp");
    compiledProgram.push_back("    mov edx, 1");
    compiledProgram.push_back("    int 0x80");
    compiledProgram.push_back("    pop ebx");
    compiledProgram.push_back("    mov ecx, edi");
    compiledProgram.push_back("    ret");

    compiledProgram.push_back("_start:");
    compiledProgram.push_back("    mov ebp, esp");
    compiledProgram.push_back("    sub esp, 0x10000"); //reserve 64kb of stack
    compiledProgram.push_back("    jmp main");

    compiledVariables.push_back("section .bss");
    while (line < len) {
        string instruction = program[line];
        instruction.erase(0, instruction.find_first_not_of("\t "));
        string parameter1, parameter2, parameter3, parameter4, parameter5;
        int spaceIndex = instruction.find(' ');
        if (spaceIndex != string::npos) {
            parameter1 = instruction.substr(spaceIndex + 1);
            spaceIndex = parameter1.find(' ');
            if (spaceIndex != string::npos) {
                parameter2 = parameter1.substr(spaceIndex + 1);
                parameter1 = parameter1.substr(0, spaceIndex);
                spaceIndex = parameter2.find(' ');
                if (spaceIndex != string::npos) {
                    parameter3 = parameter2.substr(spaceIndex + 1);
                    parameter2 = parameter2.substr(0, spaceIndex);
                    spaceIndex = parameter3.find(' ');
                    if (spaceIndex != string::npos) {
                        parameter4 = parameter3.substr(spaceIndex + 1);
                        parameter3 = parameter3.substr(0, spaceIndex);
                        spaceIndex = parameter4.find(' ');
                        if (spaceIndex != string::npos) {
                            parameter5 = parameter4.substr(spaceIndex + 1);
                            parameter4 = parameter4.substr(0, spaceIndex);
                        }
                    }
                }
            }
        }
        if(halt) break;
        if(instruction == "\n") line++;
        if(instruction == " ") line++;
        if(instruction.find("#") != string::npos){
            compiledProgram.push_back(";" + instruction);
        }
        if(instruction.find("int ") != string::npos) {
            compiledVariables.push_back("   " + parameter1 + ": resb 4");
        }
        if(instruction.find("char ") != string::npos) {
            compiledVariables.push_back("   " + parameter1 + ": resb 1");
        }
        if(instruction.find("function ") != string::npos) {
            compiledProgram.push_back(parameter1 + ":");
            compiledProgram.push_back("    push ebp");
            compiledProgram.push_back("    mov ebp, esp");
            compiledProgram.push_back("    sub esp, 0x1000"); //reserve 4kb for function
        }
        if(instruction.find("end ") != string::npos) {
            compiledProgram.push_back("    ret");
        }
        if(instruction.find("return") != string::npos) {
            compiledProgram.push_back("    mov esp, ebp");
            compiledProgram.push_back("    pop ebp");
            compiledProgram.push_back("    ret");
        }
        if(instruction.find("ifc ") != string::npos) { //if condition for char
            endifCounter++;
            uint8_t secondischar = 0;
            if (parameter3.find("'") != string::npos) secondischar = 1;
            switch (secondischar){
                case 0:
                    if (parameter2 == "==") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, [" + parameter3 + "]");
                        compiledProgram.push_back("    jne .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is met, else continue
                    } if (parameter2 == "!=") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, [" + parameter3 + "]");
                        compiledProgram.push_back("    je .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    } if (parameter2 == ">") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, [" + parameter3 + "]");
                        compiledProgram.push_back("    jna .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    } if (parameter2 == "<") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, [" + parameter3 + "]");
                        compiledProgram.push_back("    jnb .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    }
                    break;
                case 1:
                    if (parameter2 == "==") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, " + parameter3);
                        compiledProgram.push_back("    jne .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is met, else continue
                    } if (parameter2 == "!=") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, " + parameter3);
                        compiledProgram.push_back("    je .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    } if (parameter2 == ">") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, " + parameter3);
                        compiledProgram.push_back("    jna .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    } if (parameter2 == "<") {
                        compiledProgram.push_back("    mov al, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp al, " + parameter3);
                        compiledProgram.push_back("    jnb .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    }
                    break;

            }
        }
        if(instruction.find("ifi ") != string::npos) { //if condition for int
            endifCounter++;
            uint8_t secondisint = false;
            if (isdigit(parameter3[0])) secondisint = true;
            switch (secondisint){
                case 0:
                    if (parameter2 == "==") {
                        compiledProgram.push_back("    mov eax, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp eax, [" + parameter3 + "]");
                        compiledProgram.push_back("    jne .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is met, else continue
                    } if (parameter2 == "!=") {
                        compiledProgram.push_back("    mov eax, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp eax, [" + parameter3 + "]");
                        compiledProgram.push_back("    je .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    }
                    break;
                case 1:
                    if (parameter2 == "==") {
                        compiledProgram.push_back("    mov eax, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp eax, " + parameter3);
                        compiledProgram.push_back("    jne .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is met, else continue
                    } if (parameter2 == "!=") {
                        compiledProgram.push_back("    mov eax, [" + parameter1 + "]");
                        compiledProgram.push_back("    cmp eax, " + parameter3);
                        compiledProgram.push_back("    je .endif" + to_string(endifCounter - 1)); //jump to end of if statement if condition is not met, else continue
                    }
                    break;
            }
        }
        if(instruction.find("endif") != string::npos) {
            compiledProgram.push_back("    .endif" + to_string(endifCounter - 1) + ":");
        }
        if(instruction.find("add ") != string::npos) {
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            compiledProgram.push_back("    add eax, [" + parameter3 + "]");
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        if(instruction.find("sub ") != string::npos) {
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            compiledProgram.push_back("    sub eax, [" + parameter3 + "]");
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        if(instruction.find("mul ") != string::npos) {
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            compiledProgram.push_back("    imul eax, [" + parameter3 + "]");
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        if(instruction.find("div ") != string::npos) {
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            compiledProgram.push_back("    cdq");
            compiledProgram.push_back("    idiv [" + parameter3 + "]");
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        if(instruction.find("in ") != string::npos) {
            compiledProgram.push_back("    call readc");
            compiledProgram.push_back("    mov ebx,[" + parameter1 + "]");
        }
        if(instruction.find("out ") != string::npos) {
            compiledProgram.push_back("    mov ebx, [" + parameter1 + "]");
            compiledProgram.push_back("    call printc");
        }
        if(instruction.find("call ") != string::npos) {
            compiledProgram.push_back("    call " + parameter1);
        }
        if(instruction.find("main") != string::npos) {
            compiledProgram.push_back("main:");
        }
        if(instruction.find("endProgram") != string::npos) {
            compiledProgram.push_back("    mov eax, 0x1");
            compiledProgram.push_back("    mov ebx, 0x0");
            compiledProgram.push_back("    int 0x80");
        }
        if(instruction.find("assigni ") != string::npos) {
            compiledProgram.push_back("    mov dword [" + parameter1 + "]," + parameter2);
        }
        if(instruction.find("assignc ") != string::npos) {
            compiledProgram.push_back("    mov byte [" + parameter1 + "]," + parameter2);
        }
        line++;
    }

    for(int i = 0; i < compiledProgram.size(); i++) {
        fullyCompiledProgram.push_back(compiledProgram[i]);
    }

    for(int i = 0; i < compiledVariables.size(); i++) {
        fullyCompiledProgram.push_back(compiledVariables[i]);
    }

        for(int i = 0; i < fullyCompiledProgram.size(); i++) {
        output_file << fullyCompiledProgram[i] << endl;
        //cout << compiledProgram[i] << endl;
    }
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
            cout << "Copyright (c) 2024 Benjamin H." << endl;
            cout << version << endl;
        }
    }
    if (source_file.is_open() && output_file.is_open()) {
        while (!source_file.eof()) {
            string cline;
            getline(source_file, cline);
            WULFprogram.push_back(cline);
        }
        compile(WULFprogram, WULFprogram.size());
        system(("nasm -f elf32 -o " + source_name + ".o " + filename).c_str());
        system(("ld -m elf_i386 -o " + output_name + " -static -nostdlib " + source_name + ".o").c_str());
        system(("rm " + source_name + ".o").c_str());
    }
    source_file.close();
    output_file.close();
    return 0;
}

