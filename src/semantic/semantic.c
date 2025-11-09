#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/semantic/semantic.h"


int endereco_var = 0;

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

void insere_tabela(char *lexema, char *tipo_inicial, char escopo, int rotulo){

    Tabsimb aux;

    aux.lexema = malloc(strlen(lexema) + 1);
    if (aux.lexema != NULL) {
        strcpy(aux.lexema, lexema);
    }
    
    strcpy(aux.tipo, tipo_inicial);

    aux.escopo = escopo;
    
    if(escopo == 'L'){
        aux.end = rotulo;
    }
    else {
        aux.end = endereco_var;
        endereco_var ++;
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
    (void)lexema;
    //todo:
    // no caso, se nao estiver duplicado =  false
    return 0;
}

int pesquisa_declvarfunc_tabela(char *lexema){ // boolean
    (void)lexema;
    //todo
    return 0;
}

int pesquisa_declproc_tabela(char *lexema){
    (void)lexema;
    //todo: pagina 50 livro dele
    return 0;
}

void desempilha_ou_voltanivel(){
    // todo : pagina 50 do livro dele
    return;
}

int pesquisa_tabela(char *lexema){
 
    // TODO : 
    /*
    nesse caso tera que pegar o lexema que foi passado e procurar se existe na tabela, se existir:
     retorna 1,(true)
     senao retorna 0 (false)

    lembrando que tem um detalhe do nivel, nao lembro muito bem esses conceitos se é ate a marca que procura ou se é ate o final
    imagino que seja ate o final, mas seria bom revisar.

    lembrando que o endereço de ind foi passado então estaremos manipulando diretamente o valor de ind de dentro da função "analisa_fator"
    entao deveremos depositar o valor do indice da variavel encontrada pela função para o valor de ind, exemplo:

     *ind = posição_do_lexema_procurado_na_lista_de_tabela_de_simbolos
     ^
     (com o asterisco)

    */
   return 1;
}

int pesquisa_declfunc_tabela(char *lexema){
    (void)lexema;
    //TODO : declaração de função ne
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
