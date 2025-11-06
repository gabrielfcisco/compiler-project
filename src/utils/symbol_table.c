#include <stdio.h>
#include <stdlib.h>
#include "../lexical/token.h"
#include "symbol_table.h"

symbol symbol_table[1000];

symbol* sp;
symbol* init;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))

void initialize_stack(){
    sp = symbol_table;
    init = sp;
}

void insere_tabela(token t, char escopo){
    symbol aux;
    aux.lexema = malloc(strlen(t.lexema));
    if (aux.lexema != NULL) {
        strcpy(aux.lexema, t.lexema);
    }
    aux.linha = t.linha;
    aux.escopo = escopo;

    push(sp, aux);
}

void remove_tabela(){
    pop(sp);
}

int pesquisa_duplicvar_tabela(char* lexema){
    symbol* sp_aux = sp;
    while (sp_aux != NULL && sp_aux->escopo != 'L'){
        if(strcmp(sp_aux->lexema, lexema) == 0){
            return 1;
        }
        (*--(sp_aux));
    }
    return 0;
}


