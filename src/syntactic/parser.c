#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/parser/parser.h"
#include "../../include/lexical/lexer.h"
#include "../../include/semantic/semantic.h"
#include "../../include/code_generator/generator.h"
#include "../../include/code_generator/instructions.h"

Tabsimb* sp_parser;
int rotulo = 1;

void imprimir_token(token t) {

    printf("--- Conteúdo do Token ---\n");
    printf("Lexema: %s\n", t.lexema);
    printf("Símbolo: %s\n", t.simbolo);
    printf("Linha: %d\n", t.linha);
    printf("Erro: %d (%s)\n", t.erro, t.erro == 0 ? "ok" : "erro");
    printf("-------------------------\n\n");
}

void atualiza_in_fixa(token *in_fixa, int *pos, token t) {

    in_fixa[*pos] = token_create(t.lexema, t.simbolo, t.linha);
    (*pos)++;
}

void print_in_and_pos_fixa(token *vetor_tokens, int pos, int fixa) {
    if(fixa == 0){
        printf("Expressão em notação infixa: ");
        for (int i = 0; i < pos; i++) {
            printf("%s ", vetor_tokens[i].lexema);
        }
        printf("\n");
    }
    else {
        printf("Expressão em notação posfixa: ");
        for (int i = 0; i < pos; i++) {
            printf("%s ", vetor_tokens[i].lexema);
        }
        printf("\n");
    }
}

int precedencia(token t) {

    if (t.unario == 1) return 7; // maior prioridade para operadores unários   
    if (strcmp(t.simbolo, "smult")    == 0  || strcmp(t.simbolo, "sdiv")     == 0) return 6;
    if (strcmp(t.simbolo, "smais")    == 0  || strcmp(t.simbolo, "smenos")   == 0) return 5;
    if (strcmp(t.simbolo, "smaior")   == 0  || strcmp(t.simbolo, "smenor")   == 0 ||
        strcmp(t.simbolo, "smaiorig") == 0  || strcmp(t.simbolo, "smenorig") == 0 ||
        strcmp(t.simbolo, "sdif")     == 0) return 4;
    if (strcmp(t.simbolo, "snao") == 0) return 3;  
    if (strcmp(t.simbolo, "se")   == 0) return 2;
    if (strcmp(t.simbolo, "sou")  == 0) return 1;
    return 0; // menor prioridade possível (ou token inválido)

}

token *pos_fixa (token *in_fixa, int pos, int *posf) {
    // Implementação do algoritmo Shunting Yard para converter infixa para posfixa
    token *pilha = (token *)malloc(pos * sizeof(token));
    token *out = (token *)malloc(pos * sizeof(token));
    int topo = -1;
    *posf = 0;

    for (int i = 0; i < pos; i++) {
        token t = in_fixa[i];

        if (strcmp(t.simbolo, "snumero") == 0 || strcmp(t.simbolo, "sidentificador") == 0) {
            out[(*posf)++] = token_create(t.lexema, t.simbolo, t.linha);
        } else if (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0 ||
                   strcmp(t.simbolo, "smult") == 0 || strcmp(t.simbolo, "sdiv") == 0 ||
                   strcmp(t.simbolo, "se") == 0 || strcmp(t.simbolo, "sou") == 0 ||
                   strcmp(t.simbolo, "smaior") == 0 || strcmp(t.simbolo, "smenor") == 0 ||
                   strcmp(t.simbolo, "smaiorig") == 0 || strcmp(t.simbolo, "smenorig") == 0 ||
                   strcmp(t.simbolo, "sdif") == 0 || strcmp(t.simbolo, "snao") == 0) {

            while (topo != -1 && strcmp(pilha[topo].simbolo, "sabre_parenteses") != 0 && precedencia(pilha[topo]) >= precedencia(t)) {
                out[(*posf)++] = pilha[topo--];
            }
            pilha[++topo] = token_create(t.lexema, t.simbolo, t.linha);
        } else if (strcmp(t.simbolo, "sabre_parenteses") == 0) {
            pilha[++topo] = token_create(t.lexema, t.simbolo, t.linha);
        } else if (strcmp(t.simbolo, "sfecha_parenteses") == 0) {
            while (topo != -1 && strcmp(pilha[topo].simbolo, "sabre_parenteses") != 0) {
                out[(*posf)++] = pilha[topo--];
            }
            if (topo != -1) {
                token_free(&pilha[topo--]); // Remove o parêntese de abertura
            }
        }
    }
    while (topo != -1) {
        out[(*posf)++] = pilha[topo--];
    }
    free(pilha);
    return out;
}

