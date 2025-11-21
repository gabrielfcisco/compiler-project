#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/code_generator/generator.h"
#include "../../include/error_UI/error.h"

int gera_comando(generator_struct g);
void print_if_not_empty(FILE *arquivo, char *string);

void Gera(char* rotulo, char *instrucao, char *operando1, char *operando2){
    generator_struct g;

    g.rotulo = rotulo;
    g.instrucao = instrucao;
    g.operando1 = operando1;
    g.operando2 = operando2;

    if (gera_comando(g) == 1){
        report_error(ERR_CODEGEN, 10, g.instrucao, "Não foi possivel gerar o codigo");
    }
}

int gera_comando(generator_struct g){
    FILE *arquivo;

    arquivo = fopen("codigo_maquina_virtual.txt", "a");

    if (arquivo == NULL) {
        perror("Nao foi possivel criar o arquivo");
        return 1;
    }

    print_if_not_empty(arquivo,g.rotulo);
    print_if_not_empty(arquivo,g.instrucao);
    print_if_not_empty(arquivo,g.operando1);
    print_if_not_empty(arquivo,g.operando2);
    fprintf(arquivo,"\n");

    // printf(" Codigo '%s %s %s %s' gerado ! \n",g.rotulo,g.instrucao,g.operando1,g.operando2);
    fclose(arquivo);
    return 0;
}

void print_if_not_empty(FILE *arquivo, char *string){
    if (string && *string) {                 // só imprime se não for NULL e não for ""
        fprintf(arquivo, "%s", string);
        fprintf(arquivo," ");
    }
}

char *convert_integer_to_string(int input){
    char *output = malloc(40);
    if (!output) return NULL;

    sprintf(output,"%d",input);

    return output;
}

int convert_string_to_integer(const char *input) {
    if (input == NULL) return 0;  
    return atoi(input);           // converte string → int
}

void new_program_code(){
    FILE *arquivo = fopen("codigo_maquina_virtual.txt", "w");

    if (arquivo == NULL) {
        report_error(ERR_CODEGEN, 3, NULL, "Erro ao criar o arquivo do codigo do programa programa");
        exit(1);
    }

    fclose(arquivo);
    printf("Arquivo 'codigo_maquina_virtual.txt' criado com sucesso.\n");
}