#ifndef TOKEN_H
#define TOKEN_H

typedef struct token {
    char* lexema;
    char simbolo[20];
    int linha;
    int erro;  // 0 = ok, 1 = erro
} token;

token token_create(const char* lexema, const char* simbolo, int linha);
token token_create_error(int linha);
void token_free(token* t);

#endif