token analisa_tipo(parser *p) {

    if(strcmp(p->t.simbolo, "sinteiro") != 0 && strcmp(p->t.simbolo, "sbooleano") != 0){
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        token_free(&p->t);
        exit(1);
    }else{
        printf("HEEloo");
        coloca_tipo_tabela(p->t.lexema);
    }
    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    return p->t;
}

token analisa_variaveis(parser *p, int *counter_var){

    while(strcmp(p->t.simbolo, "sdoispontos") != 0){
        if(strcmp(p->t.simbolo, "sidentificador") == 0){
            if(pesquisa_duplica_var_tabela(p->t.lexema) == 0){
                insere_tabela(p->t.lexema,"variavel",' ', 0);
                (*counter_var)++; // conta cada variavel dentro do bloco ex: var a,b,c inteiro;

                token_free(&p->t);
                p->t = lexer(p->file, p->out);
                if(strcmp(p->t.simbolo, "svirgula") == 0 || strcmp(p->t.simbolo, "sdoispontos") == 0){
                    if(strcmp(p->t.simbolo, "svirgula") == 0){
                        token_free(&p->t);
                        p->t = lexer(p->file, p->out);
                        if(strcmp(p->t.simbolo, "sdoispontos") == 0){
                            printf("\nERRO: linha %d, token: %s", p->t.linha, p->t.lexema);
                            exit(1);
                        }
                    }
                }else{
                    printf("\nERRO: linha %d, token: %s", p->t.linha, p->t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO semantico: linha %d, token: %s", p->t.linha, p->t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
            exit(1);
        }
    }
    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    return analisa_tipo(p);
}

token analisa_et_variaveis(parser *p,int *counter_var){

    if(strcmp(p->t.simbolo, "svar") == 0){
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        if(strcmp(p->t.simbolo, "sidentificador") == 0){
            while(strcmp(p->t.simbolo, "sidentificador") == 0){
                p->t = analisa_variaveis(p,counter_var);         // passa o ponteiro do contador de variaveis para contar

                if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);
                } else {
                    printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
                    exit(1);
                }
            }
        }else{
            printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
            exit(1);
        }
    }
    return p->t;
}


token analisa_declaracao_procedimento(parser *p){

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    char nivel = 'L';
    if(strcmp(p->t.simbolo, "sidentificador") == 0){
        if (pesquisa_declproc_tabela(p->t.lexema) == 0 ){
            insere_tabela(p->t.lexema,"procedimento",nivel,rotulo);  //{guarda tabela de simb}
            //Gera("rotulo","NULL","","");
            instrucao("label", convert_integer_to_string(rotulo), ""); // CALL ira buscar este rotulo na tabsimb

            rotulo++;

            p->t = lexer(p->file, p->out);
            if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
                p->t = analisa_bloco(p);
            }else{
                printf("ERRO: linha %d, token: %s", p->t.linha, p->t.lexema);
                exit(1);
            }
        }else{
            printf("ERRO semantico: linha %d, token: %s", p->t.linha, p->t.lexema);
            exit(1);
        }
    }else{
        printf("ERRO: linha %d, token: %s", p->t.linha, p->t.lexema);
        exit(1);
    }
    desempilha_ou_voltanivel();
    return p->t;
}

