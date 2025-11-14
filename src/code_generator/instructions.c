#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/code_generator/generator.h"
#include "../../include/code_generator/instructions.h"
#include "../../include/lexical/token.h"
#include "../../include/semantic/semantic.h"

int verify_if_is_aritmetic(char *operando);
int verify_if_is_relational(char *operando);

void instrucao(char *instrucao, char *operando1, char *operando2) {

    printf("\n Instrucao '%s': \n", instrucao);

    if(strcmp(instrucao,"inicia_prog") == 0){
        Gera("","START","","");
        Gera("","ALLOC","0","1");
        return;
    }

    if(strcmp(instrucao,"finaliza_prog") == 0){
        Gera("","DALLOC","0","1");
        Gera("", "HLT","","");
        return;
    }

    if (!instrucao) return; // só fazendo pequena validação quando tiver sem a instrucao
    //se der erro aqui é erro do programador e nao do usuario( em uma chamada da funcao instrucao())

    //caso quando for só para marcar a linha para um jmp
    if (strcmp(instrucao, "label") == 0) {

        Gera(operando1, "NULL", "", "");
        return;
    }

    if (strcmp(instrucao, "jmp") == 0) {
        Gera("", "JMP", operando1, "");
        return;
    }

    if (strcmp(instrucao, "jmpf") == 0) {
        Gera("", "JMPF", operando1, "");
        return;
    }

    /* leia(x):
     RD ;
     STR x 
    */
    if (strcmp(instrucao, "leia") == 0) {
        Gera("", "RD",  "",  "");
        Gera("", "STR", operando1 , "");
        return;
    }

    /* escreva(x): 
     LDV x ;
     PRN 
    */
    if (strcmp(instrucao, "escreva") == 0) {
        Gera("", "LDV", operando1, "");
        Gera("", "PRN", "", "");
        return;
    }


    if(strcmp(instrucao,"var") == 0){
        
        Gera("","ALLOC", operando1, operando2);
        return;
    }

    if(strcmp(instrucao,"var_dalloc") == 0){
        
        Gera("","DALLOC", operando1, operando2);
        return;
    }

    if((strcmp(instrucao,"operacao_var") == 0)){
        Gera("", "LDV", operando1 , "");
        return;
    }

    if(strcmp(instrucao,"atribuicao") == 0){
        Gera("","STR",operando1,"");
        return;
    }

    if((strcmp(instrucao,"operacao_num") == 0)){
        Gera("", "LDC", operando1 , "");
        return;
    }
    
    if(strcmp(instrucao,"operacao") == 0){

        if(verify_if_is_aritmetic(operando1) == 1){
            return;
        }

        if(verify_if_is_relational(operando1)==1){
            return;
        }
    }

    if((strcmp(instrucao,"atribuicao") == 0)){
        Gera("", "LDV", operando1 , "");
        return;
    }

    if(strcmp(instrucao,"chamada") == 0){

        if(strcmp(operando2,"proc") == 0){
            Gera("", "CALL", operando1, "");
            return;
        }

        if(strcmp(operando2,"funcao") == 0){
            Gera("", "CALL", operando1 , "");
            Gera("","LDV","0","");
            return;
        }

    }

    if((strcmp(instrucao,"return") == 0)){
        if(strcmp(operando1,"procedimento")){
            Gera("", "RETURN","", "");
            return;
        }
    }
    

    printf(" Instrucao desconhecida: %s\n", instrucao);
}

void ins_expressao(token *vetor_pos_fixa, int posf){
    
    Tabsimb *sp_aux;   // endereco auxiliar para ver se o identificador encontrado e uma funcao
    char *endereco;

    for (int i = 0; i < posf; i++){
        if (pesquisa_tabela(vetor_pos_fixa[i].lexema, &sp_aux) == 1){
            if((strcmp(sp_aux->tipo, "funcao inteiro") == 0 || strcmp(sp_aux->tipo, "funcao booleano") == 0)){
                char *endereco = convert_integer_to_string(sp_aux->end);  //endereco aqui e rotulo
                instrucao("chamada", endereco, "funcao");
                free(endereco);
            }else{
                endereco = convert_integer_to_string(sp_aux->end);
                instrucao("operacao_var", endereco, "");
                free(endereco);
            }

        }else if(strcmp(vetor_pos_fixa[i].simbolo, "snumero") == 0){
            instrucao("operacao_num",vetor_pos_fixa[i].lexema, ""); 

        }else{
            instrucao("operacao",vetor_pos_fixa[i].lexema,"");
        }
    }
}

void ins_atr_expressao(char *lexema){
    Tabsimb *sp_aux;
    char *endereco;
    if (pesquisa_tabela(lexema, &sp_aux) == 1) {  
        if (strcmp(sp_aux->tipo, "funcao inteiro") == 0 || strcmp(sp_aux->tipo, "funcao booleano") == 0){
            instrucao("atribuicao", "0", "");

        }else{
            endereco = convert_integer_to_string (sp_aux->end);
            instrucao("atribuicao", endereco, "");
            free(endereco);
        }
    }
}

int verify_if_is_aritmetic(char *operando){
    if (strcmp(operando, "+") == 0){
            Gera("","ADD","","");
            return 1;
        }
        
        if (strcmp(operando,"-") == 0 ){
            Gera("","SUB","","");
            return 1;
        }

        if (strcmp(operando,"*") == 0){
            Gera("","MULT","","");
            return 1;
        }

        if (strcmp(operando,"div") == 0){
            Gera("","DIV","","");
            return 1;
        }

        if (strcmp(operando,"inv") == 0){
            Gera("","INV","","");
            return 1;
        }

        
        return 0;
}

int verify_if_is_relational(char *operando){
    if (strcmp(operando,"e") == 0){
            Gera("","AND","","");
            return 1;
        }

        if (strcmp(operando,"ou") == 0){
            Gera("","OR","","");
            return 1;
        }

        if (strcmp(operando,"neg") == 0){
            Gera("","NEG","","");
            return 1;
        }

        if (strcmp(operando,"<") == 0){
            Gera("","CME","","");
            return 1;
        }

        if (strcmp(operando,"<=") == 0){
            Gera("","CMEQ","","");
            return 1;
        }

        if (strcmp(operando,">") ==0){
            Gera("","CMA","","");
            return 1;
        }

        if (strcmp(operando,">=") == 0){
            Gera("","CMAQ","","");
            return 1;
        }

        if (strcmp(operando,"<") == 0){
            Gera("","CME","","");
            return 1;
        }

        if (strcmp(operando,"=") == 0){
            Gera("","CEQ","","");
            return 1;
        }

        if (strcmp(operando,"!=") == 0){
            Gera("","CDIF","","");
            return 1;
        }

        
        return 1;
}

char* deep_copy(const char *orig) {
    char *copia = malloc(strlen(orig) + 1); // +1 para o '\0'
    if (copia == NULL) return NULL;

    strcpy(copia, orig);
    return copia;
}