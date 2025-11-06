#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef struct symbol{
    char* lexema;
    char tipo[20];
    int linha; //alterar para endereco
    char escopo;
} symbol;

#endif