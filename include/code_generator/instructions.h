#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "../../include/lexical/token.h"

// Prototype

void instrucao(char *instrucao, char *operando1, char *operando2);

void ins_atr_expressao(char *lexema);

void ins_expressao(token *vetor_pos_fixa, int posf);

int verify_if_is_aritmetic(char *operando); //para fazer atribuicao

int verify_if_is_relational(char *operando);

char* deep_copy(const char *orig);



#endif
