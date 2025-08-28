#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct token {
    char* lexema;
    char simbolo[20];
} token;

token trata_digito(char c, FILE* file){
    char buffer[256];
    int i = 0;

    buffer[i++] = c;

    c = fgetc(file);
    while (isdigit(c)) {
        buffer[i++] = c;
        c = fgetc(file);
    }

    buffer[i] = '\0';

    token t;
    strcpy(t.simbolo, "snumero");
    t.lexema = malloc(i + 1);
    if (t.lexema != NULL) {
        strcpy(t.lexema, buffer);
    }

    return t;
}

token trata_identificador(char c, FILE* file){
    char buffer[20];
    int i = 0;

    buffer[i++] = c;

    c = fgetc(file);

    while(isdigit(c) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == '_'){
        buffer[i++] = c;
        c = fgetc(file);
    }

    buffer[i] = '\0';

    token t;
    t.lexema = malloc(i + 1);
    if (t.lexema != NULL) {
        strcpy(t.lexema, buffer);
    }

    if(strcmp(t.lexema, "programa") == 0){
        strcpy(t.simbolo, "sprograma");
    } else if (strcmp(t.lexema, "se") == 0){
        strcpy(t.simbolo, "sse");
    } else if (strcmp(t.lexema, "entao") == 0){
        strcpy(t.simbolo, "sentao");
    } else if (strcmp(t.lexema, "senao") == 0){
        strcpy(t.simbolo, "ssenao");
    } else if (strcmp(t.lexema, "enquanto") == 0){
        strcpy(t.simbolo, "senquanto");
    } else if (strcmp(t.lexema, "faca") == 0){
        strcpy(t.simbolo, "sfaca");
    } else if (strcmp(t.lexema, "inicio") == 0){
        strcpy(t.simbolo, "sinicio");
    } else if (strcmp(t.lexema, "fim") == 0){
        strcpy(t.simbolo, "sfim");
    } else if (strcmp(t.lexema, "escreva") == 0){
        strcpy(t.simbolo, "sescreva");
    } else if (strcmp(t.lexema, "leia") == 0){
        strcpy(t.simbolo, "sleia");
    } else if (strcmp(t.lexema, "var") == 0){
        strcpy(t.simbolo, "svar");
    } else if (strcmp(t.lexema, "inteiro") == 0){
        strcpy(t.simbolo, "sinteiro");
    } else if (strcmp(t.lexema, "booleano") == 0){
        strcpy(t.simbolo, "sbooleano");
    } else if (strcmp(t.lexema, "verdadeiro") == 0){
        strcpy(t.simbolo, "sverdadeiro");
    } else if (strcmp(t.lexema, "falso") == 0){
        strcpy(t.simbolo, "sfalso");
    } else if (strcmp(t.lexema, "procedimento") == 0){
        strcpy(t.simbolo, "sprocedimento");
    } else if (strcmp(t.lexema, "funcao") == 0){
        strcpy(t.simbolo, "sfuncao");
    } else if (strcmp(t.lexema, "div") == 0){
        strcpy(t.simbolo, "sdiv");
    } else if (strcmp(t.lexema, "e") == 0){
        strcpy(t.simbolo, "se");
    } else if (strcmp(t.lexema, "ou") == 0){
        strcpy(t.simbolo, "sou");
    } else if (strcmp(t.lexema, "nao") == 0){
        strcpy(t.simbolo, "snao");
    } else {
        strcpy(t.simbolo, "sidentificador");
    }

    return t;
}

void pega_token(char c, FILE* file){
    if (isdigit(c)){
        token t = trata_digito(c, file);
        printf("\n%s", t.lexema);
        printf("\n%s", t.simbolo);
    } else if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122)){
        token t = trata_identificador(c, file);
        printf("\n%s", t.lexema);
        printf("\n%s", t.simbolo);
    } else if (c == ':'){
        printf("\nis atribution");
    } else if (c == '+' || c == '-' || c == '*'){
        printf("\nis operation");
    } else if (c == '!' || c == '<' || c == '>' || c == '='){
        printf("\nis logical");
    } else if (c == '.' || c == ';' || c == ',' || c == '(' || c == ')'){
        printf("\nis pontuation");
    } else {
        printf("\nis ERROR");
    }
}

int main() {
    char file_name[100];

    printf("Digite o nome do arquivo a ser analisado: ");
    scanf("%s", file_name);

    FILE* file = fopen(file_name, "r");

    if (file !=0 ){
        printf("File opening successful !\n");
    } else {
        fprintf(stderr, "%s\n", "File opening unsuccessful !");
    }

    char ch = fgetc(file);

    while(ch != EOF){
        while((ch == '{' || ch == ' ' || ch == '\n' || ch == '\t' || ch == '\b' || ch == 10) && ch != EOF){
            if(ch == '{'){
                while(ch != '}' && ch != EOF){
                    ch = fgetc(file);
                }
            }
            ch = fgetc(file);
            while((ch == ' ' || ch == '\n' || ch == '\t' || ch == '\b' || ch == 10) && ch != EOF){
                ch = fgetc(file);
            }
        }
        if(ch != EOF){
            // printf("\n%c\n", ch);
            pega_token(ch, file);
            ch = fgetc(file);
        }
    }

    fclose(file);

    return 0;
}