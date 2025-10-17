#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lexical/lexer.h"
#include "../lexical/token.h"

token analisa_bloco(FILE* file, FILE* out);
token analisa_comandos_simples(FILE* file, FILE* out, token t);
token analisa_comandos(FILE* file, FILE* out, token t);
token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t);
token analisa_atribuicao(FILE* file, FILE* out, token t);
token analisa_escreva(FILE* file, FILE* out, token t);
token analisa_leia(FILE* file, FILE* out, token t);
token analisa_enquanto(FILE* file, FILE* out, token t);
token analisa_se(FILE* file, FILE* out, token t);
token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t);
token analisa_expressao_simples(FILE* file, FILE* out, token t, token *in_fixa, int *pos);
token analisa_expressao(FILE* file, FILE* out, token t, token *in_fixa, int *pos);
token analisa_termo(FILE* file, FILE* out, token t, token *in_fixa, int *pos);
token analisa_chamada_funcao(FILE* file, FILE* out, token t);
token analisa_chamada_procedimento(FILE* file, FILE* out, token t);
token analisa_fator(FILE* file, FILE* out, token t, token *in_fixa, int *pos);




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

token analisa_tipo(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sinteiro") != 0 && strcmp(t.simbolo, "sbooleano") != 0){
        // printf("\n%s", t.simbolo);
        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
        token_free(&t);
        exit(1);
    }
    token_free(&t);
    return lexer(file, out);
}

token analisa_variaveis(FILE* file, FILE* out, token t){
    while(strcmp(t.simbolo, "sdoispontos") != 0){
        if(strcmp(t.simbolo, "sidentificador") == 0){
            // printf("\n%s", t.lexema);
            token_free(&t);
            t = lexer(file, out);
            if(strcmp(t.simbolo, "svirgula") == 0 || strcmp(t.simbolo, "sdoispontos") == 0){
                if(strcmp(t.simbolo, "svirgula") == 0){
                    token_free(&t);
                    t = lexer(file, out);
                    // printf("\naqui");
                    if(strcmp(t.simbolo, "sdoispontos") == 0){
                        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
                        exit(1);
                    }
                }
            }else{
                printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }
    token_free(&t);
    t = lexer(file, out);
    return analisa_tipo(file, out, t);
}

token analisa_et_variaveis(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "svar") == 0){
        token_free(&t);
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sidentificador") == 0){
            // printf("\n%s", t.lexema);
            while(strcmp(t.simbolo, "sidentificador") == 0){
                t = analisa_variaveis(file, out, t);
                if(strcmp(t.simbolo, "sponto_virgula") == 0){
                    token_free(&t);
                    t = lexer(file, out);
                } else {
                    printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
                    exit(1);
                }
            }
        }else{
            printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }
    return t;
}

