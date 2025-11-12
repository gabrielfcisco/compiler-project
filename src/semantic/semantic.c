#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/semantic/semantic.h"



// 1 FOUND  
// 0 NOT_FOUND

Tabsimb symbol_table[1000];

Tabsimb* sp;
Tabsimb* init;

void imprimir_simbolo(Tabsimb* sp) {

    printf("--- Conteúdo do Simbolo ---\n");
    printf("Lexema: %s\n", sp->lexema);
    printf("Tipo: %s\n", sp->tipo);
    printf("Escopo: %c\n", sp->escopo);
    printf("Endereco: %d", sp->end);
    printf("-------------------------\n\n");
}

Tabsimb* initialize_stack(){
    sp = symbol_table;
    init = sp;
    return sp;
}

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

void remove_tabela(){
    pop(sp);
}

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

int pesquisa_declvar_tabela(char *lexema){  //boolean

    Tabsimb* sp_aux = sp;
    while (sp_aux != init){            
        if(strcmp(sp_aux->lexema, lexema) == 0){ 
            return 1;
        }
        sp_aux --;
    }
    return 0;
}

int pesquisa_declvarfunc_tabela(char *lexema){              //nao sei se declvarfunc e declvar sao iguais mesmo ou tem alguma diferenca, necessario verificar

    Tabsimb* sp_aux = sp;
    while (sp_aux != init){            
        if(strcmp(sp_aux->lexema, lexema) == 0){ 
            return 1;
        }
        sp_aux --;
    }
    return 0;
}

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

void desempilha_ou_voltanivel(){
    while(sp != init && sp->escopo != 'L'){
        remove_tabela();
    }
    sp->escopo = ' ';
}

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