token analisa_declaracao_funcao(parser *p){

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    char nivel = 'L';

    if(strcmp(p->t.simbolo, "sidentificador") == 0){
        if(pesquisa_declfunc_tabela(p->t.lexema) == 0){
            insere_tabela(p->t.lexema,"",nivel,rotulo);
            rotulo++;
            p->t = lexer(p->file, p->out);
            if(strcmp(p->t.simbolo, "sdoispontos") == 0){
                p->t = lexer(p->file, p->out);
                if((strcmp(p->t.simbolo, "sinteiro") == 0) || strcmp(p->t.simbolo, "sbooleano") == 0){
                    if(strcmp(p->t.simbolo, "sinteiro") == 0){ 
                        strcpy((sp_parser)->tipo, "funcao inteiro");
                    }else{
                        strcpy((sp_parser)->tipo, "funcao booleano");
                    }
                    p->t = lexer(p->file, p->out);
                    if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
                        p->t = analisa_bloco(p);
                    }
                } else {
                    printf("ERRO: linha %d, token: %s", p->t.linha, p->t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO: linha %d, token: %s", p->t.linha, p->t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO semantico: linha %d, token: %s", p->t.linha, p->t.lexema);
            exit(1);
        }
    }else{
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }

    desempilha_ou_voltanivel();
    return p->t;
}

token analisa_subrotinas(parser *p){

    int auxrot,flag;

    flag = 0;
    if((strcmp(p->t.simbolo, "sprocedimento") == 0) || (strcmp(p->t.simbolo, "sfuncao") == 0)){
        auxrot = rotulo;
        //Gera("","JMP","rotulo","");
        instrucao("jmp", convert_integer_to_string(rotulo), ""); //{salta sub-rotinas}

        rotulo++;
        flag = 1;
    }

    while((strcmp(p->t.simbolo, "sprocedimento") == 0) || (strcmp(p->t.simbolo, "sfuncao") == 0)) {
        if(strcmp(p->t.simbolo, "sprocedimento") == 0) {
            p->t = analisa_declaracao_procedimento(p);
        } else {
            p->t = analisa_declaracao_funcao(p);
        }
        if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
            token_free(&p->t);
            p->t = lexer(p->file, p->out);
        }else{
            printf("ERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
            exit(1);
        }
    }

    if (flag == 1){
        //Gera("auxrot","NULL","","");    
        instrucao("label", convert_integer_to_string(auxrot), ""); // {inicio do principal}
    }

    return p->t;
}

token analisa_comandos_simples(parser *p) {

    if (strcmp(p->t.simbolo, "sidentificador") == 0) {
        p->t = analisa_atrib_chprocedimento(p);
    } else if (strcmp(p->t.simbolo, "sse") == 0) {
        p->t = analisa_se(p);
    } else if (strcmp(p->t.simbolo, "senquanto") == 0) {
        p->t = analisa_enquanto(p);
    } else if (strcmp(p->t.simbolo, "sleia") == 0) {
        p->t = analisa_leia(p);
    } else if (strcmp(p->t.simbolo, "sescreva") == 0) {
        p->t = analisa_escreva(p);
    } else {
        p->t = analisa_comandos(p);
    }
    return p->t;
}

token analisa_atrib_chprocedimento(parser *p) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    if (strcmp(p->t.simbolo, "satribuicao") == 0) {
        p->t = analisa_atribuicao(p);
    } else {
        p->t = analisa_chamada_procedimento(p);
    }
    return p->t;
}



token analisa_se(parser *p) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    token in_fixa[100];
    token *pos_fixa_vetor;
    int pos = 0;  // numero de elementos do vetor in_fixa ao termino da expressao
    int posf = 0; // numero de elementos do vetor pos_fixa ao termino da expressao

    p->t = analisa_expressao(p, in_fixa, &pos);
    
    pos_fixa_vetor = pos_fixa(in_fixa, pos, &posf);
    // print_in_and_pos_fixa(in_fixa, pos, 0);
    // print_in_and_pos_fixa(pos_fixa_vetor, posf, 1);

    for (int i = 0; i < pos; i++)
        token_free(&in_fixa[i]);

    for (int i = 0; i < posf; i++)
        token_free(&pos_fixa_vetor[i]);

    free(pos_fixa_vetor);

    if (strcmp(p->t.simbolo, "sentao") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p);
        if (strcmp(p->t.simbolo, "ssenao") == 0) {
            token_free(&p->t);
            p->t = lexer(p->file, p->out);
            p->t = analisa_comandos_simples(p);
        }
    } else {
        printf("\nERRO entao: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }

    return p->t;
}


token analisa_enquanto(parser *p) {

    int auxrot1,auxrot2;

    auxrot1 = rotulo;
    //Gera(rotulo,"NULL","","");    
    instrucao("label", convert_integer_to_string(rotulo), ""); // {inicio do while}
    rotulo++;

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    token in_fixa[100];
    token *pos_fixa_vetor;
    int pos = 0;  // numero de elementos do vetor in_fixa ao termino da expressao
    int posf = 0; // numero de elementos do vetor pos_fixa ao termino da expressao

    p->t = analisa_expressao(p, in_fixa, &pos);
    
    pos_fixa_vetor = pos_fixa(in_fixa, pos, &posf);
    // print_in_and_pos_fixa(in_fixa, pos, 0);
    // print_in_and_pos_fixa(pos_fixa_vetor, posf, 1);

    for (int i = 0; i < pos; i++)
        token_free(&in_fixa[i]);

    for (int i = 0; i < posf; i++)
        token_free(&pos_fixa_vetor[i]);

    free(pos_fixa_vetor);

    if (strcmp(p->t.simbolo, "sfaca") == 0) {
        auxrot2 = rotulo;
        //Gera("","JMPF",convert_integer_to_string(rotulo),""); 
        instrucao("jmpf", convert_integer_to_string(rotulo), "");// {salta se falso}
        rotulo++;

        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p);

        //Gera("","JMP",auxrot1,""); 
        instrucao("jmp", convert_integer_to_string(auxrot1), ""); // {retorna inicio do loop}
        //Gera(auxrot2,"NULL","","");       
        instrucao("label", convert_integer_to_string(auxrot2), ""); //{fim do while}

    } else {
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }
    return p->t;
}


