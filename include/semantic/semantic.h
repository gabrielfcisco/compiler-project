#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../../include/lexical/token.h"

typedef struct{
    char* lexema;
    char tipo[20];
    char escopo;
    int end; 
    
} Tabsimb;


#define push(sp, n) ( ((sp)++), *(sp) = (n)) //primeiro incrementa e depois atribui
#define pop(sp) (--(sp))



//Prototype
Tabsimb** initialize_stack();
void insere_tabela(char *lexema, char *tipo_inicial, char escopo, int end, int *endereco_var, int flag);
int pesquisa_duplica_var_tabela(char *lexema); 
void coloca_tipo_tabela(char *lexema);
int pesquisa_declproc_dup_tabela(char *lexema);
void desempilha_ou_voltanivel();
int pesquisa_tabela(char *lexema, Tabsimb **sp_func);
int pesquisa_declfunc_dup_tabela(char *lexema);
int verifica_tipo_pos_fixa (token *vetor_pos_fixa, int posf);
int verifica_tipo (token t);
int is_operator(token t);

void imprimir_tabela_simbolos();



#endif