#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/semantic/semantic.h"

// 1 FOUND  
// 0 NOT_FOUND

Tabsimb symbol_table[1000];

Tabsimb* sp;
Tabsimb* init;

// Imprime os dados de um símbolo da tabela de símbolos.
void imprimir_simbolo(Tabsimb* sp) {

    printf("--- Conteúdo do Simbolo ---\n");
    printf("Lexema: %s\n", sp->lexema);
    printf("Tipo: %s\n", sp->tipo);
    printf("Escopo: %c\n", sp->escopo);
    printf("Endereco: %d", sp->end);
    printf("-------------------------\n\n");
}

// Inicializa a pilha/tabela de símbolos e retorna ponteiro para o topo.
Tabsimb** initialize_stack(){
    sp = symbol_table;
    init = sp;
    return &sp;
}

// Insere um novo símbolo na tabela de símbolos, com tipo, escopo e endereço.
void insere_tabela(char *lexema, char *tipo_inicial, char escopo, int rotulo, int *endereco_var, int flag){

    Tabsimb aux;

    aux.lexema = malloc(strlen(lexema) + 1);
    if (aux.lexema != NULL) {
        strcpy(aux.lexema, lexema);
    }
    
    strcpy(aux.tipo, tipo_inicial);

    aux.escopo = escopo;
    
    if(escopo == 'L' && flag == 0){
        aux.end = rotulo;
    }
    else
    if (flag == 1){
        aux.end = *endereco_var;
        (*endereco_var) ++;
    }
    
    push(sp, aux);
    // imprimir_simbolo(sp);
}

// Remove o símbolo do topo da tabela de símbolos.
void remove_tabela(){
    pop(sp);
}

// Verifica se já existe uma variável com o mesmo nome declarada (duplicidade).
int pesquisa_duplica_var_tabela(char *lexema){ //boolean
    Tabsimb* sp_aux = sp;
    while (sp_aux != init && sp_aux->escopo != 'L'){            // Agora o inicio nao possui nada, pois o push primeiro incrementa, depois atribui
        if(strcmp(sp_aux->lexema, lexema) == 0){ 
            return 1;
        }
        sp_aux --;
    }
    return 0;
}

// Atribui o tipo correto às variáveis recém-inseridas na tabela de símbolos.
void coloca_tipo_tabela(char *lexema){
    Tabsimb* sp_aux = sp;
     while (sp_aux != init){
        if (strcmp(sp_aux->tipo, "variavel") == 0){
            strcpy(sp_aux->tipo, lexema);
            // printf("Tipo %s atribuido ao lexema %s\n", lexema, sp_aux->lexema);
        }
        sp_aux --;
     }
}

// Verifica duplicidade de declaração de procedimento.
int pesquisa_declproc_dup_tabela(char *lexema){

    Tabsimb* sp_aux = sp;
    while (sp_aux != init){            
        if(strcmp(sp_aux->lexema, lexema) == 0){ 
            return 1;
        }
        sp_aux --;
    }
    return 0;
}

// Desempilha símbolos até encontrar o início do bloco local (escopo 'L').
void desempilha_ou_voltanivel(){
    while(sp != init && sp->escopo != 'L'){
        remove_tabela();
    }
    sp->escopo = ' ';
}

// Procura um símbolo na tabela e retorna ponteiro via argumento se encontrado.
int pesquisa_tabela(char *lexema, Tabsimb** sp_func){

    Tabsimb* sp_aux = sp;
    while (sp_aux != init){            
        if(strcmp(sp_aux->lexema, lexema) == 0){
            *sp_func = sp_aux; 
            return 1;
            
        }
        sp_aux --;
    }
    return 0;

}

// Verifica duplicidade de declaração de função.
int pesquisa_declfunc_dup_tabela(char *lexema){

    Tabsimb* sp_aux = sp;
    while (sp_aux != init){            
        if(strcmp(sp_aux->lexema, lexema) == 0){ 
            return 1;
        }
        sp_aux --;
    }
    return 0;
}