token analisa_leia(parser *p) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    if (strcmp(p->t.simbolo, "sabre_parenteses") == 0) {

        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        if (strcmp(p->t.simbolo, "sidentificador") == 0) {
            if (pesquisa_declvar_tabela(p->t.lexema) == 0) {

                char operando_1[100];
                strcpy(operando_1, p->t.lexema); // GERACODIGO salva antes para depois gerar o codigo(apenas se for valido)

                token_free(&p->t);
                p->t = lexer(p->file, p->out);

                if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);

                    instrucao("leia", operando_1, ""); // GERACODIGO cria a instrução se realmente for um operando da funcao leia

                } else {
                    printf("\nERRO: linha %d, token: %s", p->t.linha, p->t.lexema);
                    exit(1);
                }
            } else {
                printf("\nERRO semantico: linha %d, token: %s", p->t.linha, p->t.lexema);
                exit(1);
            }
        } else {
            printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
            exit(1);
        }
    } else {
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }

    return p->t;
}

token analisa_escreva(parser *p) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    if (strcmp(p->t.simbolo, "sabre_parenteses") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        if (strcmp(p->t.simbolo, "sidentificador") == 0) {
            if (pesquisa_declvarfunc_tabela(p->t.lexema) == 0) {

                char operando_1[100];
                strcpy(operando_1, p->t.lexema); // GERACODIGO salva antes para depois gerar o codigo(apenas se for valido)

                token_free(&p->t);
                p->t = lexer(p->file, p->out);

                if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);

                    instrucao("escreva", operando_1, NULL); // GERACODIGO cria a instrução se realmente for um operando da funcao leia

                } else {
                    printf("\nERRO: token sfecha_parenteses esperado\nLinha %d, Token: %s",
                           p->t.linha, p->t.lexema);
                    exit(1);
                }
            } else {
                printf("\nERRO semantico: Linha %d, token: %s", p->t.linha, p->t.lexema);
                exit(1);
            }
        } else {
            printf("\nERRO: token sidentificador esperado\nLinha %d, token: %s",
                   p->t.linha, p->t.lexema);
            exit(1);
        }
    } else {
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }

    return p->t;
}

token analisa_atribuicao(parser *p) {

    if (strcmp(p->t.simbolo, "satribuicao") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        token in_fixa[100];
        token *pos_fixa_vetor;
        int pos = 0;  // numero de elementos do vetor in_fixa ao termino da expressao
        int posf = 0; // numero de elementos do vetor pos_fixa ao termino da expressao

        p->t = analisa_expressao(p, in_fixa, &pos);
        
        pos_fixa_vetor = pos_fixa(in_fixa, pos, &posf);
        // print_in_and_pos_fixa(in_fixa, pos, 0);
        // print_in_and_pos_fixa(pos_fixa_vetor, posf, 1);

        for (int i = 0; i < pos; i++)
            token_free(&in_fixa[i]);

        for (int i = 0; i < posf; i++)
            token_free(&pos_fixa_vetor[i]);
        
        free(pos_fixa_vetor);

    } else {
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }
    return p->t;
}


