#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lexical/lexer.h"
#include "../lexical/token.h"

token analisa_bloco(FILE* file, FILE* out);
token analisa_comandos_simples(FILE* file, FILE* out, token t);
token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t);
token analisa_atribuicao(FILE* file, FILE* out, token t);
token analisa_escreva(FILE* file, FILE* out, token t);
token analisa_leia(FILE* file, FILE* out, token t);
token analisa_enquanto(FILE* file, FILE* out, token t);
token analisa_se(FILE* file, FILE* out, token t);
token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t);
token analisa_expressao_simples(FILE* file, FILE* out, token t);
token analisa_expressao(FILE* file, FILE* out, token t);
token analisa_atribuicao(FILE* file, FILE* out, token t);
token chamada_procedimento();
token analisa_termo(FILE* file, FILE* out, token t);



token analisa_tipo(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sinteiro") != 0 && strcmp(t.simbolo, "sbooleano") != 0){
        printf("\n%s", t.simbolo);
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }
    return lexer(file, out);
}

token analisa_variaveis(FILE* file, FILE* out, token t){
    while(strcmp(t.simbolo, "sdoispontos") != 0){
        if(strcmp(t.simbolo, "sidentificador") == 0){
            printf("\n%s", t.lexema);
            t = lexer(file, out);
            if(strcmp(t.simbolo, "svirgula") == 0 || strcmp(t.simbolo, "sdoispontos") == 0){
                if(strcmp(t.simbolo, "svirgula") == 0){
                    t = lexer(file, out);
                    if(strcmp(t.simbolo, "sdoispontos") == 0){
                        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                        exit(1);
                    }
                }
            }else{
                printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }

    t = lexer(file, out);
    return analisa_tipo(file, out, t);
}

token analisa_et_variaveis(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "svar") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sidentificador") == 0){
            analisa_variaveis(file, out, t);
            if(strcmp(t.simbolo, "sponto_virgula") == 0){
                t = lexer(file, out);
            } else {
                printf("ERRO");
                exit(1);
            }
        }else{
            printf("ERRO");
            exit(1);
        }
    }else{
        printf("ERRO");
        exit(1);
    }
}

token analisa_declaracao_procedimento(FILE* file, FILE* out, token t){
    t = lexer(file, out);

    if(strcmp(t.simbolo, "sidentificador") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sponto_virgula") == 0){
            t = analisa_bloco(file, out);
        }else{
            printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }else{
        printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }

    return t;
}

token analisa_declaracao_funcao(FILE* file, FILE* out, token t){
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
                printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
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
        printf("\n%s", t.lexema);
        if(strcmp(t.simbolo, "sponto_virgula") == 0){
            t = lexer(file, out);
        }else{
            printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }

    return t;
}

token analisa_comandos_simples(FILE* file, FILE* out, token t){

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
    t = lexer(file, out);
    if (strcmp(t.simbolo, "satribuicao") == 0){
        t = analisa_atribuicao(file, out, t);
    } else{
        t = chamada_procedimento(file, out, t);
    }
    return t;
}


token analisa_se(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    analisa_expressao(file, out, t);
    if (strcmp(t.simbolo, "sentao") == 0){
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
        if (strcmp(t.simbolo, "ssenao") == 0){
            t = lexer(file, out);
            t = analisa_comandos_simples(file, out, t);
        }
    } else {
        printf("ERRO");
        exit(1);
    }
    return t;
}

token analisa_enquanto(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    t = analisa_expressao(file, out, t);
    if (strcmp(t.simbolo, "sfaca") == 0){
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
    } else {
        printf("ERRO");
        exit(1);
    }
    return t;
}

token analisa_leia(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sabre_parenteses") == 0){
        t = lexer(file, out);
        if (strcmp(t.simbolo, "s_identificador") == 0){
            t = lexer(file, out);
            if (strcmp(t.simbolo, "sfecha_parenteses") == 0){
                t = lexer(file, out);
            }else {
                printf("ERRO");
                exit(1);
            }
        }else {
            printf("ERRO");
            exit(1);
        }
    }else {
        printf("ERRO");
        exit(1);
    }
    return t;
}

token analisa_escreva(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sabre_parenteses") == 0){
        t = lexer(file, out);
        if (strcmp(t.simbolo, "s_identificador") == 0){
            t = lexer(file, out);
            if (strcmp(t.simbolo, "sfecha_parenteses") == 0){
                t = lexer(file, out);
            }else {
                printf("ERRO");
                exit(1);
            }
        }else {
            printf("ERRO");
            exit(1);
        }
    }else {
        printf("ERRO");
        exit(1);
    }
    return t;
}

token analisa_atribuicao(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sdoispontos") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sigual") == 0){
            t = lexer(file, out);
            t = analisa_expressao(file, out, t);
        }else {
            printf("ERRO");
            exit(1);
        }
    } else {
        printf("ERRO");
        exit(1);
    }
    return t;
}

token chamada_procedimento();  // implementar

token analisa_expressao(FILE* file, FILE* out, token t){
    t = analisa_expressao_simples(file, out, t);
    while (strcmp(t.simbolo, "smaior") == 0 || strcmp(t.simbolo, "smaiorrig") == 0 ||
           strcmp(t.simbolo, "smenor") == 0 || strcmp(t.simbolo, "smenorrig") == 0 ||
           strcmp(t.simbolo, "sdif") == 0){
        t = lexer(file, out);
        t = analisa_expressao_simples(file, out, t);
    }
    return t;
}

token analisa_expressao_simples(FILE* file, FILE* out, token t){
    if (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0){
        t = lexer(file, out);
    }
    t = analisa_termo(file, out, t);
    while (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0 || strcmp(t.simbolo, "sou") == 0){
        t = lexer(file, out);
        t = analisa_termo(file, out, t);
    }

    return t;
}

token analisa_termo(FILE* file, FILE* out, token t); // implementar

token analisa_comandos(FILE* file, FILE* out, token t){

    if(strcmp(t.simbolo, "sinicio") == 0){
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
        while (strcmp(t.simbolo, "sfim") != 0){
            if (strcmp(t.simbolo, "sponto_virgula") == 0){
                t = lexer(file, out);
                t = analisa_comandos_simples(file, out, t);
            } else {
                printf("ERRO");
                exit(1);
            }
        }
        if (strcmp(t.simbolo, "sfim") == 0){
            t = lexer(file, out);
        } else {
            printf("ERRO");
            exit(1);
        }

    } else {
        printf("ERRO");
        exit(1);
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
    int rotulo = 1;

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
    printf("\n%s", t.lexema);
    if(strcmp(t.simbolo, "sprograma") == 0){
        t = lexer(file, out);
        printf("\n%s", t.lexema);
        if(strcmp(t.simbolo, "sidentificador") == 0){
            t = lexer(file, out);
            printf("\n%s", t.lexema);
            if(strcmp(t.simbolo, "sponto_virgula") == 0){
                t = analisa_bloco(file, out);
                printf("\n%s", t.lexema);
                if(strcmp(t.simbolo, "sponto") == 0){
                    if(file == EOF){
                        printf("\nSucesso");
                    }else{
                        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                        exit(1);
                    }
                }else{
                    printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }else{
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }

    return 0;
}