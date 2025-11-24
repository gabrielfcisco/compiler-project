#ifndef PARSER_H
#define PARSER_H

#include "../lexical/token.h"
#include "../semantic/semantic.h"

typedef struct {
    FILE *file;
    FILE *out;
    token t;
} parser;

// Prototype
void atualiza_in_fixa(token *in_fixa, int *pos, token t);
void print_in_and_pos_fixa(token *vetor_tokens, int pos, int fixa, const char *origem);
int precedencia(token t);
token *pos_fixa (token *in_fixa, int pos, int *posf);
token analisa_bloco(parser *p);
token analisa_comandos_simples(parser *p);
token analisa_comandos(parser *p);
token analisa_atrib_chprocedimento(parser *p);
token analisa_atribuicao(parser *p, token left_side);
token analisa_escreva(parser *p);
token analisa_leia(parser *p);
token analisa_enquanto(parser *p);
token analisa_se(parser *p);
token analisa_expressao_simples(parser *p, token *in_fixa, int *pos);
token analisa_expressao(parser *p, token *in_fixa, int *pos);
token analisa_termo(parser *p, token *in_fixa, int *pos);
token analisa_chamada_funcao(parser *p);
token analisa_chamada_procedimento(parser *p, int end);
token analisa_fator(parser *p, token *in_fixa, int *pos);

#endif