#ifndef GENERATOR_H
#define GENERATOR_H

typedef struct{
    int rotulo;// obs: -1 para sem rótulo; >=0 para L<rotulo>
    char* instrucao;            // ROTULO  |  INSTRUCAO  |  OPERANDO1  |  OPERANDO2 
    char* operando1;            //   1          NULL                                            *EXEMPLOS
    char* operando2;            //              ALLOC           2      ,      2
} generator_struct;


int generator_open(const char* path, int truncate); 
void generator_close(void);

void Gera(int rotulo, char *instrucao, char *operando1, char *operando2);

int Gera_arquivo(generator_struct g);

#endif
