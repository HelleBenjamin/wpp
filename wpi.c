#include <stdio.h>

/*Wuf+ Interpreter
Wuf+ is minimalistic, based on Wuf++
Syntax:
+ - increment current memory location
- - decrement current memory location
> - print current memory location as ascii character
; - increment current memory location pointer, if memory location is zero, decrement current memory location pointer
*/

int main(){
    unsigned char memarray[0xff] = {0};
    unsigned char mempointer = 0;

    while(1){
        char c = getchar();
        switch(c){
            case '+':
                memarray[mempointer]++;
                break;
            case '-':
                memarray[mempointer]--;
                break;
            case '>':
                putchar(memarray[mempointer]);
                break;
            case ';':
                if(memarray[mempointer] == 0){
                    mempointer--;
                } else {
                    mempointer++;
                }
                break;
        }
    }

    return 0;
}