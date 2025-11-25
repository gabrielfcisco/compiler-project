/**********************************************
 * Arquivo: generator.c
 * Autores: Enzo, Gabriel, Guilherme, Samuel
 * 
 *
 * Descrição:
 *    Implementa o gerador de código de máquina virtual do compilador,
 *    Responsável por escrever instruções formatadas no arquivo de saída,
 *    gerenciar rótulos, operandos e gerar o código intermediário executável.
 *    
 *
 * Dependências:
 *    - generator.h
 *    - error.h
 *
 **********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../include/code_generator/generator.h"
#include "../../include/error_UI/error.h"

// Gera uma linha de código de máquina virtual a partir dos parâmetros fornecidos.
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

// Escreve o comando formatado no arquivo de saída do código de máquina virtual.
int gera_comando(generator_struct g){
    FILE *arquivo;

    arquivo = fopen("./output/codigo_maquina_virtual.obj", "a");

    if (arquivo == NULL) {
        perror("Nao foi possivel criar o arquivo");
        return 1;
    }

    if (strcmp(g.instrucao,"NULL") == 0) {
        fprintf(arquivo, "%s\t", g.rotulo);
    } else {
        // Se não tem rótulo → imprime TAB no começo
        fprintf(arquivo, "\t");
    }
    print_if_not_empty(arquivo,g.instrucao);
    print_if_not_empty(arquivo,g.operando1);
    print_if_not_empty(arquivo,g.operando2);
    fprintf(arquivo,"\n");

    // printf(" Codigo '%s %s %s %s' gerado ! \n",g.rotulo,g.instrucao,g.operando1,g.operando2);
    fclose(arquivo);
    return 0;
}

// Imprime a string no arquivo se ela não for vazia ou nula.
void print_if_not_empty(FILE *arquivo, char *string){
    if (string && *string) {                 // só imprime se não for NULL e não for ""
        fprintf(arquivo, "%s", string);
        fprintf(arquivo," ");
    }
}

// Converte um inteiro para uma string alocada dinamicamente.
char *convert_integer_to_string(int input){
    char *output = malloc(40);
    if (!output) return NULL;

    sprintf(output,"%d",input);

    return output;
}

// Converte uma string para inteiro (retorna 0 se input for NULL).
int convert_string_to_integer(const char *input) {
    if (input == NULL) return 0;  
    return atoi(input);           // converte string → int
}

// Cria o arquivo de código de máquina virtual e o diretório de saída, se necessário.
void new_program_code(){
    // Criar diretório "output" caso não exista
    struct stat st;
    if (stat("output", &st) != 0) {
        MKDIR("output");
    }

    FILE *arquivo1 = fopen("output/codigo_maquina_virtual.obj", "w");

    if (arquivo1 == NULL) {
        report_error(ERR_CODEGEN, 3, NULL,
                     "Erro ao criar o arquivo do código do programa");
        exit(1);
    }

    fclose(arquivo1);

    printf("Arquivo 'codigo_maquina_virtual.obj' criado com sucesso em './output'.\n");
}