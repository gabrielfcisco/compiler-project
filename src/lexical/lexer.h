#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdio.h>

token lexer(FILE* file, FILE* out);

#endif