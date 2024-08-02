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
 - int <name>// 32 bit signed integer
 - char <name>// 8 bit signed integer
 - array <type> <name> <size> //array, for example: array int arr 10
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
 - cin <var> //read input to char variable
 - iin <var> //read input to int variable
 - outc <var> //print char variable
 - outi <var> //print int variable
 - call <function> //call function
 - main //main function, program starts here
 - endProgram //end program
 - assignc <var> <value> //assign value to char variable
 - assigni <var> <value> //assign value to int variable
 - arrasgnc <arr> <index> <value> //assign value to char array
 - arrasgni <arr> <index> <value> //assign value to int array
 - loop <times> //loop x times
 - endloop //end loop
 - inc <var> //increment variable
 - dec <var> //decrement variable
 - prints <message> //print string message
 - arrcpy <type> <dest> <src> <index> //copy array value to variable
 - arroutc <arr> <index> //print char array at index
 - arrouti <arr> <index> //print int array at index
 - arrcin <arr> <index> //read input to char array
 - arriin <arr> <index> //read input to int array
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

short strcmp(const char *str1, const char *str2) { // string compare
    int i = 0;
    while (str1[i] != '\0') {
        if (str1[i] != str2[i]) {
            return 1;
        }
        i++;
    }
    return 0;
}

