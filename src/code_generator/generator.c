#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/code_generator/generator.h"



void Gera(int rotulo, char *instrucao, char *operando1, char *operando2){
    generator_struct g;

    strcpy(g.rotulo, rotulo);
    strcpy(g.instrucao, instrucao);
    strcpy(g.operando1, operando1);
    strcpy(g.operando2, operando2);

    Gera_arquivo(g);
}

void Gera_arquivo(generator_struct g){
    
}