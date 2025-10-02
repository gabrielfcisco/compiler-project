#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char ch;
int line;

typedef struct token {
    char* lexema;
    char simbolo[20];
    int linha;
    int erro;  // 0 = ok, 1 = erro
} token;

token token_create(char* lexema, char* simbolo, int linha) {
    token t;
    t.lexema = malloc(strlen(lexema) + 1);
    if (t.lexema != NULL) {
        strcpy(t.lexema, lexema);
    }
    strcpy(t.simbolo, simbolo);
    t.linha = linha;
    t.erro = 0;
    return t;
}

token token_create_error(int linha) {
    token t;
    t.lexema = malloc(50);
    if (t.lexema != NULL) {
        sprintf(t.lexema, "Erro lexical: linha %d", linha);
    }
    strcpy(t.simbolo, "serro");
    t.linha = linha;
    t.erro = 1;
    return t;
}

void token_free(token* t) {
    if (t && t->lexema) {
        free(t->lexema);
        t->lexema = NULL;
    }
}

token trata_digito(FILE* file){
    char buffer[256];
    int i = 0;
    int linha_inicio = line;

    buffer[i++] = ch;

    ch = fgetc(file);
    while (isdigit(ch)) {
        buffer[i++] = ch;
        ch = fgetc(file);
    }

    buffer[i] = '\0';

    return token_create(buffer, "snumero", linha_inicio);
}

token trata_identificador(FILE* file){
    char buffer[256];
    int i = 0;
    int linha_inicio = line;

    buffer[i++] = ch;

    ch = fgetc(file);

    while(isdigit(ch) || (ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122) || ch == '_'){
        buffer[i++] = ch;
        ch = fgetc(file);
    }

    buffer[i] = '\0';

    // Determina o símbolo baseado na palavra reservada
    const char* simbolo = "sidentificador";

    if(strcmp(buffer, "programa") == 0){
        simbolo = "sprograma";
    } else if (strcmp(buffer, "se") == 0){
        simbolo = "sse";
    } else if (strcmp(buffer, "entao") == 0){
        simbolo = "sentao";
    } else if (strcmp(buffer, "senao") == 0){
        simbolo = "ssenao";
    } else if (strcmp(buffer, "enquanto") == 0){
        simbolo = "senquanto";
    } else if (strcmp(buffer, "faca") == 0){
        simbolo = "sfaca";
    } else if (strcmp(buffer, "inicio") == 0){
        simbolo = "sinicio";
    } else if (strcmp(buffer, "fim") == 0){
        simbolo = "sfim";
    } else if (strcmp(buffer, "escreva") == 0){
        simbolo = "sescreva";
    } else if (strcmp(buffer, "leia") == 0){
        simbolo = "sleia";
    } else if (strcmp(buffer, "var") == 0){
        simbolo = "svar";
    } else if (strcmp(buffer, "inteiro") == 0){
        simbolo = "sinteiro";
    } else if (strcmp(buffer, "booleano") == 0){
        simbolo = "sbooleano";
    } else if (strcmp(buffer, "verdadeiro") == 0){
        simbolo = "sverdadeiro";
    } else if (strcmp(buffer, "falso") == 0){
        simbolo = "sfalso";
    } else if (strcmp(buffer, "procedimento") == 0){
        simbolo = "sprocedimento";
    } else if (strcmp(buffer, "funcao") == 0){
        simbolo = "sfuncao";
    } else if (strcmp(buffer, "div") == 0){
        simbolo = "sdiv";
    } else if (strcmp(buffer, "e") == 0){
        simbolo = "se";
    } else if (strcmp(buffer, "ou") == 0){
        simbolo = "sou";
    } else if (strcmp(buffer, "nao") == 0){
        simbolo = "snao";
    }

    return token_create(buffer, simbolo, linha_inicio);
}

token trata_aritmetico(FILE* file){
    char lexema[2];
    int linha_inicio = line;
    const char* simbolo;

    lexema[0] = ch;
    lexema[1] = '\0';

    if(ch == '+'){
        simbolo = "smais";
    } else if(ch == '-'){
        simbolo = "smenos";
    } else if (ch == '*'){
        simbolo = "smult";
    } else {
        ch = fgetc(file);
        return token_create_error(linha_inicio);
    }

    ch = fgetc(file);

    return token_create(lexema, simbolo, linha_inicio);
}

token trata_relacional(FILE* file){
    char buffer[3];
    int linha_inicio = line;
    const char* simbolo;

    if(ch == '='){
        ch = fgetc(file);
        return token_create("=", "sig", linha_inicio);
    }

    char operator = ch;
    ch = fgetc(file);

    if(operator == '!' && ch == '='){
        strcpy(buffer, "!=");
        simbolo = "sdif";
        ch = fgetc(file);
    } else if(operator == '<' && ch == '='){
        strcpy(buffer, "<=");
        simbolo = "smenorig";
        ch = fgetc(file);
    } else if(operator == '<'){
        strcpy(buffer, "<");
        simbolo = "smenor";
    } else if(operator == '>' && ch == '='){
        strcpy(buffer, ">=");
        simbolo = "smaiorig";
        ch = fgetc(file);
    } else if(operator == '>'){
        strcpy(buffer, ">");
        simbolo = "smaior";
    } else {
        return token_create_error(linha_inicio);
    }

    return token_create(buffer, simbolo, linha_inicio);
}

