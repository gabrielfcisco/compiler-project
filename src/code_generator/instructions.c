#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/code_generator/generator.h"
#include "../../include/code_generator/instructions.h"

void instrucao(char *instrucao, char *operando1, char *operando2) {
    (void)operando2; // nao usa o segundo operador por enquanto

    printf("\n Instrucao '%s': \n", instrucao);

    if(strcmp(instrucao,"inicia_prog") == 0){
        Gera("","START","","");
        return;
    }

    if(strcmp(instrucao,"finaliza_prog") == 0){
        Gera("", "HLT","","");
        return;
    }

    if (!operando1) return; // só fazendo pequena validação quando tiver sem o primeiro operador
    //(só dará erro se o programador escrever errado!)

    //caso quando for só para marcar a linha para um jmp
    if (strcmp(instrucao, "label") == 0) {
        Gera(operando1, "NULL", "", "");
        return;
    }

    if (strcmp(instrucao, "jmp") == 0) {
        Gera("", "JMP", operando1, "");
        return;
    }

    if (strcmp(instrucao, "jmpf") == 0) {
        Gera("", "JMPF", operando1, "");
        return;
    }

    /* leia(x):
     RD ;
     STR x 
    */
    if (strcmp(instrucao, "leia") == 0) {
        Gera("", "RD",  "",  "");
        Gera("", "STR", operando1 , "");
        return;
    }

    /* escreva(x): 
     LDV x ;
     PRN 
    */
    if (strcmp(instrucao, "escreva") == 0) {
        Gera("", "LDV", operando1, "");
        Gera("", "PRN", "", "");
        return;
    }

    printf(" Instrucao desconhecida: %s\n", instrucao);
}
