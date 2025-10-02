#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lexical/lexer.h"
#include "../lexical/token.h"

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
void analisa_subrotinas();

void analisa_comandos(FILE* file, FILE* out, token t){

    if(strcmp(t.simbolo, "sinicio") == 0){
        t = lexer(file, out);
        analisa_comandos_simples(file, out, t);
        while (strcmp(t.simbolo, "sfim") != 0){
            if (strcmp(t.simbolo, "sponto_virgula") == 0){
                t = lexer(file, out);
                analisa_comandos_simples(file, out, t);
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
}

void_analisa_comandos_simples(FILE* file, FILE* out, token t){

    if(strcmp(t.simbolo, "sidentificador") == 0){
        analisa_atrib_chprocedimento(file, out, t);
    }else if (strcmp(t.simbolo, "sse") == 0){
        analisa_se(file, out, t);
    } else if (strcmp(t.simbolo, "senquanto") == 0){
        analisa_enquanto(file, out, t);
    } else if (strcmp(t.simbolo, "sleia") == 0){
        analisa_leia(file, out, t);
    } else if (strcmp(t.simbolo, "sescreva") == 0){
        analisa_escreva(file, out, t);
    } else {
        analisa_comandos(file, out, t);
    }


}

void analisa_atrib_chprocedimento(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "satribuicao") == 0){
        analisa_atribuicao(file, out, t);
    } else{
        chamada_procedimento(file, out, t);
    }
}


void analisa_se(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    analisa_expressao(file, out, t);
    if (strcmp(t.simbolo, "sentao") == 0){
        t = lexer(file, out);
        analisa_comandos_simples(file, out, t);
        if (strcmp(t.simbolo, "ssenao") == 0){
            t = lexer(file, out);
            analisa_comandos_simples(file, out, t);
        }
    } else {
        printf("ERRO");
        exit(1);
    }

}

void analisa_enquanto(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    analisa_expressao(file, out, t);
    if (strcmp(t.simbolo, "sfaca") == 0){
        t = lexer(file, out);
        analisa_comandos_simples(file, out, t);
    } else {
        printf("ERRO");
        exit(1);
    }
}

void analisa_leia(FILE* file, FILE* out, token t){
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
}

void analisa_escreva(FILE* file, FILE* out, token t){
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
}

void analisa_atribuicao(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sdoispontos") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sigual") == 0){
            t = lexer(file, out);
            analisa_expressao(file, out, t);
        }else {
            printf("ERRO");
            exit(1);
        }
    } else {
        printf("ERRO");
        exit(1);
    }

};

void chamada_procedimento();  // implementar

void analisa_expressao(FILE* file, FILE* out, token t){
    analisa_expressao_simples(file, out, t);
    while (strcmp(t.simbolo, "smaior") == 0 || strcmp(t.simbolo, "smaiorrig") == 0 ||
           strcmp(t.simbolo, "smenor") == 0 || strcmp(t.simbolo, "smenorrig") == 0 ||
           strcmp(t.simbolo, "sdif") == 0){
        t = lexer(file, out);
        analisa_expressao_simples(file, out, t);
    }
}

void analisa_expressao_simples(FILE* file, FILE* out, token t){
    if (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0){
        t = lexer(file, out);
    }
    analisa_termo(file, out, t);
    while (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0 || strcmp(t.simbolo, "sou") == 0){
        t = lexer(file, out);
        analisa_termo(file, out, t);
    }
}

void analisa_termo(FILE* file, FILE* out, token t); // implementar


void analisa_bloco(FILE* file, FILE* out){
    token t = lexer(file, out);
    analisa_et_variaveis(file, out, t);
    analisa_subrotinas();
    analisa_comandos(file, out, t);
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