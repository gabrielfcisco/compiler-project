#ifndef GENERATOR_H
#define GENERATOR_H

typedef struct{
    char *rotulo;
    char* instrucao;            // ROTULO  |  INSTRUCAO  |  OPERANDO1  |  OPERANDO2 
    char* operando1;            //   1          NULL                                            *EXEMPLOS
    char* operando2;            //              ALLOC           2      ,      2
} generator_struct;


#ifdef _WIN32
  #include <direct.h>  // _mkdir
  #define MKDIR(p) _mkdir(p)
#else
  #define MKDIR(p) mkdir(p, 0755)
#endif


// int generator_open(const char* path, int truncate); // fazer aqui quando voltar
// void generator_close(void);

// Prototype

void Gera(char *rotulo, char *instrucao, char *operando1, char *operando2);
char *convert_integer_to_string(int input);
int convert_string_to_integer(const char *input);
void new_program_code();




#endif
