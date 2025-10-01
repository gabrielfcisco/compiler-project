#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lexical/lexer.h"
#include "../lexical/token.h"

token analisa_tipo(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sinteiro") != 0 && strcmp(t.simbolo, "sbooleano") != 0){
        print("\n%s", t.simbolo);
        printf("ERRO");
        exit(1);
    }
    return lexer(file, out);
}

token analisa_variaveis(FILE* file, FILE* out, token t){
    while(strcmp(t.simbolo, "sdoispontos") != 0){
        if(strcmp(t.simbolo, "sidentificador") == 0){
            t = lexer(file, out);
            if(strcmp(t.simbolo, "svirgula") == 0 || strcmp(t.simbolo, "sdoispontos") == 0){
                if(strcmp(t.simbolo, "svirgula") == 0){
                    t = lexer(file, out);
                    if(strcmp(t.simbolo, "sdoispontos") == 0){
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
        }else{
            printf("ERRO");
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
            t = analisa_variaveis(file, out, t);
            if(strcmp(t.simbolo, "sponto_virgula") == 0){
                return lexer(file, out);
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
void analisa_comandos();

token analisa_bloco(FILE* file, FILE* out){
    token t = lexer(file, out);
    return analisa_et_variaveis(file, out, t);
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
        }else{
            printf("ERRO");
            exit(1);
        }
    }else{
        printf("ERRO");
        exit(1);
    }

    return 0;
}