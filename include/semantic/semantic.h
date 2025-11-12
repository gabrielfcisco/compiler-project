#ifndef SEMANTIC_H
#define SEMANTIC_H

// typedef struct {
//     char lexema[10]; //nome do identificador
//     char escopo[10]; //nivel de declaração
//     char tipo[10]; //padrao do identificador
//     char memoria[10]; //endereço de memoria alocada
// }Tabsimb;

typedef struct{
    char* lexema;
    char tipo[20];
    char escopo;
    int end; 
    
} Tabsimb;


#define push(sp, n) ( ((sp)++), *(sp) = (n)) //primeiro incrementa e depois atribui
#define pop(sp) (--(sp))



//Prototype
Tabsimb* initialize_stack();
void insere_tabela(char *lexema, char *tipo_inicial, char escopo, int end);
int pesquisa_duplica_var_tabela(char *lexema); 
void coloca_tipo_tabela(char *lexema);
int pesquisa_declvar_tabela(char *lexema);  
int pesquisa_declvarfunc_tabela(char *lexema); 
int pesquisa_declproc_tabela(char *lexema);
void desempilha_ou_voltanivel();
int pesquisa_tabela(char *lexema, Tabsimb **sp_func);
int pesquisa_declfunc_tabela(char *lexema);
void imprimir_tabela_simbolos();


#endif