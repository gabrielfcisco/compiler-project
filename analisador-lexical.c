#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char ch;
FILE* file;
typedef struct token {
    char* lexema;
    char simbolo[20];
} token;

token trata_digito(){
    char buffer[256];
    int i = 0;

    buffer[i++] = ch;

    ch = fgetc(file);
    while (isdigit(ch)) {
        buffer[i++] = ch;
        ch = fgetc(file);
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

token trata_identificador(){
    char buffer[256];
    int i = 0;

    buffer[i++] = ch;

    ch = fgetc(file);

    while(isdigit(ch) || (ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122) || ch == '_'){
        buffer[i++] = ch;
        ch = fgetc(file);
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

token trata_aritmetico(){

    token t;

    if(ch == '+'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "+\0");
        }
        strcpy(t.simbolo, "smais");
    } else if(ch == '-'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "-\0");
        }
        strcpy(t.simbolo, "smenos");
    } else if (ch == '*'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "*\0");
        }
        strcpy(t.simbolo, "smult");
    }

    ch = fgetc(file);

    return t;
}

token trata_relacional(){
    token t;

    if(ch == '='){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "=\0");
        }
        strcpy(t.simbolo, "sig");
        ch = fgetc(file);
        return t;
    }

    char operator = ch;
    ch = fgetc(file);

    if(operator == '!' && ch == '='){
        t.lexema = malloc(3);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "!=\0");
        }
        strcpy(t.simbolo, "sdif");
        ch = fgetc(file);
    } else if(operator == '<' && ch == '='){
        t.lexema = malloc(3);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "<=\0");
        }
        strcpy(t.simbolo, "smenorig");
        ch = fgetc(file);
    } else if(operator == '<'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "<\0");
        }
        strcpy(t.simbolo, "smenor");
    } else if(operator == '>' && ch == '='){
        t.lexema = malloc(3);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ">=\0");
        }
        strcpy(t.simbolo, "smaiorig");
        ch = fgetc(file);
    } else if(operator == '>'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ">\0");
        }
        strcpy(t.simbolo, "smaior");
    } else {
        t.lexema = malloc(13);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "Erro lexical\0");
        }
        strcpy(t.simbolo, "serro");
        return t;
    }

    return t;

}

token trata_atribuicao(){
    ch = fgetc(file);

    token t;

    if(ch == '='){
        t.lexema = malloc(3);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ":=\0");
        }
        strcpy(t.simbolo, "satribuicao");
        ch = fgetc(file);
    } else {
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ":\0");
        }
        strcpy(t.simbolo, "sdoispontos");
    }

    return t;
}

token trata_pontuacao(){
    token t;

    if(ch == '.'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ".\0");
        }
        strcpy(t.simbolo, "sponto");
    } else if(ch == ';'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ";\0");
        }
        strcpy(t.simbolo, "sponto_virgula");
    } else if(ch == ','){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ",\0");
        }
        strcpy(t.simbolo, "svirgula");
    } else if(ch == '('){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "(\0");
        }
        strcpy(t.simbolo, "sabre_parenteses");
    } else if(ch == ')'){
        t.lexema = malloc(2);
        if (t.lexema != NULL) {
            strcpy(t.lexema, ")\0");
        }
        strcpy(t.simbolo, "sfecha_parenteses");
    }

    ch = fgetc(file);

    return t;
}

token pega_token(){
    if (isdigit(ch)){
        return trata_digito();
    } else if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122)){
        return trata_identificador();
    } else if (ch == ':'){
        return trata_atribuicao();
    } else if (ch == '+' || ch == '-' || ch == '*'){
        return trata_aritmetico();
    } else if (ch == '!' || ch == '<' || ch == '>' || ch == '='){
        return trata_relacional();
    } else if (ch == '.' || ch == ';' || ch == ',' || ch == '(' || ch == ')'){
        return trata_pontuacao();
    } else {
        token t;
        t.lexema = malloc(13);
        if (t.lexema != NULL) {
            strcpy(t.lexema, "Erro lexical\0");
        }
        strcpy(t.simbolo, "serro");
        ch = fgetc(file);
        return t;
    }
}

void salva_tabela_simbolos(FILE* out, token t) {
    if (!out) {
        return;
    }
    fprintf(out, "%-20s | %-20s\n", t.lexema, t.simbolo);
}

int main() {
    char file_name[100];

    printf("Digite o nome do arquivo a ser analisado: ");
    scanf("%s", file_name);

    file = fopen(file_name, "r");

    if (file !=0 ){
        printf("File opening successful !\n");
    } else {
        fprintf(stderr, "%s\n", "File opening unsuccessful !");
    }

    ch = fgetc(file);

    FILE* out = fopen("tabela_tokens.txt", "w");
    if (!out) {
        printf("Erro ao criar arquivo da tabela de símbolos!\n");
    }
    fprintf(out, "Tabela de Tokens:\n");
    fprintf(out, "%-20s | %-20s\n","Lexema", "Simbolo");
    fprintf(out, "--------------------+-----------------------\n");

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
            token t = pega_token();
            salva_tabela_simbolos(out, t);
            if (t.lexema) free(t.lexema);
        }
    }

    fclose(file);
    fclose(out);

    return 0;
}