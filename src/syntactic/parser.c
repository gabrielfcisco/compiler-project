#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/parser/parser.h"
#include "../../include/lexical/lexer.h"
#include "../../include/semantic/semantic.h"

#define TAMANHO_TAB_SIMB 100 // EXEMPLO SÓ PRA NAO DAR ERRO


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

void print_in_fixa(token *in_fixa, int pos) {

    printf("Expressão em notação infixa: ");
    for (int i = 0; i < pos; i++) {
        printf("%s ", in_fixa[i].lexema);
    }
    printf("\n");
}

token analisa_tipo(parser *p) {

    if(strcmp(p->t.simbolo, "sinteiro") != 0 && strcmp(p->t.simbolo, "sbooleano") != 0){
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        token_free(&p->t);
        exit(1);
    }else{
        coloca_tipo_tabela(p->t.lexema);
    }
    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    return p->t;
}

token analisa_variaveis(parser *p){

    while(strcmp(p->t.simbolo, "sdoispontos") != 0){
        if(strcmp(p->t.simbolo, "sidentificador") == 0){
            if(pesquisa_duplicacvar_tabela(p->t.lexema) == 0){
                insere_tabela(p->t.lexema,"variavel",' ',"");
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

token analisa_et_variaveis(parser *p){

    if(strcmp(p->t.simbolo, "svar") == 0){
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        if(strcmp(p->t.simbolo, "sidentificador") == 0){
            while(strcmp(p->t.simbolo, "sidentificador") == 0){
                p->t = analisa_variaveis(p);
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


token analisa_declaracao_procedimento(parser *p, Tabsimb *tabela_simbolos, int *pc){

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    char nivel = 'L';
    if(strcmp(p->t.simbolo, "sidentificador") == 0){
        if (pesquisa_declproc_tabela(p->t.lexema) == 0 ){
            insere_tabela(p->t.lexema,"procedimento",nivel,"");
            p->t = lexer(p->file, p->out);
            if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
                p->t = analisa_bloco(p, tabela_simbolos, pc);
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

token analisa_declaracao_funcao(parser *p, Tabsimb *tabela_simbolos, int *pc){

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    char nivel = 'L';

    if(strcmp(p->t.simbolo, "sidentificador") == 0){
        if(pesquisa_declfunc_tabela(p->t.lexema) == 0){
            insere_tabela(p->t.lexema,"",nivel,"");
            p->t = lexer(p->file, p->out);
            if(strcmp(p->t.simbolo, "sdoispontos") == 0){
                p->t = lexer(p->file, p->out);
                if((strcmp(p->t.simbolo, "sinteiro") == 0) || strcmp(p->t.simbolo, "sbooleano") == 0){
                    if(strcmp(p->t.simbolo, "sinteiro") == 0){ 
                        strcpy(tabela_simbolos[*pc].tipo, "funcao inteiro");
                    }else{
                        strcpy(tabela_simbolos[*pc].tipo, "funcao booleano");
                    }
                    p->t = lexer(p->file, p->out);
                    if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
                        p->t = analisa_bloco(p, tabela_simbolos, pc);
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

token analisa_subrotinas(parser *p, Tabsimb *tabela_simbolos,int *pc){

    while((strcmp(p->t.simbolo, "sprocedimento") == 0) || (strcmp(p->t.simbolo, "sfuncao") == 0)) {
        if(strcmp(p->t.simbolo, "sprocedimento") == 0) {
            p->t = analisa_declaracao_procedimento(p, tabela_simbolos, pc);
        } else {
            p->t = analisa_declaracao_funcao(p, tabela_simbolos, pc);
        }
        if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
            token_free(&p->t);
            p->t = lexer(p->file, p->out);
        }else{
            printf("ERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
            exit(1);
        }
    }
    return p->t;
}

token analisa_comandos_simples(parser *p, Tabsimb *tabela_simbolos) {

    if (strcmp(p->t.simbolo, "sidentificador") == 0) {
        p->t = analisa_atrib_chprocedimento(p, tabela_simbolos);
    } else if (strcmp(p->t.simbolo, "sse") == 0) {
        p->t = analisa_se(p, tabela_simbolos);
    } else if (strcmp(p->t.simbolo, "senquanto") == 0) {
        p->t = analisa_enquanto(p, tabela_simbolos);
    } else if (strcmp(p->t.simbolo, "sleia") == 0) {
        p->t = analisa_leia(p);
    } else if (strcmp(p->t.simbolo, "sescreva") == 0) {
        p->t = analisa_escreva(p);
    } else {
        p->t = analisa_comandos(p, tabela_simbolos);
    }
    return p->t;
}

token analisa_atrib_chprocedimento(parser *p, Tabsimb *tabela_simbolos) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    if (strcmp(p->t.simbolo, "satribuicao") == 0) {
        p->t = analisa_atribuicao(p, tabela_simbolos);
    } else {
        p->t = analisa_chamada_procedimento(p);
    }
    return p->t;
}



token analisa_se(parser *p, Tabsimb *tabela_simbolos) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    token in_fixa[100];
    int pos = 0;

    p->t = analisa_expressao(p, in_fixa, &pos, tabela_simbolos);
    print_in_fixa(in_fixa, pos);

    for (int i = 0; i < pos; i++)
        token_free(&in_fixa[i]);

    if (strcmp(p->t.simbolo, "sentao") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p, tabela_simbolos);
        if (strcmp(p->t.simbolo, "ssenao") == 0) {
            token_free(&p->t);
            p->t = lexer(p->file, p->out);
            p->t = analisa_comandos_simples(p, tabela_simbolos);
        }
    } else {
        printf("\nERRO entao: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }

    return p->t;
}


token analisa_enquanto(parser *p, Tabsimb *tabela_simbolos) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    token in_fixa[100];
    int pos = 0;

    p->t = analisa_expressao(p, in_fixa, &pos, tabela_simbolos);
    print_in_fixa(in_fixa, pos);

    for (int i = 0; i < pos; i++)
        token_free(&in_fixa[i]);

    if (strcmp(p->t.simbolo, "sfaca") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p, tabela_simbolos);
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
                token_free(&p->t);
                p->t = lexer(p->file, p->out);

                if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);
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
                token_free(&p->t);
                p->t = lexer(p->file, p->out);

                if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);
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

token analisa_atribuicao(parser *p, Tabsimb *tabela_simbolos) {

    if (strcmp(p->t.simbolo, "satribuicao") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        token in_fixa[100];
        int pos = 0;

        p->t = analisa_expressao(p, in_fixa, &pos, tabela_simbolos);
        print_in_fixa(in_fixa, pos);

        for (int i = 0; i < pos; i++)
            token_free(&in_fixa[i]);
    } else {
        printf("\nERRO: linha %d, token: %s\n", p->t.linha, p->t.lexema);
        exit(1);
    }
    return p->t;
}


token analisa_comandos(parser *p, Tabsimb *tabela_simbolos) {

    if (strcmp(p->t.simbolo, "sinicio") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p, tabela_simbolos);

        while (strcmp(p->t.simbolo, "sfim") != 0) {
            if (strcmp(p->t.simbolo, "sponto_virgula") == 0) {
                token_free(&p->t);
                p->t = lexer(p->file, p->out);
                if (strcmp(p->t.simbolo, "sfim") != 0) {
                    p->t = analisa_comandos_simples(p, tabela_simbolos);
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


token analisa_expressao(parser *p, token *in_fixa, int *pos, Tabsimb *tabela_simbolos) {

    p->t = analisa_expressao_simples(p, in_fixa, pos, tabela_simbolos);

    if (strcmp(p->t.simbolo, "smaior") == 0 || strcmp(p->t.simbolo, "smaiorig") == 0 ||
        strcmp(p->t.simbolo, "smenor") == 0 || strcmp(p->t.simbolo, "smenorig") == 0 ||
        strcmp(p->t.simbolo, "sdif") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_expressao_simples(p, in_fixa, pos, tabela_simbolos);
    }

    return p->t;
}


token analisa_expressao_simples(parser *p, token *in_fixa, int *pos, Tabsimb *tabela_simbolos) {

    if (strcmp(p->t.simbolo, "smais") == 0 || strcmp(p->t.simbolo, "smenos") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    }

    p->t = analisa_termo(p, in_fixa, pos, tabela_simbolos);

    while (strcmp(p->t.simbolo, "smais") == 0 ||
           strcmp(p->t.simbolo, "smenos") == 0 ||
           strcmp(p->t.simbolo, "sou") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_termo(p, in_fixa, pos, tabela_simbolos);
    }

    return p->t;
}

token analisa_termo(parser *p, token *in_fixa, int *pos, Tabsimb *tabela_simbolos) {

    p->t = analisa_fator(p, in_fixa, pos, tabela_simbolos);

    while (strcmp(p->t.simbolo, "smult") == 0 ||
           strcmp(p->t.simbolo, "sdiv") == 0 ||
           strcmp(p->t.simbolo, "se") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        p->t = analisa_fator(p, in_fixa, pos, tabela_simbolos);
    }

    return p->t;
}


token analisa_fator(parser *p, token *in_fixa, int *pos, Tabsimb *tabela_simbolos) {

    if (strcmp(p->t.simbolo, "sidentificador") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        p->t = analisa_chamada_funcao(p);
        // int ind; // semantico  /COMENTANDO POR ENQUANTO
    
        // if (pesquisa_tabela(t.lexema,&ind,tabela_simbolos) == 1){ // semantico, verifica se é true e atribui o valor de ind no endereço passado para a função

        //     if (strcmp(tabela_simbolos[ind].tipo, "funcao inteiro") == 0 || strcmp(tabela_simbolos[ind].tipo, "funcao booleano") == 0){
        //         t = analisa_chamada_funcao(file, out, t);
        //     }else{
        //         t = lexer(file,out);
        //     }
        // }else{
        //     printf("\nERRO semantico:       Linha %d, Token: %s", t.linha, t.lexema);
        //     exit(1);
        // }
     } else if (strcmp(p->t.simbolo, "snumero") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    } else if (strcmp(p->t.simbolo, "snao") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_fator(p, in_fixa, pos, tabela_simbolos);
    } else if (strcmp(p->t.simbolo, "sabre_parenteses") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        p->t = analisa_expressao(p, in_fixa, pos, tabela_simbolos);
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

token analisa_bloco(parser *p, Tabsimb *tabela_simbolos, int *pc) {
    p->t = lexer(p->file, p->out);
    p->t = analisa_et_variaveis(p);
    p->t = analisa_subrotinas(p, tabela_simbolos, pc);
    p->t = analisa_comandos(p, tabela_simbolos);
    return p->t;
}


int main(){
    Tabsimb tabela_simbolos[TAMANHO_TAB_SIMB];  // lista de struct de para simbolos da tabela
    int pc = 0;                                 //ponteiro para o topo da pilha da tabela de simbolos (= 0 porque inicia o ponteiro em 0 da pilha)
                                                //somente para ilustrar, depois troca pelo certo
                                                //repare que tem um tratamento de ponteiro com Tabsimb e pc, analisar antes de alterações
                                                //int rotulo = 1; *samuel que comentou essa parte, só comentei tambem

    parser p; // Nova struct para o parser (melhor organizacao e clareza do codigo)

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
    p.t = lexer(p.file, p.out);
    if (strcmp(p.t.simbolo, "sprograma") == 0) {
        token_free(&p.t);
        p.t = lexer(p.file, p.out);

        if (strcmp(p.t.simbolo, "sidentificador") == 0) {
            insere_tabela(p.t.lexema, "nomedeprograma", ' ', "");
            token_free(&p.t);
            p.t = lexer(p.file, p.out);

            if (strcmp(p.t.simbolo, "sponto_virgula") == 0) {
                token_free(&p.t);
                p.t = analisa_bloco(&p, tabela_simbolos, &pc);

                if (strcmp(p.t.simbolo, "sponto") == 0) {
                    token_free(&p.t);
                    p.t = lexer(p.file, p.out);
                    char ch = fgetc(p.file);
                    if (ch == EOF) {
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
    return 0;
}
// DETALHE A FUNÇÃO : "ANALISA_FATOR" TINHA UM TRATAMENTO DIFERENTE PARA PESQUISA_TABELA, ACHO QUE NAO TINHA NECESSIDADE ENTAO FIZ DIFERENTE,
// MAS SERIA BOM CONFERIR