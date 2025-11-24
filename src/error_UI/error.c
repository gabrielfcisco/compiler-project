// src/error.c
#include "../../include/error_UI/error.h"
#include <stdarg.h>
#include <time.h>

// Retorna uma string representando o tipo de erro informado.
static const char *type_str(ErrorType t) {
    switch(t){
        case ERR_LEXICAL: return "LEXICAL";
        case ERR_SYNTACTIC: return "SYNTACTIC";
        case ERR_SEMANTIC: return "SEMANTIC";
        case ERR_CODEGEN: return "CODEGEN";
        case ERR_INFO: return "INFO";
        default: return "UNKNOWN";
    }
}

// Reporta um erro formatado na saída padrão de erro, com timestamp, tipo, linha e token.
void report_error(ErrorType type, int linha, const char *token, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    // timestamp opcional
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);

    fprintf(stderr, "[%s] %s", ts, type_str(type));
    if (linha >= 0) fprintf(stderr, " linha=%d", linha);
    if (token && token[0]) fprintf(stderr, " token=%s", token);

    fprintf(stderr, " msg="); 
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

// Reporta uma expressão em notação infixa e pós-fixa para depuração/análise, com timestamp.
void report_posfix(int id, const char *origem, const char *infixa, const char *posfixa) {
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);

    // Tudo em uma linha para fácil parsing na GUI
    fprintf(stderr, "[%s] POSFIXA id=%d origem=%s infixa=\"%s\" posfixa=\"%s\"\n", ts, id, origem ? origem : "-", infixa ? infixa : "-", posfixa ? posfixa : "-");
}
