#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/code_generator/generator.h"
#include "../../include/code_generator/instructions.h"

void instrucao(char *instrucao, char *operando1, char *operando2) {

    printf("\n Instrucao '%s': \n", instrucao);

    if(strcmp(instrucao,"inicia_prog") == 0){
        Gera("","START","","");
        return;
    }

    if(strcmp(instrucao,"finaliza_prog") == 0){
        Gera("", "HLT","","");
        return;
    }

    if (!instrucao) return; // só fazendo pequena validação quando tiver sem a instrucao
    //se der erro aqui é erro do programador e nao do usuario( em uma chamada da funcao instrucao())

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


    if(strcmp(instrucao,"var") == 0){
        
        Gera("","ALLOC", operando1, operando2);
        return;
    }

    if(strcmp(instrucao,"var_dalloc") == 0){
        
        Gera("","DALLOC", operando1, operando2);
        return;
    }

    printf(" Instrucao desconhecida: %s\n", instrucao);
}
