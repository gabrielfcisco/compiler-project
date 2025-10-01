#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdio.h>

void lexer_init(FILE* file);
token lexer(FILE* file, FILE* out);

#endif