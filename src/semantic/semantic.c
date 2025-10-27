#include <stdio.h>
#include <stdlib.h>
#include "../../include/semantic/semantic.h"


void insere_tabela(char *lexema, char *tipo, char escopo, char *mem){
    (void)lexema;
    (void)tipo;
    (void)escopo;
    (void)mem;
    //todo
    // inserir no topo da tabela de simbolos
    //inserir na tabela de simbolos
    return;
}

int pesquisa_duplicacvar_tabela(char *lexema){ //boolean
    (void)lexema;
    //todo:
    //verificar se o lexema do identificador nao está duplicada na tabela de smbolos
    return 0;
}

void coloca_tipo_tabela(char *lexema){
    (void)lexema;
    //todo:
    //não sei mt o que fazer aqui, dar uma olhada melhor kk
    // pagina 47 do livro dele
    return;
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

int pesquisa_tabela(char *lexema, int *ind,Tabsimb *tabela_simbolos){
    (void)lexema;
    (void)ind;
    (void)tabela_simbolos;
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


/* todos os booleans (declarados com int) são nesse formato:
     0 = falso
     1 =  true
*/
