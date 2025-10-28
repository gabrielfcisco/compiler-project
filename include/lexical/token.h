#ifndef TOKEN_H
#define TOKEN_H


typedef struct token {
    char* lexema;
    char simbolo[20];
    int linha;
    int erro;  // 0 = ok, 1 = erro
    int unario; // 0 = nao, 1 = sim
} token;

// Prototype

token token_create(char* lexema, char* simbolo, int linha);
token token_create_error(int linha);
void token_free(token* t);
token trata_digito(FILE* file);
token trata_identificador(FILE* file);
token trata_aritmetico(FILE* file);
token trata_relacional(FILE* file);
token trata_atribuicao(FILE* file);
token trata_pontuacao(FILE* file);
token pega_token(FILE* file);
void salva_tabela_simbolos(FILE* out, token t);
void lexer_init(FILE* file);
token lexer(FILE* file, FILE* out);

#endif