// Verifica o tipo resultante de uma expressão pós-fixa (0=int, 1=bool, -1=erro) e retorna o topo da pilha com o ultimo tipo valido.
int verifica_tipo_pos_fixa (token *vetor_pos_fixa, int posf){

    int pilha[1000];
    int topo = -1;

    int a, b;

    for (int i = 0; i < posf; i++){

        token t = vetor_pos_fixa[i];
        /* Caso 1: Operando */
        if (!is_operator(t)) {

            int tipo = verifica_tipo(t);  // 0=int, 1=bool
            if (tipo < 0) return -1;  //tipo invalido
            pilha[++topo] = tipo;
            continue;
        }

        /* Caso 2: Operador Unário */
        if (strcmp(t.simbolo, "sinv") == 0 || strcmp(t.simbolo, "snao") == 0) {

            if (topo < 0) return -1;

            a = pilha[topo--];
            /* sinv → só aceita inteiro */
            if (strcmp(t.simbolo, "sinv") == 0){
                if (a != 0) return -1;
                pilha[++topo] = 0;
            }
            /* snao → só aceita booleano */
            else if (strcmp(t.simbolo, "snao") == 0){
                if (a != 1) return -1;
                pilha[++topo] = 1;
            }

            continue;
        }

        /* Caso 3: Operadores Binários */
        if (topo < 1) return -1;  //necessita dois operadores

        b = pilha[topo--];
        a = pilha[topo--];

        /* RELACIONAIS: resultado é booleano (1) */
        if (strcmp(t.simbolo, "smaior")   == 0 ||
            strcmp(t.simbolo, "smenor")   == 0 ||
            strcmp(t.simbolo, "smaiorig") == 0 ||
            strcmp(t.simbolo, "smenorig") == 0 ||
            strcmp(t.simbolo, "sig")      == 0 ||
            strcmp(t.simbolo, "sdif")     == 0)
        {
            if (a != 0 || b != 0)   // relacionais so aceitam inteiros
                return -1;

            pilha[++topo] = 1; // booleano
            continue;
        }

        /* LÓGICOS */
        if (strcmp(t.simbolo, "se") == 0 ||    // AND
            strcmp(t.simbolo, "sou") == 0)     // OR
        {
            if (a != 1 || b != 1)  // so booleano
                return -1;

            pilha[++topo] = 1;
            continue;
        }

        /* Caso 4: Aritméticos */
        if (strcmp(t.simbolo, "smais")  == 0 ||
            strcmp(t.simbolo, "smenos") == 0 ||
            strcmp(t.simbolo, "smult")  == 0 ||
            strcmp(t.simbolo, "sdiv")   == 0)
        {
            if (a != 0 || b != 0)
                return -1;

            pilha[++topo] = 0;
            continue;
        }

        /* Operador desconhecido */
        return -1;
        

    }
    if (topo == 0){
        return pilha[topo]; // retorna o topo para comparacao com o left_side da atribuicao
    }
    else{
        return -1;
    }
}

// Retorna o tipo de um token (0=int, 1=bool, -1=inválido).
int verifica_tipo (token t){

    Tabsimb *sp_aux;
    
    if(strcmp(t.simbolo, "snumero") == 0){
        return 0;
    }
    if (strcmp(t.simbolo, "sverdadeiro") == 0 || strcmp(t.simbolo, "sfalso") == 0){
        return 1; // booleano
    }
    if (pesquisa_tabela(t.lexema, &sp_aux) == 1){
        if(strcmp(sp_aux->tipo, "inteiro") == 0 || strcmp(sp_aux->tipo, "funcao inteiro") == 0){
            return 0;  //inteiro
        }
        else{
            return 1; //booleano
        }
    }

    return -1;  //tipo invalido
   
}

// Retorna 1 se o token for operador, 0 caso contrário.
int is_operator(token t) {

    if (strcmp(t.simbolo, "smaior") == 0   || strcmp(t.simbolo, "smaiorig") == 0 ||
        strcmp(t.simbolo, "smenor") == 0   || strcmp(t.simbolo, "smenorig") == 0 ||
        strcmp(t.simbolo, "sig")    == 0   || strcmp(t.simbolo, "sdif")     == 0 ||
        strcmp(t.simbolo, "smais")  == 0   || strcmp(t.simbolo, "smenos")   == 0 ||
        strcmp(t.simbolo, "sou")    == 0   || strcmp(t.simbolo, "smult")    == 0 ||
        strcmp(t.simbolo, "sdiv")   == 0   || strcmp(t.simbolo, "se")       == 0 ||
        strcmp(t.simbolo, "sinv")   == 0   || strcmp(t.simbolo, "snao")     == 0
    ){  
        return 1;
    }
    else{
        return 0;
    }
 
}

// Imprime toda a tabela de símbolos atual.
void imprimir_tabela_simbolos(){
    Tabsimb* sp_aux = init + 1; //pula o primeiro que é vazio
    printf("\n--- Tabela de Simbolos ---\n");
    while (sp_aux != sp + 1){ // como o push primeiro incrementa, depois atribui, entao o sp aponta para o ultimo elemento adicionado
        imprimir_simbolo(sp_aux);
        sp_aux ++;
    }
}


/* todos os booleans (declarados com int) são nesse formato:
     0 = falso
     1 =  true
*/