token analisa_declaracao_procedimento(FILE* file, FILE* out, token t){
    token_free(&t);
    t = lexer(file, out);

    if(strcmp(t.simbolo, "sidentificador") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sponto_virgula") == 0){
            t = analisa_bloco(file, out);
        }else{
            printf("ERRO: linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }else{
        printf("ERRO: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }

    return t;
}

token analisa_declaracao_funcao(FILE* file, FILE* out, token t){
    token_free(&t);
    t = lexer(file, out);

    if(strcmp(t.simbolo, "sidentificador") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sdoispontos") == 0){
            t = lexer(file, out);
            if((strcmp(t.simbolo, "sinteiro") == 0) || strcmp(t.simbolo, "sbooleano") == 0){
                t = lexer(file, out);
                if(strcmp(t.simbolo, "sponto_virgula") == 0){
                    t = analisa_bloco(file, out);
                }
            } else {
                printf("ERRO: linha %d, token: %s\n", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }else{
        printf("ERRO: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_subrotinas(FILE* file, FILE* out, token t){

    while((strcmp(t.simbolo, "sprocedimento") == 0) || (strcmp(t.simbolo, "sfuncao") == 0)) {
        if(strcmp(t.simbolo, "sprocedimento") == 0) {
            t = analisa_declaracao_procedimento(file, out, t);
        } else {
            t = analisa_declaracao_funcao(file, out, t);
        }
        // printf("\n%s", t.lexema);
        if(strcmp(t.simbolo, "sponto_virgula") == 0){
            token_free(&t);
            t = lexer(file, out);
        }else{
            printf("ERRO: linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }

    return t;
}

token analisa_comandos_simples(FILE* file, FILE* out, token t){
    // printf("\nanalisando comando simples");
    if(strcmp(t.simbolo, "sidentificador") == 0){
        t = analisa_atrib_chprocedimento(file, out, t);
    }else if (strcmp(t.simbolo, "sse") == 0){
        t = analisa_se(file, out, t);
    } else if (strcmp(t.simbolo, "senquanto") == 0){
        t = analisa_enquanto(file, out, t);
    } else if (strcmp(t.simbolo, "sleia") == 0){
        t = analisa_leia(file, out, t);
    } else if (strcmp(t.simbolo, "sescreva") == 0){
        t = analisa_escreva(file, out, t);
    } else {
        t = analisa_comandos(file, out, t);
    }

    return t;
}

token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t){
    token_free(&t);
    t = lexer(file, out);
    if (strcmp(t.simbolo, "satribuicao") == 0){
        // imprimir_token(t);
        t = analisa_atribuicao(file, out, t);
    } else{
        t = analisa_chamada_procedimento(file, out, t);
    }
    return t;
}


token analisa_se(FILE* file, FILE* out, token t){
    token_free(&t);
    t = lexer(file, out);
    token in_fixa[100]; //vetor que armazena a expressao em notacao in_fixa
    int pos = 0;    // necessario para a pos do vetor in_fixa
    t = analisa_expressao(file, out, t, in_fixa, &pos);

    print_in_fixa(in_fixa, pos); // Imprime a expressão em notação infixa

    for (int i = 0; i < pos; i++) { // libera a memória usada pelo vetor 'in_fixa'
        token_free(&in_fixa[i]);
    }

    if (strcmp(t.simbolo, "sentao") == 0){
        token_free(&t);
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
        if (strcmp(t.simbolo, "ssenao") == 0){
            token_free(&t);
            t = lexer(file, out);
            t = analisa_comandos_simples(file, out, t);
        }
    } else {
        printf("\nERRO entao: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_enquanto(FILE* file, FILE* out, token t){
    token_free(&t);
    t = lexer(file, out);
    token in_fixa[100]; //vetor que armazena a expressao em notacao in_fixa
    int pos = 0;    // necessario para a pos do vetor in_fixa
    t = analisa_expressao(file, out, t, in_fixa, &pos);

    print_in_fixa(in_fixa, pos); // Imprime a expressão em notação infixa

    for (int i = 0; i < pos; i++) { // libera a memória usada pelo vetor 'in_fixa'
        token_free(&in_fixa[i]);
    }
    

    if (strcmp(t.simbolo, "sfaca") == 0){
        token_free(&t);
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
    } else {
        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_leia(FILE* file, FILE* out, token t){
    token_free(&t);
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sabre_parenteses") == 0){
        token_free(&t);
        t = lexer(file, out);
        if (strcmp(t.simbolo, "sidentificador") == 0){
            token_free(&t);
            t = lexer(file, out);
            if (strcmp(t.simbolo, "sfecha_parenteses") == 0){
                token_free(&t);
                t = lexer(file, out);
            }else {
                printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
                exit(1);
            }
        }else {
            printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }else {
        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_escreva(FILE* file, FILE* out, token t){
    token_free(&t);
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sabre_parenteses") == 0){
        token_free(&t);
        t = lexer(file, out);
        if (strcmp(t.simbolo, "sidentificador") == 0){
            token_free(&t);
            t = lexer(file, out);
            if (strcmp(t.simbolo, "sfecha_parenteses") == 0){
                token_free(&t);
                t = lexer(file, out);
            }else {
                printf("\nERRO: token sfecha_parenteses esperado\nLinha %d, Token: %s\n", t.linha, t.lexema);
                exit(1);
            }
        }else {
            printf("\nERRO: token sidentificador esperado\n Linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }else {
        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_atribuicao(FILE* file, FILE* out, token t){
    if (strcmp(t.simbolo, "satribuicao") == 0) {
        token_free(&t);
        t = lexer(file, out);
        token in_fixa[100]; //vetor que armazena a expressao em notacao in_fixa
        int pos = 0;    // necessario para a pos do vetor in_fixa
        t = analisa_expressao(file, out, t, in_fixa, &pos);

        print_in_fixa(in_fixa, pos); // Imprime a expressão em notação infixa

        for (int i = 0; i < pos; i++) { // libera a memória usada pelo vetor 'in_fixa'
        token_free(&in_fixa[i]);
        }

    } else {
        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_comandos(FILE* file, FILE* out, token t){

    if(strcmp(t.simbolo, "sinicio") == 0){
        token_free(&t);
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
        while (strcmp(t.simbolo, "sfim") != 0){
            if (strcmp(t.simbolo, "sponto_virgula") == 0){
                token_free(&t);
                t = lexer(file, out);
                if(strcmp(t.simbolo, "sfim") != 0){
                    t = analisa_comandos_simples(file, out, t);
                }
            } else {
                printf("\nERRO: token sponto_virgula esperado\nLinha %d, Token: %s\n", t.linha, t.lexema);
                exit(1);
            }
        }
        token_free(&t);
        t = lexer(file, out);
    } else {
        printf("\nERRO: token sinicio esperado\nLinha %d, Token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}


token analisa_expressao(FILE* file, FILE* out, token t, token *in_fixa, int *pos){

    t = analisa_expressao_simples(file, out, t, in_fixa, pos);
    if(strcmp(t.simbolo, "smaior") == 0 || strcmp(t.simbolo, "smaiorig") == 0 ||
           strcmp(t.simbolo, "smenor") == 0 || strcmp(t.simbolo, "smenorig") == 0 ||
           strcmp(t.simbolo, "sdif") == 0){
        atualiza_in_fixa(in_fixa, pos, t); //atualiza o vetor in_fixa
        token_free(&t);
        t = lexer(file, out);
        t = analisa_expressao_simples(file, out, t, in_fixa, pos);
    }
    return t;
}

token analisa_expressao_simples(FILE* file, FILE* out, token t, token *in_fixa, int *pos){
    if (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0){              //Se o token da expressao for + ou - e o unario (maior prioridade)
        atualiza_in_fixa(in_fixa, pos, t); //atualiza o vetor in_fixa
        token_free(&t);
        t = lexer(file, out);
    }
    t = analisa_termo(file, out, t, in_fixa, pos); 
    while (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0 || strcmp(t.simbolo, "sou") == 0){
        atualiza_in_fixa(in_fixa, pos, t);
        token_free(&t);
        t = lexer(file, out);
        t = analisa_termo(file, out, t, in_fixa, pos);
    }
    
    return t;
}


token analisa_termo(FILE* file, FILE* out, token t, token *in_fixa, int *pos){
    t = analisa_fator(file, out, t, in_fixa, pos); // Primeiro analisa um fator

    while(strcmp(t.simbolo, "smult") == 0 ||
          strcmp(t.simbolo, "sdiv") == 0 ||
          strcmp(t.simbolo, "se") == 0){

        atualiza_in_fixa(in_fixa, pos, t); 
        token_free(&t);
        t = lexer(file, out);
        t = analisa_fator(file, out, t, in_fixa, pos); // Depois analisa outro fator
    }

    return t;
}

token analisa_fator(FILE* file, FILE* out, token t, token *in_fixa, int *pos){
    if(strcmp(t.simbolo, "sidentificador") == 0){
        atualiza_in_fixa(in_fixa, pos, t); 
        t = analisa_chamada_funcao(file, out, t);
    }
    else if(strcmp(t.simbolo, "snumero") == 0){
        atualiza_in_fixa(in_fixa, pos, t);
        token_free(&t);
        t = lexer(file, out);
    }
    else if(strcmp(t.simbolo, "snao") == 0){
        atualiza_in_fixa(in_fixa, pos, t);
        token_free(&t);
        t = lexer(file, out);
        t = analisa_fator(file, out, t, in_fixa, pos);
    }
    else if(strcmp(t.simbolo, "sabre_parenteses") == 0){
        atualiza_in_fixa(in_fixa, pos, t);
        token_free(&t);
        t = lexer(file, out);

        t = analisa_expressao(file, out, t, in_fixa, pos);
        if(strcmp(t.simbolo, "sfecha_parenteses") == 0){
            atualiza_in_fixa(in_fixa, pos, t);
            token_free(&t);
            t = lexer(file, out);
        } else {
            printf("\nERRO: esperado fecha parênteses\nLinha %d, Token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }
    else if(strcmp(t.lexema, "verdadeiro") == 0 ||
            strcmp(t.lexema, "falso") == 0){
            atualiza_in_fixa(in_fixa, pos, t);
            token_free(&t);
            t = lexer(file, out);
    }
    else{
        printf("\nERRO: fator inválido\nLinha %d, Token: %s\n", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_chamada_funcao(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sidentificador") == 0){
        token_free(&t);
        t = lexer(file, out);
    }
    return t;
}

token analisa_chamada_procedimento(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sidentificador") == 0){
        token_free(&t);
        t = lexer(file, out);
    }
    return t;
}


token analisa_bloco(FILE* file, FILE* out){
    token t = lexer(file, out);
    t = analisa_et_variaveis(file, out, t);
    t = analisa_subrotinas(file, out, t);
    t = analisa_comandos(file, out, t);
    return t;
}

int main(){
    // int rotulo = 1;

    FILE* file;

    char file_name[100];

    printf("Digite o nome do arquivo a ser analisado: ");
    scanf("%s", file_name);

    file = fopen(file_name, "r");

    if (file != 0 ){
        printf("Arquivo aberto com sucesso!\n");
    } else {
        fprintf(stderr, "%s\n", "Erro ao abrir arquivo!");
    }

    lexer_init(file);

    FILE* out = fopen("output/tokens/tabela_tokens.txt", "w");
    if (!out) {
        printf("Erro ao criar arquivo da tabela de tokens!\n");
    } else {
        printf("Arquivo tabela_tokens criado com sucesso!\n");
    }
    fprintf(out, "Tabela de Tokens:\n");
    fprintf(out, "%-21s | %-20s\n","Lexema", "Símbolo");
    fprintf(out, "----------------------+-----------------------\n");

    token t = lexer(file, out);
    if(strcmp(t.simbolo, "sprograma") == 0){
        token_free(&t);
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sidentificador") == 0){
            token_free(&t);
            t = lexer(file, out);
            if(strcmp(t.simbolo, "sponto_virgula") == 0){
                token_free(&t);
                t = analisa_bloco(file, out);
                if(strcmp(t.simbolo, "sponto") == 0){
                    token_free(&t);
                    t = lexer(file, out);
                    char ch = fgetc(file);
                    if(ch == EOF){
                        printf("\nSucesso");
                    }else{
                        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
                        exit(1);
                    }
                    token_free(&t); // libera o token final
                }else{
                    printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
            exit(1);
        }
    }else{
        printf("\nERRO: linha %d, token: %s\n", t.linha, t.lexema);
        exit(1);
    }

    return 0;
}