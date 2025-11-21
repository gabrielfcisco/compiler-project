// include/error.h
#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

typedef enum {
    ERR_LEXICAL,
    ERR_SYNTACTIC,
    ERR_SEMANTIC,
    ERR_CODEGEN,
    ERR_INFO  // para logs informativos (ex: posfixa)
} ErrorType;

/**
 * report_error:
 *  type -> tipo do erro (ERR_LEXICAL, ERR_SYNTACTIC, ERR_SEMANTIC, ERR_CODEGEN)
 *  linha -> número da linha (ou -1 se desconhecido)
 *  token -> token/lexema associado (ou NULL)
 *  fmt,... -> mensagem formatada (printf-style)
 *
 * Saída padronizada em stderr:
 *  [TYPE] linha=X token=LEXEMA msg=...
 */
void report_error(ErrorType type, int linha, const char *token, const char *fmt, ...);

/**
 * report_posfix:
 *  id -> id único da ocorrência
 *  origem -> contexto (por ex. "se", "atribuicao", nome da função)
 *  infixa -> string com expressão infixa
 *  posfixa -> string com expressão posfixa
 *
 * Gera uma linha padronizada em stderr que a GUI vai capturar.
 */
void report_posfix(int id, const char *origem, const char *infixa, const char *posfixa);

#endif // ERROR_H