short startsWith(const char *str1, const char *str2) {
    while (*str2 != '\0') {
        if (*str1 != *str2) {
            return 1;
        }
        str1++;
        str2++;
    }
    return 0;
}


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
    unsigned int loopCounter = 0;
    unsigned int msgCounter = 0; 
    vector<string> compiledProgram;
    vector<string> compiledVariables; //bss section
    vector<string> compiledData; //data section
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
    compiledProgram.push_back("    add esp, 0x4");
    compiledProgram.push_back("    ret");

    compiledProgram.push_back("prints:");
    compiledProgram.push_back("    .prints_loop:");
    compiledProgram.push_back("        mov ebx, [edx]");
    compiledProgram.push_back("        cmp ebx, 0x0");
    compiledProgram.push_back("        je .prints_end");
    compiledProgram.push_back("        push edx");
    compiledProgram.push_back("        call printc");
    compiledProgram.push_back("        pop edx");
    compiledProgram.push_back("        inc edx");
    compiledProgram.push_back("        jmp .prints_loop");
    compiledProgram.push_back("    .prints_end:");
    compiledProgram.push_back("        ret");


    compiledProgram.push_back("_start:");
    compiledProgram.push_back("    mov ebp, esp");
    compiledProgram.push_back("    sub esp, 0x10000"); //reserve 64kb of stack
    compiledProgram.push_back("    jmp main");

    compiledVariables.push_back("section .bss");
    compiledVariables.push_back("   loopTimes: resb 4");
    compiledVariables.push_back("   loopCounter: resb 4");

    compiledData.push_back("section .data");
    while (line < len) {
        string instruction = program[line];
        string wholeInstruction = instruction;
        string parameter1, parameter2, parameter3, parameter4, parameter5 = "";
        instruction.erase(0, instruction.find_first_not_of("\t "));

        int spaceIndex = instruction.find(' ');
        if (spaceIndex != string::npos) {
            parameter1 = instruction.substr(spaceIndex + 1);
            instruction = instruction.substr(0, spaceIndex);
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
        else if(instruction == "\n") line++;
        else if(wholeInstruction == " ") line++;
        else if(startsWith(instruction.c_str(), "#") == 0) {
            compiledProgram.push_back(";" + instruction);
        }
        else if(strcmp(instruction.c_str(), "int") == 0) {
            compiledVariables.push_back("   " + parameter1 + ": resb 4");
        }
        else if(strcmp(instruction.c_str(), "char") == 0) {
            compiledVariables.push_back("   " + parameter1 + ": resb 1");
        }
        else if(strcmp(instruction.c_str(), "array") == 0) {
            if(parameter1.find("char") != string::npos) {
                compiledVariables.push_back("   " + parameter2 + ": resb " + parameter3);
            } else {
                compiledVariables.push_back("   " + parameter2 + ": resd " + to_string(stoi(parameter3) * 4));
            }
        }
        else if(strcmp(instruction.c_str(), "function") == 0) {
            compiledProgram.push_back(parameter1 + ":");
            compiledProgram.push_back("    push ebp");
            compiledProgram.push_back("    mov ebp, esp");
            compiledProgram.push_back("    sub esp, 0x1000"); //reserve 4kb for function
        }
        else if(strcmp(instruction.c_str(), "end") == 0) {
            compiledProgram.push_back("    ret");
        }
        else if(strcmp(instruction.c_str(), "return") == 0) {
            compiledProgram.push_back("    mov esp, ebp");
            compiledProgram.push_back("    pop ebp");
            compiledProgram.push_back("    ret");
        }
        else if(strcmp(instruction.c_str(), "ifc") == 0) { //if condition for char
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
        else if(strcmp(instruction.c_str(), "ifi") == 0) { //if condition for int
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
        else if(strcmp(instruction.c_str(), "endif") == 0) {
            compiledProgram.push_back("    .endif" + to_string(endifCounter - 1) + ":");
        }
        else if(strcmp(instruction.c_str(), "add") == 0) {
            bool secondisint = false;
            if (isdigit(parameter3[0])) secondisint = true;
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            if (secondisint) compiledProgram.push_back("    add eax, " + parameter3);
            else compiledProgram.push_back("    add eax, [" + parameter3 + "]");
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        else if(strcmp(instruction.c_str(), "sub") == 0) {
            bool secondisint = false;
            if (isdigit(parameter3[0])) secondisint = true;
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            if (secondisint) compiledProgram.push_back("    sub eax, " + parameter3);
            else compiledProgram.push_back("    sub eax, [" + parameter3 + "]");
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        else if(strcmp(instruction.c_str(), "mul") == 0) {
            bool secondisint = false;
            if (isdigit(parameter3[0])) secondisint = true;
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            if (secondisint) compiledProgram.push_back("    imul eax, " + parameter3);
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        else if(strcmp(instruction.c_str(), "div") == 0) {
            bool secondisint = false;
            if (isdigit(parameter3[0])) secondisint = true;
            compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
            if (secondisint) compiledProgram.push_back("    cdq");
            compiledProgram.push_back("    idiv " + parameter3);
            compiledProgram.push_back("    mov [" + parameter1 + "], eax");
        }
        else if(strcmp(instruction.c_str(), "cin") == 0) { //char input
            compiledProgram.push_back("    call readc");
            compiledProgram.push_back("    mov byte [" + parameter1 + "], bl");
        }
        else if(strcmp(instruction.c_str(), "iin") == 0) { //int input
            compiledProgram.push_back("    call readc");
            compiledProgram.push_back("    sub ebx, 0x30");
            compiledProgram.push_back("    mov [" + parameter1 + "], ebx");
        }
        else if(strcmp(instruction.c_str(), "outc") == 0) { //char output
            compiledProgram.push_back("    mov ebx, [" + parameter1 + "]");
            compiledProgram.push_back("    call printc");
        }
        else if(strcmp(instruction.c_str(), "outi") == 0) { //int output
            compiledProgram.push_back("    mov ebx, [" + parameter1 + "]");
            compiledProgram.push_back("    add ebx, 0x30");
            compiledProgram.push_back("    call printc");
        }
        else if(strcmp(instruction.c_str(), "call") == 0) {
            compiledProgram.push_back("    call " + parameter1);
        }
        else if(strcmp(instruction.c_str(), "main") == 0) {
            compiledProgram.push_back("main:");
        }
        else if(strcmp(instruction.c_str(), "endProgram") == 0) {
            compiledProgram.push_back("    mov eax, 0x1");
            compiledProgram.push_back("    mov ebx, 0x0");
            compiledProgram.push_back("    int 0x80");
        }
        else if(strcmp(instruction.c_str(), "assigni") == 0) {
            bool isvar = false;
            if (!isdigit(parameter2[0])) isvar = true;
            if (isvar) {
                compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
                compiledProgram.push_back("    mov dword [" + parameter1 + "],eax");
            }
            else {
                compiledProgram.push_back("    mov dword [" + parameter1 + "]," + parameter2);
            }
        }
        else if(strcmp(instruction.c_str(), "assignc") == 0) {
            bool isvar = false;
            if (startsWith(parameter2.c_str(), "'")) isvar = true;
            if (isvar) {
                compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
                compiledProgram.push_back("    mov byte [" + parameter1 + "],al");
            }
            else {
                compiledProgram.push_back("    mov byte [" + parameter1 + "]," + parameter2);
            }
        }
        else if(strcmp(instruction.c_str(), "arrassignc") == 0) {
            bool isvar = false;
            if (!isdigit(parameter2[0])) isvar = true;
            if (isvar) {
                compiledProgram.push_back("    mov eax, [" + parameter3 + "]");
                compiledProgram.push_back("    mov ebx, [" + parameter2 + "]");
                compiledProgram.push_back("    mov byte [" + parameter1 + " + ebx],al");
            }
            else {
                compiledProgram.push_back("    mov byte [" + parameter1 + " + " + parameter2 + "]," + parameter3);
            }
        }
        else if(strcmp(instruction.c_str(), "arrassigni") == 0) {
            bool isvar = false;
            if (!isdigit(parameter2[0])) isvar = true;
            if (isvar) {
                compiledProgram.push_back("    mov eax, [" + parameter3 + "]");
                compiledProgram.push_back("    mov ebx, [" + parameter2 + "]");
                compiledProgram.push_back("    mov dword [" + parameter1 + " + ebx],eax");
            }
            else {
                compiledProgram.push_back("    mov dword [" + parameter1 + " + " + parameter2 + "]," + parameter3);
            }
        }
        else if(strcmp(instruction.c_str(), "loop") == 0) {
            bool isvar = false;
            if (!isdigit(parameter1[0])) isvar = true;
            if (isvar) {
                compiledProgram.push_back("    mov eax, [" + parameter1 + "]");
                compiledProgram.push_back("    mov dword [loopTimes], eax");
            }
            else {
                compiledProgram.push_back("    mov eax, " + parameter1);
                compiledProgram.push_back("    mov dword [loopTimes], eax");
            }
            compiledProgram.push_back("    .loop" + to_string(loopCounter) + ":");
            compiledProgram.push_back("    dec dword [loopTimes]");
            loopCounter++;
        }
        else if(strcmp(instruction.c_str(), "endloop") == 0) {
            compiledProgram.push_back("    cmp dword [loopTimes], 0");
            compiledProgram.push_back("    je .endloop" + to_string(loopCounter-1));
            compiledProgram.push_back("    jnz .loop" + to_string(loopCounter-1));
            compiledProgram.push_back("    .endloop" + to_string(loopCounter - 1) + ":");
        }
        else if(strcmp(instruction.c_str(), "inc") == 0) {
            compiledProgram.push_back("    inc dword [" + parameter1 + "]");
        }
        else if(strcmp(instruction.c_str(), "dec") == 0) {
            compiledProgram.push_back("    dec dword [" + parameter1 + "]");
        }
        else if(strcmp(instruction.c_str(), "prints") == 0) {
            string message = wholeInstruction.substr(wholeInstruction.find("prints") + 7);
            compiledData.push_back("    msg" + to_string(msgCounter) + ": db " + message + ", 0x0");
            compiledProgram.push_back("    mov edx, msg" + to_string(msgCounter));
            compiledProgram.push_back("    call prints");
            msgCounter++;
        }
        else if(strcmp(instruction.c_str(), "arrcpy") == 0) {
            bool isvar = false;
            if (!isdigit(parameter3[0])) isvar = true;
            if (isvar) {
                compiledProgram.push_back("    mov eax, [" + parameter3 + "]");
                compiledProgram.push_back("    mov ebx, [" + parameter4 + "]");
                if(parameter1 == "int") {
                    compiledProgram.push_back("    mov dword [" + parameter2 + "ebx], eax");
                } else {
                    compiledProgram.push_back("    mov byte [" + parameter2 + "ebx], eax");
                }
            }
            else {
                compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
                if(parameter1 == "int") {
                    compiledProgram.push_back("    mov dword [" + parameter2 + " + " + parameter4 + "], eax");
                } else {
                    compiledProgram.push_back("    mov byte [" + parameter2 + " + " + parameter4 + "], eax");
                }
            }
        }
        else if(strcmp(instruction.c_str(), "arroutc") == 0) {
            bool isvar = false;
            if (!isdigit(parameter2[0])) isvar = true;
            if (isvar) {
                compiledProgram.push_back("    mov eax, [" + parameter2 + "]");
                compiledProgram.push_back("    mov ebx, [" + parameter1 + " + eax]");
            }
            else {
                compiledProgram.push_back("    mov ebx, [" + parameter1 + " + " + parameter2 + "]");
            }
            compiledProgram.push_back("    call printc");
        }
        line++;
    }

    for(int i = 0; i < compiledProgram.size(); i++) {
        fullyCompiledProgram.push_back(compiledProgram[i]);
    }

    for(int i = 0; i < compiledData.size(); i++) {
        fullyCompiledProgram.push_back(compiledData[i]);
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
            if (!cline.empty()) {
                WULFprogram.push_back(cline);
            }
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

