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

struct variable{
    string name;
    int value;
    int type;
};

vector<variable> variables;

struct function{
    string name;
    int line; //used for calling
};

vector<function> functions;

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
 - if <condition>//conditional, example if eq num1 num2
 - then //after if statement 
 - eq //equal
 - neq //not equal
 - else //after else statement
 - add <dest> <op1> <op2> //add
 - sub <dest> <op1> <op2> //sub
 - mul <dest> <op1> <op2> //mul
 - div <dest> <op1> <op2> //div
 - in <var> //read input to variable
 - out <var> //print variable
 - call <function> //call function
 - main //main function, program starts here
 - endProgram //end program
 - assign <var> <value> //assign value to variable
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



void interpret(string *InterpretedProgram){
    while (line < InterpretedProgram->size()) {
        string instruction = InterpretedProgram[line];
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
        if(instruction.find("#") != string::npos) line++;
        if(instruction.find("int ") != string::npos) {
            variables.push_back({parameter1, 0, 0});
        }
        if(instruction.find("char ") != string::npos) {
            variables.push_back({parameter1, 0, 1});
        }
        if(instruction.find("function ") != string::npos) {
            functions.push_back({parameter1, line+1});
        }
        if(instruction.find("end") != string::npos) {
            line = call_stack[call_stack_pointer - 1];
            call_stack_pointer--;
        }
        if(instruction.find("return") != string::npos) {
            line = call_stack[call_stack_pointer - 1];
            call_stack_pointer--;
        }
        if(instruction.find("if ") != string::npos) {
            line++;
        }
        if(instruction.find("then") != string::npos) {
            line++;
        }
        if(instruction.find("else") != string::npos) {
            line++;
        }
        if(instruction.find("add") != string::npos) {
            string dest = parameter1;
            string op1 = parameter2;
            string op2 = parameter3;
        }
        if(instruction.find("sub") != string::npos) {
            string dest = instruction.substr(4, instruction.size()-4);
            string op1 = instruction.substr(7, instruction.size()-7);
            string op2 = instruction.substr(10, instruction.size()-10);
            variables[variables.size() - 1].value = variables[variables.size() - 1].value - variables[variables.size() - 2].value;
        }
        if(instruction.find("mul") != string::npos) {
            string dest = instruction.substr(4, instruction.size()-4);
            string op1 = instruction.substr(7, instruction.size()-7);
            string op2 = instruction.substr(10, instruction.size()-10);
            variables[variables.size() - 1].value = variables[variables.size() - 1].value * variables[variables.size() - 2].value;
        }
        if(instruction.find("div") != string::npos) {
            string dest = instruction.substr(4, instruction.size()-4);
            string op1 = instruction.substr(7, instruction.size()-7);
            string op2 = instruction.substr(10, instruction.size()-10);
            variables[variables.size() - 1].value = variables[variables.size() - 1].value / variables[variables.size() - 2].value;
        }
        if(instruction.find("out") != string::npos) {
            cout << variables[variables.size() - 1].value << endl;
            line++;
        }
        if(instruction.find("call") != string::npos) {
            call_stack[call_stack_pointer] = line;
            call_stack_pointer++;
            line++;
        }
        if(instruction.find("return") != string::npos) {
            line++;
        }
        if(instruction.find("assign") != string::npos) {
            string dest = parameter1;
            int val = stoi(parameter2);
            variables[variables.size() - 1].value = val;
        }
        if (instruction.find("endProgram") != string::npos) halt = 1;
        line++;
    }

}

vector<string> fullyCompiledProgram; //final output

void compile(string *program, int len) {
    vector<string> compiledProgram; //supported architectures: X86 on linux, windows not supported
    vector<string> compiledVariables; //bss section
    compiledProgram.push_back("bits 32");
    compiledProgram.push_back("global _start");
    compiledProgram.push_back(";wulf interpreter compiler v0.1.0");
    compiledProgram.push_back("section .text");
    compiledProgram.push_back("readc:"); //read character
    compiledProgram.push_back("    mov edi, ecx");
    compiledProgram.push_back("    mov eax, 0x3");
    compiledProgram.push_back("    mov ebx, 0x0");
    compiledProgram.push_back("    mov ecx, ebx");
    compiledProgram.push_back("    mov edx, 1");
    compiledProgram.push_back("    int 0x80");
    compiledProgram.push_back("    mov ecx, edi");
    compiledProgram.push_back("    ret");
    compiledProgram.push_back("printc:");
    compiledProgram.push_back("    mov edi, ecx");
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
    compiledProgram.push_back("    mov ebx, 0");
    compiledProgram.push_back("    mov ecx, 0");
    compiledProgram.push_back("    mov edx, 0");
    compiledProgram.push_back("    mov ebp, esp");
    compiledProgram.push_back("    sub esp, 0x10000"); //reserve 64kb of stack
    compiledProgram.push_back("main:");

    compiledVariables.push_back("section .bss");

    while (line < len) {
        string instruction = program[line];
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
        if(instruction.find("end") != string::npos) {
            compiledProgram.push_back("    ret");
        }
        if(instruction.find("return") != string::npos) {
            compiledProgram.push_back("    mov esp, ebp");
            compiledProgram.push_back("    pop ebp");
            compiledProgram.push_back("    ret");
        }
        if(instruction.find("if ") != string::npos) {
            if (parameter1 == "eq") {
                compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            }
        }
        line++;
    }

    for(int i = 0; i < compiledProgram.size(); i++) {
        fullyCompiledProgram.push_back(compiledProgram[i]);
    }

    for(int i = 0; i < compiledVariables.size(); i++) {
        fullyCompiledProgram.push_back(compiledVariables[i]);
    }
}

int main(int argc, char *argv[]) {
    string cProgram[] = {"int cs", "int num", "char chacha", "function test"};
    compile(cProgram, 4);
    for (int i = 0; i < fullyCompiledProgram.size(); i++) {
        cout << fullyCompiledProgram[i] << endl;
    }
    return 0;
}