token trata_atribuicao(FILE* file){
    int linha_inicio = line;
    ch = fgetc(file);

    if(ch == '='){
        ch = fgetc(file);
        return token_create(":=", "satribuicao", linha_inicio);
    } else {
        return token_create(":", "sdoispontos", linha_inicio);
    }
}

token trata_pontuacao(FILE* file){
    char lexema[2];
    int linha_inicio = line;
    const char* simbolo;

    lexema[0] = ch;
    lexema[1] = '\0';

    if(ch == '.'){
        simbolo = "sponto";
    } else if(ch == ';'){
        simbolo = "sponto_virgula";
    } else if(ch == ','){
        simbolo = "svirgula";
    } else if(ch == '('){
        simbolo = "sabre_parenteses";
    } else if(ch == ')'){
        simbolo = "sfecha_parenteses";
    } else {
        ch = fgetc(file);
        return token_create_error(linha_inicio);
    }

    ch = fgetc(file);

    return token_create(lexema, simbolo, linha_inicio);
}

token pega_token(FILE* file){
    int linha_inicio = line;

    if (isdigit(ch)){
        return trata_digito(file);
    } else if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122)){
        return trata_identificador(file);
    } else if (ch == ':'){
        return trata_atribuicao(file);
    } else if (ch == '+' || ch == '-' || ch == '*'){
        return trata_aritmetico(file);
    } else if (ch == '!' || ch == '<' || ch == '>' || ch == '='){
        return trata_relacional(file);
    } else if (ch == '.' || ch == ';' || ch == ',' || ch == '(' || ch == ')'){
        return trata_pontuacao(file);
    } else {
        ch = fgetc(file);
        return token_create_error(linha_inicio);
    }
}

void salva_tabela_simbolos(FILE* out, token t) {
    if (!out) {
        return;
    }
    fprintf(out, "%-21s | %-20s\n", t.lexema, t.simbolo);
}

void lexer_init(FILE* file){
    ch = fgetc(file);
    line = 1;
}

token lexer(FILE* file, FILE* out) {

    while((ch == '{' || ch == ' ' || ch == '\n' || ch == '\t' || ch == '\b' || ch == 10 || ch == '\r') && ch != EOF){
        if(ch == '\r'){
            ch = fgetc(file);
            if(ch == '\n'){
                // \r\n encontrado, conta só uma linha
                line++;
            } else {
                // Só \r, conta linha e devolve o próximo caractere
                line++;
            }
        } else if (ch == '\n'){
            ch = fgetc(file);
            line++;
        }
        if(ch == '{'){
            while(ch != '}' && ch != EOF){
                if(ch == '\r'){
                    ch = fgetc(file);
                    if(ch == '\n'){
                        // \r\n encontrado, conta só uma linha
                        ch = fgetc(file);
                        line++;
                    } else {
                        // Só \r, conta linha e devolve o próximo caractere
                        line++;
                    }
                } else if (ch == '\n'){
                    ch = fgetc(file);
                    line++;
                } else {
                    ch = fgetc(file);
                }
            }
            if(ch == EOF){
                token t;
                t.lexema = malloc(16);
                if (t.lexema != NULL) {
                    sprintf(t.lexema, "Fim do arquivo!");
                }
                strcpy(t.simbolo, "seof");
                salva_tabela_simbolos(out, t);
                if (t.lexema){
                    free(t.lexema);
                }
            }
        }
        ch = fgetc(file);
        while((ch == ' ' || ch == '\n' || ch == '\t' || ch == '\b' || ch == 10 || ch == 13) && ch != EOF){
            if(ch == '\r'){
                ch = fgetc(file);
                if(ch == '\n'){
                    // \r\n encontrado, conta só uma linha
                    ch = fgetc(file);
                    line++;
                } else {
                    // Só \r, conta linha e devolve o próximo caractere
                    line++;
                }
            } else if (ch == '\n'){
                ch = fgetc(file);
                line++;
            } else {
                ch = fgetc(file);
            }
        }
    }
    token t = pega_token(file);
    salva_tabela_simbolos(out, t);
    return t;
}

// token lexer(FILE* file, FILE* out) {
//     // pula espaços e comentários até encontrar um caractere significativo
//     while (ch != EOF) {
//         if (ch == '{') {
//             // pula comentário até '}' (ou EOF), atualizando número de linhas
//             ch = fgetc(file);
//             while (ch != '}' && ch != EOF) {
//                 if (ch == '\n') line++;
//                 ch = fgetc(file);
//             }
//             if (ch == '}') {
//                 // consome o '}' e pega o próximo caractere
//                 ch = fgetc(file);
//             }
//         } else if (ch == '\r') {
//             ch = fgetc(file);
//             if (ch == '\n') ch = fgetc(file); // trata CRLF
//             line++;
//         } else if (ch == '\n') {
//             ch = fgetc(file);
//             line++;
//         } else if (isspace((unsigned char)ch)) {
//             ch = fgetc(file);
//         } else {
//             break; // encontrou caractere significativo
//         }
//     }

//     if (ch == EOF) {
//         // opcional: retornar token EOF/erro
//         token t = token_create("", "seof", line);
//         salva_tabela_simbolos(out, t);
//         return t;
//     }

//     token t = pega_token(file);
//     salva_tabela_simbolos(out, t);
//     return t;
// }
