#ifndef SEMANTIC_H
#define SEMANTIC_H

typedef struct {
    char lexema[10]; //nome do identificador
    char escopo[10]; //nivel de declaração
    char tipo[10]; //padrao do identificador
    char memoria[10]; //endereço de memoria alocada
}Tabsimb;


//Prototype

void insere_tabela(char *lexema, char *tipo, char escopo, char *mem);
int pesquisa_duplicacvar_tabela(char *lexema); 
void coloca_tipo_tabela(char *lexema);
int pesquisa_declvar_tabela(char *lexema);  
int pesquisa_declvarfunc_tabela(char *lexema); 
int pesquisa_declproc_tabela(char *lexema);
void desempilha_ou_voltanivel();
int pesquisa_tabela(char *lexema, int *ind,Tabsimb *tabela_simbolos);
int pesquisa_declfunc_tabela(char *lexema);


#endif