token analisa_comandos(parser *p) {

    if (strcmp(p->t.simbolo, "sinicio") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p);

        while (strcmp(p->t.simbolo, "sfim") != 0) {
            if (strcmp(p->t.simbolo, "sponto_virgula") == 0) {
                token_free(&p->t);
                p->t = lexer(p->file, p->out);
                if (strcmp(p->t.simbolo, "sfim") != 0) {
                    p->t = analisa_comandos_simples(p);
                }
            } else {
                printf("\nERRO: token sponto_virgula esperado\nLinha %d, Token: %s\n",
                       p->t.linha, p->t.lexema);
                exit(1);
            }
        }

        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    } else {
        printf("\nERRO: token sinicio esperado\nLinha %d, Token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }
    return p->t;
}


token analisa_expressao(parser *p, token *in_fixa, int *pos) {

    p->t = analisa_expressao_simples(p, in_fixa, pos);

    if (strcmp(p->t.simbolo, "smaior") == 0 || strcmp(p->t.simbolo, "smaiorig") == 0 ||
        strcmp(p->t.simbolo, "smenor") == 0 || strcmp(p->t.simbolo, "smenorig") == 0 ||
        strcmp(p->t.simbolo, "sdif") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_expressao_simples(p, in_fixa, pos);
    }

    return p->t;
}


token analisa_expressao_simples(parser *p, token *in_fixa, int *pos) {

    if (strcmp(p->t.simbolo, "smais") == 0 || strcmp(p->t.simbolo, "smenos") == 0) {
        p->t.unario = 1; // marca como operador unário
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    }

    p->t = analisa_termo(p, in_fixa, pos);

    while (strcmp(p->t.simbolo, "smais") == 0 ||
           strcmp(p->t.simbolo, "smenos") == 0 ||
           strcmp(p->t.simbolo, "sou") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_termo(p, in_fixa, pos);
    }

    return p->t;
}

token analisa_termo(parser *p, token *in_fixa, int *pos) {

    p->t = analisa_fator(p, in_fixa, pos);

    while (strcmp(p->t.simbolo, "smult") == 0 ||
           strcmp(p->t.simbolo, "sdiv") == 0 ||
           strcmp(p->t.simbolo, "se") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        p->t = analisa_fator(p, in_fixa, pos);
    }

    return p->t;
}


token analisa_fator(parser *p, token *in_fixa, int *pos) {

    if (strcmp(p->t.simbolo, "sidentificador") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        p->t = analisa_chamada_funcao(p);
     
    
        if (pesquisa_tabela(p->t.lexema) == 1){ // semantico, verifica se é true e atribui o valor de ind no endereço passado para a função  -           IMPLEMENTAR

            strcpy((sp_parser)->tipo, "funcao booleano");

            if (strcmp((sp_parser)->tipo, "funcao inteiro") == 0 || strcmp((sp_parser)->tipo, "funcao booleano") == 0){
                p->t = analisa_chamada_funcao(p);
            }else{
                p->t = lexer(p->file, p->out);
            }
        }else{
            printf("\nERRO semantico:       Linha %d, Token: %s", p->t.linha, p->t.lexema);
            exit(1);
        }
     } else if (strcmp(p->t.simbolo, "snumero") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    } else if (strcmp(p->t.simbolo, "snao") == 0) {
        p->t.unario = 1; // marca como operador unário
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_fator(p, in_fixa, pos);
    } else if (strcmp(p->t.simbolo, "sabre_parenteses") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        p->t = analisa_expressao(p, in_fixa, pos);
        if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
            atualiza_in_fixa(in_fixa, pos, p->t);
            token_free(&p->t);
            p->t = lexer(p->file, p->out);
        } else {
            printf("\nERRO: esperado fecha parênteses\nLinha %d, Token: %s\n",
                   p->t.linha, p->t.lexema);
            exit(1);
        }
    } else if (strcmp(p->t.lexema, "verdadeiro") == 0 ||
               strcmp(p->t.lexema, "falso") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    } else {
        printf("\nERRO: fator inválido\nLinha %d, Token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }

    return p->t;
}

token analisa_chamada_funcao(parser *p) {
    if (strcmp(p->t.simbolo, "sidentificador") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    }
    return p->t;
}


token analisa_chamada_procedimento(parser *p) {
    if (strcmp(p->t.simbolo, "sidentificador") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    }
    return p->t;
}

token analisa_bloco(parser *p) {
    p->t = lexer(p->file, p->out);

    int counter_var = 0;
    p->t = analisa_et_variaveis(p,&counter_var);

    if(counter_var > 0){
        char *aux = convert_integer_to_string(counter_var);
        instrucao("var","",aux);                             //aloca as variaveis conforme a quantidade
        free(aux);
    }
    
    p->t = analisa_subrotinas(p);
    p->t = analisa_comandos(p);

    if (counter_var > 0){
        char *aux = convert_integer_to_string(counter_var);
        instrucao("var_dalloc","",aux);                         //terminando o bloco desaloca as variaveis
        free(aux);
    }
    
    return p->t;
}

int main(){
      // lista de struct de para simbolos da tabela
     //ponteiro para o topo da pilha da tabela de simbolos (= 0 porque inicia o ponteiro em 0 da pilha)
    //somente para ilustrar, depois troca pelo certo
      //repare que tem um tratamento de ponteiro com Tabsimb e pc, analisar antes de alterações
      //int rotulo = 1; *samuel que comentou essa parte, só comentei tambem

    sp_parser = initialize_stack();
    parser p; // Nova struct para o parser (melhor organizacao e clareza do codigo)
    new_program_code();

    char file_name[100];
    printf("Digite o nome do arquivo a ser analisado: ");
    scanf("%s", file_name);

    // --- Abertura do arquivo fonte ---
    p.file = fopen(file_name, "r");
    if (p.file) {
        printf("Arquivo aberto com sucesso!\n");
    } else {
        fprintf(stderr, "Erro ao abrir arquivo!\n");
        exit(1);
    }

    lexer_init(p.file);

    // --- Criação do arquivo de saída ---
    p.out = fopen("output/tokens/tabela_tokens.txt", "w");
    if (!p.out) {
        printf("Erro ao criar arquivo da tabela de tokens!\n");
        fclose(p.file);
        exit(1);
    } else {
        printf("Arquivo tabela_tokens criado com sucesso!\n");
    }

    fprintf(p.out, "Tabela de Tokens:\n");
    fprintf(p.out, "%-21s | %-20s\n", "Lexema", "Símbolo");
    fprintf(p.out, "----------------------+-----------------------\n");

    // --- Início da análise ---

    // "def" rotulo integer -> essa parte a gente declarou no semantico
    //      declarado como variavel global

    instrucao("inicia_prog","","");

    p.t = lexer(p.file, p.out);
    if (strcmp(p.t.simbolo, "sprograma") == 0) {
        token_free(&p.t);
        p.t = lexer(p.file, p.out);

        if (strcmp(p.t.simbolo, "sidentificador") == 0) {
            insere_tabela(p.t.lexema, "nomedeprograma", ' ', 0);
            token_free(&p.t);
            p.t = lexer(p.file, p.out);

            if (strcmp(p.t.simbolo, "sponto_virgula") == 0) {
                token_free(&p.t);
                p.t = analisa_bloco(&p);

                if (strcmp(p.t.simbolo, "sponto") == 0) {
                    token_free(&p.t);
                    p.t = lexer(p.file, p.out);
                    char ch = fgetc(p.file);
                    if (ch == EOF) {
                        imprimir_tabela_simbolos();
                        printf("\nSucesso\n");
                    } else {
                        printf("\nERRO: linha %d, token: %s\n", p.t.linha, p.t.lexema);
                        exit(1);
                    }
                    token_free(&p.t);
                } else {
                    printf("\nERRO: linha %d, token: %s\n", p.t.linha, p.t.lexema);
                    exit(1);
                }
            } else {
                printf("\nERRO: linha %d, token: %s\n", p.t.linha, p.t.lexema);
                exit(1);
            }
        } else {
            printf("\nERRO: linha %d, token: %s\n", p.t.linha, p.t.lexema);
            exit(1);
        }
    } else {
        printf("\nERRO: linha %d, token: %s\n", p.t.linha, p.t.lexema);
        exit(1);
    }

    fclose(p.file);
    fclose(p.out);

    instrucao("finaliza_prog","","");
    return 0;
}
// DETALHE A FUNÇÃO : "ANALISA_FATOR" TINHA UM TRATAMENTO DIFERENTE PARA PESQUISA_TABELA, ACHO QUE NAO TINHA NECESSIDADE ENTAO FIZ DIFERENTE,
// MAS SERIA BOM CONFERIR