/**********************************************
 * Arquivo: parser.c
 * Autores: Enzo, Gabriel, Guilherme, Samuel
 * 
 *
 * Descrição:
 *    Implementa a sintatico do compilador,
 *    Responsável por vericar a sintaxe, a sequencia dos lexemas, 
 *    realizar chamadas ao lexico, ao semantico e ao gerador de codigo.
 *    Alem disso, e a rotina principal do compilador (main).
 *    
 *
 * Dependências:
 *    - parser.h
 *    - lexer.h
 *    - semantic.h
 *    - generator.h
 *    - instruction.h
 *    - error.h
 *
 * Como compilar:
 *    make
 *    OU
 *    gcc src/syntactic/parser.c src/lexical/lexer.c src/semantic/semantic.c src/code_generator/generator.c src/code_generator/instructions.c ./src/error_UI/error.c -o parser
 *
 * Como executar:
 *    ./parser
 **********************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../../include/parser/parser.h"
#include "../../include/lexical/lexer.h"
#include "../../include/semantic/semantic.h"
#include "../../include/code_generator/generator.h"
#include "../../include/code_generator/instructions.h"
#include "../../include/error_UI/error.h"

Tabsimb** sp_parser;
int rotulo = 1;
int endereco_var = 1;

// Imprime as informações de um token para depuração.
void imprimir_token(token t) {

    printf("--- Conteúdo do Token ---\n");
    printf("Lexema: %s\n", t.lexema);
    printf("Símbolo: %s\n", t.simbolo);
    printf("Linha: %d\n", t.linha);
    printf("Erro: %d (%s)\n", t.erro, t.erro == 0 ? "ok" : "erro");
    printf("-------------------------\n\n");
}

// Atualiza o vetor de tokens infixa, tratando operadores unários e binários.
void atualiza_in_fixa(token *in_fixa, int *pos, token t) {

    if (t.unario == 1) {
        if (strcmp(t.simbolo, "smenos") == 0) {
            in_fixa[*pos] = token_create("inv", "sinv", t.linha);
            in_fixa[*pos].unario = 1;
        } 
        else if (strcmp(t.simbolo, "snao") == 0) {
            in_fixa[*pos] = token_create("neg", t.simbolo, t.linha);
            in_fixa[*pos].unario = 1;
        }
        else if (strcmp(t.simbolo, "smais") == 0) {
            return; // descarta + unário
        }
    }else{
        in_fixa[*pos] = token_create(t.lexema, t.simbolo, t.linha);
    }
    
    (*pos)++;
}

int pf_id_counter = 0;

// Imprime a expressão em notação infixa ou posfixa, e chama report_posfix para depuração.
void print_in_and_pos_fixa(token *vetor_tokens, int pos, int fixa, const char *origem) {
    // buffers locais para montar as strings
    char buffer_tmp[2048];
    buffer_tmp[0] = '\0';

    for (int i = 0; i < pos; ++i) {
        // protege contra lexemas nulos
        if (vetor_tokens[i].lexema && vetor_tokens[i].lexema[0]) {
            // concat com segurança (usando strncat)
            strncat(buffer_tmp, vetor_tokens[i].lexema, sizeof(buffer_tmp) - strlen(buffer_tmp) - 1);
            if (i + 1 < pos) {
                strncat(buffer_tmp, " ", sizeof(buffer_tmp) - strlen(buffer_tmp) - 1);
            }
        }
    }

    #define LAST_INFIX_MAX 4096
    static char last_infix[LAST_INFIX_MAX] = {0};
    static char last_origem[128] = {0};

    if (fixa == 0) {
        // armazenar infixa para combinar depois
        strncpy(last_infix, buffer_tmp, LAST_INFIX_MAX - 1);
        last_infix[LAST_INFIX_MAX - 1] = '\0';

        if (origem) {
            strncpy(last_origem, origem, sizeof(last_origem) - 1);
            last_origem[sizeof(last_origem) - 1] = '\0';
        } else {
            last_origem[0] = '\0';
        }

        // imprimir para compatibilidade/depuração
        // printf("Expressão em notação infixa: %s\n", last_infix);
        fflush(stdout);
    } else {
        // posfixa: buffer_tmp contém a posfixa
        // printf("Expressão em notação posfixa: %s\n", buffer_tmp);
        fflush(stdout);

        // combine com a infixa armazenada (se houver) e reporte
        const char *use_infix = last_infix[0] ? last_infix : "";
        const char *use_origem = last_origem[0] ? last_origem : (origem ? origem : "-");

        int id = ++pf_id_counter;
        // chama report_posfix com infixa e posfixa
        report_posfix(id, use_origem, use_infix, buffer_tmp);

        // opcional: limpar last_infix para evitar reuso indevido
        last_infix[0] = '\0';
        last_origem[0] = '\0';
    }
}

// Retorna a precedência de um token operador.
int precedencia(token t) {

    if (t.unario == 1) return 6; // maior prioridade para operadores unários   
    if (strcmp(t.simbolo, "smult")    == 0  || strcmp(t.simbolo, "sdiv")     == 0) return 5;
    if (strcmp(t.simbolo, "smais")    == 0  || strcmp(t.simbolo, "smenos")   == 0) return 4;
    if (strcmp(t.simbolo, "smaior")   == 0  || strcmp(t.simbolo, "smenor")   == 0 ||
        strcmp(t.simbolo, "smaiorig") == 0  || strcmp(t.simbolo, "smenorig") == 0 ||
        strcmp(t.simbolo, "sdif")     == 0) return 3; 
    if (strcmp(t.simbolo, "se")   == 0) return 2;
    if (strcmp(t.simbolo, "sou")  == 0) return 1;
    return 0; // menor prioridade possível (ou token inválido)

}

// Converte um vetor de tokens em notação infixa para posfixa (Shunting Yard).
token *pos_fixa (token *in_fixa, int pos, int *posf) {
    // Implementação do algoritmo Shunting Yard para converter infixa para posfixa
    token *pilha = (token *)malloc(pos * sizeof(token));
    token *out = (token *)malloc(pos * sizeof(token));
    int topo = -1;
    *posf = 0;

    for (int i = 0; i < pos; i++) {
        token t = in_fixa[i];

        if (strcmp(t.simbolo, "snumero") == 0     || strcmp(t.simbolo, "sidentificador") == 0 ||
            strcmp(t.simbolo, "sverdadeiro") == 0 || strcmp(t.simbolo, "sfalso") == 0){
            out[(*posf)++] = token_create(t.lexema, t.simbolo, t.linha);
        } else if (strcmp(t.simbolo, "smais") == 0    || strcmp(t.simbolo, "smenos") == 0   ||
                   strcmp(t.simbolo, "smult") == 0    || strcmp(t.simbolo, "sdiv") == 0     ||
                   strcmp(t.simbolo, "se") == 0       || strcmp(t.simbolo, "sou") == 0      ||
                   strcmp(t.simbolo, "smaior") == 0   || strcmp(t.simbolo, "smenor") == 0   ||
                   strcmp(t.simbolo, "smaiorig") == 0 || strcmp(t.simbolo, "smenorig") == 0 ||
                   strcmp(t.simbolo, "sdif") == 0     || strcmp(t.simbolo, "snao") == 0     || 
                   strcmp(t.simbolo, "sinv") == 0     || strcmp(t.simbolo, "sig") == 0) {

            while (topo != -1 && strcmp(pilha[topo].simbolo, "sabre_parenteses") != 0 && precedencia(pilha[topo]) >= precedencia(t)) {
                out[(*posf)++] = pilha[topo--];
            }
            if (strcmp(t.simbolo, "snao") == 0 || strcmp(t.simbolo, "sinv") == 0){
                pilha[++topo] = token_create(t.lexema, t.simbolo, t.linha);
                pilha[topo].unario = 1;
            }
            else {
                pilha[++topo] = token_create(t.lexema, t.simbolo, t.linha);
            }
           
        } else if (strcmp(t.simbolo, "sabre_parenteses") == 0) {
            pilha[++topo] = token_create(t.lexema, t.simbolo, t.linha);
        } else if (strcmp(t.simbolo, "sfecha_parenteses") == 0) {
            while (topo != -1 && strcmp(pilha[topo].simbolo, "sabre_parenteses") != 0) {
                out[(*posf)++] = pilha[topo--];
            }
            if (topo != -1) {
                token_free(&pilha[topo--]); // Remove o parêntese de abertura
            }
        }
    }
    while (topo != -1) {
        out[(*posf)++] = pilha[topo--];
    }
    free(pilha);
    return out;
}

// Analisa e valida o tipo de um token (só aceita inteiro ou booleano).
token analisa_tipo(parser *p) {

    if(strcmp(p->t.simbolo, "sinteiro") != 0 && strcmp(p->t.simbolo, "sbooleano") != 0){
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
        token_free(&p->t);
        exit(1);
    }else{
        coloca_tipo_tabela(p->t.lexema);
    }
    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    return p->t;
}

// Analisa declaração de variáveis, verifica duplicidade e insere na tabela de símbolos.
token analisa_variaveis(parser *p, int *counter_var){

    while(strcmp(p->t.simbolo, "sdoispontos") != 0){
        if(strcmp(p->t.simbolo, "sidentificador") == 0){
            if(pesquisa_duplica_var_tabela(p->t.lexema) == 0){
                insere_tabela(p->t.lexema,"variavel",' ', 0, &endereco_var, 1);
                (*counter_var)++; // conta cada variavel dentro do bloco ex: var a,b,c inteiro;

                token_free(&p->t);
                p->t = lexer(p->file, p->out);
                if(strcmp(p->t.simbolo, "svirgula") == 0 || strcmp(p->t.simbolo, "sdoispontos") == 0){
                    if(strcmp(p->t.simbolo, "svirgula") == 0){
                        token_free(&p->t);
                        p->t = lexer(p->file, p->out);
                        if(strcmp(p->t.simbolo, "sdoispontos") == 0){
                            report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
                            exit(1);
                        }
                    }
                }else{
                    report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
                    exit(1);
                }
            }else{
                report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "variavel duplicada ou não declarada");
                exit(1);
            }
        }else{
            report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
            exit(1);
        }
    }
    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    return analisa_tipo(p);
}

// Analisa o bloco de declaração de variáveis.
token analisa_et_variaveis(parser *p,int *counter_var){

    if(strcmp(p->t.simbolo, "svar") == 0){
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        if(strcmp(p->t.simbolo, "sidentificador") == 0){
            while(strcmp(p->t.simbolo, "sidentificador") == 0){
                p->t = analisa_variaveis(p,counter_var);         // passa o ponteiro do contador de variaveis para contar

                if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);
                } else {
                    report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
                    exit(1);
                }
            }
        }else{
            report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
            exit(1);
        }
    }
    return p->t;
}

// Analisa declaração de procedimento, insere na tabela e gera rótulo.
token analisa_declaracao_procedimento(parser *p){

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    char nivel = 'L';
    if(strcmp(p->t.simbolo, "sidentificador") == 0){

        if (pesquisa_declproc_dup_tabela(p->t.lexema) == 0 ){

            insere_tabela(p->t.lexema,"procedimento", nivel, rotulo, &endereco_var, 0);  //{guarda tabela de simb}

            char *endereco = convert_integer_to_string(rotulo);
            instrucao("label", endereco, ""); // CALL ira buscar este rotulo na tabsimb
            free(endereco);
            rotulo++;

            p->t = lexer(p->file, p->out);

            if(strcmp(p->t.simbolo, "sponto_virgula") == 0){

                p->t = analisa_bloco(p);

            }else{
                report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
                exit(1);
            }
        }else{
            report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "procedimento duplicado ou nao declarado");
            exit(1);
        }
    }else{
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
        exit(1);
    }
    desempilha_ou_voltanivel();

    instrucao("return","","");
    
    return p->t;
}

// Analisa declaração de função, insere na tabela, gera rótulo e tipo.
token analisa_declaracao_funcao(parser *p){

    token_free(&p->t);
    p->t = lexer(p->file, p->out);
    char nivel = 'L';

    if(strcmp(p->t.simbolo, "sidentificador") == 0){
        if(pesquisa_declfunc_dup_tabela(p->t.lexema) == 0){
            insere_tabela(p->t.lexema,"", nivel, rotulo, &endereco_var, 0);

            char *endereco = convert_integer_to_string(rotulo);
            instrucao("label", endereco, ""); // CALL ira buscar este rotulo na tabsimb
            free(endereco);
            rotulo++;
            
            p->t = lexer(p->file, p->out);
            if(strcmp(p->t.simbolo, "sdoispontos") == 0){
                p->t = lexer(p->file, p->out);
                if((strcmp(p->t.simbolo, "sinteiro") == 0) || strcmp(p->t.simbolo, "sbooleano") == 0){
                    if(strcmp(p->t.simbolo, "sinteiro") == 0){
                        strcpy((*sp_parser)->tipo, "funcao inteiro");
                    }else{
                        strcpy((*sp_parser)->tipo, "funcao booleano");
                    }
                    p->t = lexer(p->file, p->out);
                    if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
                        p->t = analisa_bloco(p);
                    }
                } else {
                    report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
                    exit(1);
                }
            }else{
                report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
                exit(1);
            }
        }else{
            report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "funcao duplicada ou nao declarado");
            exit(1);
        }
    }else{
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
        exit(1);
    }

    desempilha_ou_voltanivel();

    instrucao("return","","");

    return p->t;
}

// Analisa sub-rotinas (funções e procedimentos) e gera saltos para o bloco principal.
token analisa_subrotinas(parser *p){

    int auxrot,flag;
    char *endereco;
    flag = 0;
    if((strcmp(p->t.simbolo, "sprocedimento") == 0) || (strcmp(p->t.simbolo, "sfuncao") == 0)){

        auxrot = rotulo;

        endereco = convert_integer_to_string(rotulo);
        instrucao("jmp", endereco, "");                 //{salta sub-rotinas}
        free(endereco);

        rotulo++;
        flag = 1;
    }

    while((strcmp(p->t.simbolo, "sprocedimento") == 0) || (strcmp(p->t.simbolo, "sfuncao") == 0)) {
        
        if(strcmp(p->t.simbolo, "sprocedimento") == 0) {
            p->t = analisa_declaracao_procedimento(p);
        } else {
            p->t = analisa_declaracao_funcao(p);
        }
        if(strcmp(p->t.simbolo, "sponto_virgula") == 0){
            token_free(&p->t);
            p->t = lexer(p->file, p->out);
        }else{
            report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
            exit(1);
        }
    }

    if (flag == 1){
        endereco = convert_integer_to_string(auxrot);
        instrucao("label", endereco, ""); // {inicio do principal}
        free(endereco);
    }

    return p->t;
}

// Analisa comandos simples (atribuição, chamada, controle, E/S).
token analisa_comandos_simples(parser *p) {
    
    if (strcmp(p->t.simbolo, "sidentificador") == 0) {
        p->t = analisa_atrib_chprocedimento(p);
    } else if (strcmp(p->t.simbolo, "sse") == 0) {
        p->t = analisa_se(p);
    } else if (strcmp(p->t.simbolo, "senquanto") == 0) {
        p->t = analisa_enquanto(p);
    } else if (strcmp(p->t.simbolo, "sleia") == 0) {
        p->t = analisa_leia(p);
    } else if (strcmp(p->t.simbolo, "sescreva") == 0) {
        p->t = analisa_escreva(p);
    } else {
        p->t = analisa_comandos(p);
    }
    return p->t;
}

// Analisa atribuição ou chamada de procedimento.
token analisa_atrib_chprocedimento(parser *p) {
    
    token left_side = token_create(p->t.lexema, p->t.simbolo, p->t.linha); // guardando o token da esquerda para gerar o codigo depois de analisar a expressao da direita
    Tabsimb *sp_aux;
    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    if (strcmp(p->t.simbolo, "satribuicao") == 0) {
        if(pesquisa_tabela(left_side.lexema, &sp_aux) == 1){
            p->t = analisa_atribuicao(p, left_side);
        }
        else{
            report_error(ERR_SEMANTIC, p->t.linha, left_side.lexema, "Variavel nao declarada");
            exit(1);
        }
    } else {

        if(pesquisa_tabela(left_side.lexema, &sp_aux) == 1){
            p->t = analisa_chamada_procedimento(p, sp_aux->end);
        }else{
            report_error(ERR_SEMANTIC, p->t.linha, left_side.lexema, "Procedimento nao encontrado");
            exit(1);
        }
    }
    token_free(&left_side);
    return p->t;
}

// Analisa comando 'se', gera código para saltos e blocos 'então'/'senão'.
token analisa_se(parser *p) {

    token in_fixa[1000];
    token *vetor_pos_fixa;
    int pos = 0;  // numero de elementos do vetor in_fixa ao termino da expressao
    int posf = 0; // numero de elementos do vetor pos_fixa ao termino da expressao
    int flag = 0; // flag para saber se o se tem senao
    char *endereco;
    int auxrot, auxrot2;
    int tipo_pos_fixa = -1;

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    p->t = analisa_expressao(p, in_fixa, &pos);
    
    vetor_pos_fixa = pos_fixa(in_fixa, pos, &posf);

    tipo_pos_fixa = verifica_tipo_pos_fixa(vetor_pos_fixa, posf);

    if (tipo_pos_fixa == -1){
        report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "Tipos incompativeis");
        exit(1);
    }
    
    print_in_and_pos_fixa(in_fixa, pos, 0, "se");
    print_in_and_pos_fixa(vetor_pos_fixa, posf, 1, "se");

    ins_expressao(vetor_pos_fixa, posf);   // gera as instrucoes conforme pos_fixa

    auxrot = rotulo;

    endereco = convert_integer_to_string(rotulo);
    instrucao("jmpf", endereco, "");// {salta se falso}
    free(endereco);

    rotulo ++;

    for (int i = 0; i < pos; i++)
        token_free(&in_fixa[i]);

    for (int i = 0; i < posf; i++)
        token_free(&vetor_pos_fixa[i]);

    free(vetor_pos_fixa);

    if (strcmp(p->t.simbolo, "sentao") == 0) {

        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p);

      

        if (strcmp(p->t.simbolo, "ssenao") == 0) {
            flag = 1;
            auxrot2 = rotulo;
            endereco = convert_integer_to_string(rotulo);
            instrucao("jmp", endereco, "");// {salta o (senao)se entrar no entao}
            free(endereco);
            
            rotulo ++;

            endereco = convert_integer_to_string(auxrot);
            instrucao("label", endereco, "");  //gera o label para o jmpf
            free(endereco);

            token_free(&p->t);
            p->t = lexer(p->file, p->out);
            p->t = analisa_comandos_simples(p);

            endereco = convert_integer_to_string(auxrot2);
            instrucao("label", endereco, "");  //gera o label para saltar o senao
            free(endereco);
        }
    } else {
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "Entao");
        exit(1);
    }

    if(flag == 0){
        endereco = convert_integer_to_string(auxrot);
        instrucao("label", endereco, "");  //gera o label se so tiver se
        free(endereco);
    }

    
    return p->t;
}

// Analisa comando 'enquanto', gera código para laço e saltos.
token analisa_enquanto(parser *p) {

    char *endereco;
    int auxrot1,auxrot2;
    auxrot1 = rotulo;
    
    endereco = convert_integer_to_string(rotulo);
    instrucao("label", endereco, ""); // {inicio do while}
    free(endereco);
    rotulo++;

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    token in_fixa[1000];
    token *vetor_pos_fixa;
    int pos = 0;  // numero de elementos do vetor in_fixa ao termino da expressao
    int posf = 0; // numero de elementos do vetor pos_fixa ao termino da expressao
    int tipo_pos_fixa = -1;

    p->t = analisa_expressao(p, in_fixa, &pos);
    
    vetor_pos_fixa = pos_fixa(in_fixa, pos, &posf);

    tipo_pos_fixa = verifica_tipo_pos_fixa(vetor_pos_fixa, posf);
    
    if (tipo_pos_fixa == -1){
        report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "Tipos incompativeis");
        exit(1);
    }

    print_in_and_pos_fixa(in_fixa, pos, 0, "enquanto");
    print_in_and_pos_fixa(vetor_pos_fixa, posf, 1, "enquanto");

    ins_expressao(vetor_pos_fixa, posf);      // gera as instrucoes conforme pos_fixa

    for (int i = 0; i < pos; i++)
        token_free(&in_fixa[i]);

    for (int i = 0; i < posf; i++)
        token_free(&vetor_pos_fixa[i]);

    free(vetor_pos_fixa);

    if (strcmp(p->t.simbolo, "sfaca") == 0) {

        auxrot2 = rotulo;

        endereco = convert_integer_to_string(rotulo);
        instrucao("jmpf", endereco, "");// {salta se falso}
        free(endereco);

        rotulo++;

        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p);

        endereco = convert_integer_to_string(auxrot1);
        instrucao("jmp", endereco, ""); // {retorna inicio do loop}
        free(endereco);

        endereco = convert_integer_to_string(auxrot2);
        instrucao("label", endereco, ""); //{fim do while}
        free(endereco);

    } else {
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
        exit(1);
    }
    return p->t;
}

// Analisa comando 'leia', verifica tipo e gera código de leitura.
token analisa_leia(parser *p) {
    
    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    if (strcmp(p->t.simbolo, "sabre_parenteses") == 0) {

        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        if (strcmp(p->t.simbolo, "sidentificador") == 0) {
            Tabsimb *sp_aux;
            if (pesquisa_tabela(p->t.lexema, &sp_aux) == 1) {
                if (verifica_tipo(p->t) != 0 || strcmp(sp_aux->tipo, "funcao inteiro") == 0){
                    report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "Tipo da variavel incompativel com instrucao leia");
                    exit(1);
                }
                token_free(&p->t);
                p->t = lexer(p->file, p->out);

                if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);

                    char *endereco = convert_integer_to_string(sp_aux->end);
                    instrucao("leia", endereco, "");
                    free(endereco);

                } else {
                    report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
                    exit(1);
                }
            } else {
                report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "identificador nao encontrado");
                exit(1);
            }
        } else {
            report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
            exit(1);
        }
    } else {
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
        exit(1);
    }

    return p->t;
}

// Analisa comando 'escreva', verifica tipo e gera código de escrita.
token analisa_escreva(parser *p) {

    token_free(&p->t);
    p->t = lexer(p->file, p->out);

    if (strcmp(p->t.simbolo, "sabre_parenteses") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        if (strcmp(p->t.simbolo, "sidentificador") == 0) {
            Tabsimb *sp_aux;
            if (pesquisa_tabela(p->t.lexema, &sp_aux) == 1) {
                if (verifica_tipo(p->t) != 0 || strcmp(sp_aux->tipo, "funcao inteiro") == 0){
                    report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "Tipo da variavel incompativel com instrucao escreva");
                    exit(1);
                }
                token_free(&p->t);
                p->t = lexer(p->file, p->out);

                if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
                    token_free(&p->t);
                    p->t = lexer(p->file, p->out);

                    char *endereco = convert_integer_to_string(sp_aux->end);
                    instrucao("escreva", endereco, ""); 
                    free(endereco);
                } else {
                    report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token sfecha_parenteses esperado");
                    exit(1);
                }
            } else {
                report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "identificador nao encontrado");
                exit(1);
            }
        } else {
            report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token sidentificador esperado");
            exit(1);
        }
    } else {
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
        exit(1);
    }

    return p->t;
}

// Analisa atribuição, verifica tipos e gera código para expressão e armazenamento.
token analisa_atribuicao(parser *p, token left_side) {

/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!/// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// fazer a verificacao se o resultado da posfixa é do mesmo tipo do "left_side"

    if (strcmp(p->t.simbolo, "satribuicao") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        token in_fixa[1000];
        token *vetor_pos_fixa;
        int pos = 0;  // numero de elementos do vetor in_fixa ao termino da expressao
        int posf = 0; // numero de elementos do vetor pos_fixa ao termino da expressao
        int tipo_pos_fixa = -1;

        p->t = analisa_expressao(p, in_fixa, &pos);
        
        vetor_pos_fixa = pos_fixa(in_fixa, pos, &posf);
        print_in_and_pos_fixa(in_fixa, pos, 0, "atribuicao");
        print_in_and_pos_fixa(vetor_pos_fixa, posf, 1, "atribuicao");

        tipo_pos_fixa = verifica_tipo_pos_fixa(vetor_pos_fixa, posf);

        if (tipo_pos_fixa == -1){
            report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "Tipos incompativeis");
            exit(1);
        }
        
        if (verifica_tipo(left_side) != tipo_pos_fixa){
            report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "Tipo da expressao incompativel com atribuicao");
            exit(1);
        }
        
        ins_expressao(vetor_pos_fixa, posf);  // gera as instrucoes conforme pos_fixa

        ins_atr_expressao(left_side.lexema);  // gera o store para a expressao

        for (int i = 0; i < pos; i++)
            token_free(&in_fixa[i]);

        for (int i = 0; i < posf; i++)
            token_free(&vetor_pos_fixa[i]);
        
        free(vetor_pos_fixa);


    } else {
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token inesperado");
        exit(1);
    }
    return p->t;
}

// Analisa bloco de comandos (início/fim) e comandos internos.
token analisa_comandos(parser *p) {

    if (strcmp(p->t.simbolo, "sinicio") == 0) {
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_comandos_simples(p);

        while (strcmp(p->t.simbolo, "sfim") != 0) {
            if (strcmp(p->t.simbolo, "sponto_virgula") == 0) {
                token_free(&p->t);
                p->t = lexer(p->file, p->out);
                if (strcmp(p->t.simbolo, "sfim") != 0) {
                    p->t = analisa_comandos_simples(p);
                }
            } else {
                report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token sponto_virgula esperado");
                exit(1);
            }
        }

        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    } else {
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "token sinicio esperado");
        exit(1);
    }
    return p->t;
}

// Analisa expressão relacional, lógica ou aritmética.
token analisa_expressao(parser *p, token *in_fixa, int *pos) {

    p->t = analisa_expressao_simples(p, in_fixa, pos);

    if (strcmp(p->t.simbolo, "smaior") == 0   || strcmp(p->t.simbolo, "smaiorig") == 0 ||
        strcmp(p->t.simbolo, "sig") == 0      || strcmp(p->t.simbolo, "smenor") == 0   || 
        strcmp(p->t.simbolo, "smenorig") == 0 || strcmp(p->t.simbolo, "sdif") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_expressao_simples(p, in_fixa, pos);
    }

    return p->t;
}

// Analisa expressão simples (soma, subtração, ou).
token analisa_expressao_simples(parser *p, token *in_fixa, int *pos) {

    if (strcmp(p->t.simbolo, "smais") == 0 || strcmp(p->t.simbolo, "smenos") == 0) {
        p->t.unario = 1; // marca como operador unário
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    }

    p->t = analisa_termo(p, in_fixa, pos);

    while (strcmp(p->t.simbolo, "smais") == 0 ||
           strcmp(p->t.simbolo, "smenos") == 0 ||
           strcmp(p->t.simbolo, "sou") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_termo(p, in_fixa, pos);
    }

    return p->t;
}

// Analisa termo (multiplicação, divisão, e).
token analisa_termo(parser *p, token *in_fixa, int *pos) {

    p->t = analisa_fator(p, in_fixa, pos);

    while (strcmp(p->t.simbolo, "smult") == 0 ||
           strcmp(p->t.simbolo, "sdiv") == 0 ||
           strcmp(p->t.simbolo, "se") == 0) {

        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);

        p->t = analisa_fator(p, in_fixa, pos);
    }

    return p->t;
}

// Analisa fator (identificador, número, parênteses, unário, booleano).
token analisa_fator(parser *p, token *in_fixa, int *pos) {

    if (strcmp(p->t.simbolo, "sidentificador") == 0) {

        Tabsimb *sp_aux;   // endereco auxiliar para ver se o identificador encontrado e uma funcao
        
        if (pesquisa_tabela(p->t.lexema, &sp_aux) == 1){
            atualiza_in_fixa(in_fixa, pos, p->t);
            if (strcmp(sp_aux->tipo, "funcao inteiro") == 0 || strcmp(sp_aux->tipo, "funcao booleano") == 0){
                p->t = analisa_chamada_funcao(p);
            }else{
                p->t = lexer(p->file, p->out);
            }
        }else{
            report_error(ERR_SEMANTIC, p->t.linha, p->t.lexema, "identificador nao encontrado");
            exit(1);
        }
     } else if (strcmp(p->t.simbolo, "snumero") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    } else if (strcmp(p->t.simbolo, "snao") == 0) {        //nao e neg?
        p->t.unario = 1; // marca como operador unário
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_fator(p, in_fixa, pos);
    } else if (strcmp(p->t.simbolo, "sabre_parenteses") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
        p->t = analisa_expressao(p, in_fixa, pos);
        if (strcmp(p->t.simbolo, "sfecha_parenteses") == 0) {
            atualiza_in_fixa(in_fixa, pos, p->t);
            token_free(&p->t);
            p->t = lexer(p->file, p->out);
        } else {
            report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "esperado fecha parenteses");
            exit(1);
        }
    } else if (strcmp(p->t.lexema, "verdadeiro") == 0 ||
               strcmp(p->t.lexema, "falso") == 0) {
        atualiza_in_fixa(in_fixa, pos, p->t);
        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    } else {
        report_error(ERR_SYNTACTIC, p->t.linha, p->t.lexema, "fator invalido");
        exit(1);
    }

    return p->t;
}

// Analisa chamada de função.
token analisa_chamada_funcao(parser *p) {
    if (strcmp(p->t.simbolo, "sidentificador") == 0) {

        token_free(&p->t);
        p->t = lexer(p->file, p->out);
    }
    return p->t;
}

// Analisa chamada de procedimento e gera código de chamada.
token analisa_chamada_procedimento(parser *p, int end) {

    char *endereco = convert_integer_to_string(end);  //endereco aqui e rotulo
    instrucao("chamada", endereco, "proc");
    free(endereco);

    return p->t;
}

// Analisa um bloco (variáveis, sub-rotinas, comandos) e gera código de alocação/desalocação.
token analisa_bloco(parser *p) {

    char *endereco;
    char *endereco2;
    int counter_var = 0;
    int end_aux_var = endereco_var;

    p->t = lexer(p->file, p->out);

    p->t = analisa_et_variaveis(p,&counter_var);

    if(counter_var > 0){
        endereco = convert_integer_to_string(end_aux_var);
        endereco2 = convert_integer_to_string(counter_var);
        instrucao("var", endereco, endereco2);        //aloca as variaveis conforme a quantidade  
        free(endereco);
        free(endereco2);                             
    }
    
    p->t = analisa_subrotinas(p);
    p->t = analisa_comandos(p);

    if (counter_var > 0){
        endereco = convert_integer_to_string(end_aux_var);
        endereco2 = convert_integer_to_string(counter_var);
        instrucao("var_dalloc", endereco, endereco2);                   //terminando o bloco desaloca as variaveis
        free(endereco);
        free(endereco2);                                 
    }
    
    return p->t;
}

// Função principal: inicializa, abre arquivos, executa análise sintática e gera código.
int main(int argc, char **argv){

    sp_parser = initialize_stack();
    parser p; // Nova struct para o parser (melhor organizacao e clareza do codigo)
    new_program_code();

    char file_name[512];

    if (argc > 1) {
        // usar argumento passado pela linha de comando
        strncpy(file_name, argv[1], sizeof(file_name)-1);
        file_name[sizeof(file_name)-1] = '\0';
    } else {
        printf("Digite o nome do arquivo a ser analisado: ");
        if (scanf("%511s", file_name) != 1) {
            report_error(ERR_INFO, 0, 0, "nome de arquivo invalido na entrada padrao.");
            return 1;
        }
    }

    // --- Abertura do arquivo fonte ---
    p.file = fopen(file_name, "r");
    if (p.file) {
        fprintf(stderr, "Arquivo aberto com sucesso: %s\n", file_name);
    } else {
        report_error(ERR_INFO, 1, NULL, "Erro ao abrir arquivo");
        exit(1);
    }

    lexer_init(p.file);

    // --- Criação do arquivo de saída ---
    struct stat st;
    if (stat("output", &st) != 0) {
        MKDIR("output");
    }
    if (stat("output/tokens", &st) != 0) {
        MKDIR("output/tokens");
    }

    /* agora safe fopen */
    p.out = fopen("output/tokens/tabela_tokens.txt", "w");
    if (!p.out) {
        fprintf(stderr,"Erro ao criar arquivo da tabela de tokens!\n");
        fclose(p.file);
        exit(1);
    }

    fprintf(p.out, "Tabela de Tokens:\n");
    fprintf(p.out, "%-21s | %-20s\n", "Lexema", "Símbolo");
    fprintf(p.out, "----------------------+-----------------------\n");

    // --- Início da análise ---

    // "def" rotulo integer -> essa parte a gente declarou no semantico
    //      declarado como variavel global

    instrucao("inicia_prog","","");

    p.t = lexer(p.file, p.out);
    if (strcmp(p.t.simbolo, "sprograma") == 0) {
        token_free(&p.t);
        p.t = lexer(p.file, p.out);

        if (strcmp(p.t.simbolo, "sidentificador") == 0) {
            insere_tabela(p.t.lexema, "nomedeprograma", ' ', 0, &endereco_var, 0); // insere nome do programa na tabela de simbolos
            token_free(&p.t);
            p.t = lexer(p.file, p.out);

            if (strcmp(p.t.simbolo, "sponto_virgula") == 0) {
                token_free(&p.t);
                p.t = analisa_bloco(&p);

                if (strcmp(p.t.simbolo, "sponto") == 0) {
                    token_free(&p.t);
                    p.t = lexer(p.file, p.out);
                    char ch = fgetc(p.file);
                    if (ch == EOF) {
                        // imprimir_tabela_simbolos();     // apenas para testes
                        // fprintf(stderr,"\nSucesso\n");
                    } else {
                        report_error(ERR_SYNTACTIC, p.t.linha, p.t.lexema, "token inesperado");
                        exit(1);
                    }
                    token_free(&p.t);
                } else {
                    report_error(ERR_SYNTACTIC, p.t.linha, p.t.lexema, "token inesperado");
                    exit(1);
                }
            } else {
                report_error(ERR_SYNTACTIC, p.t.linha, p.t.lexema, "token inesperado");
                exit(1);
            }
        } else {
            report_error(ERR_SYNTACTIC, p.t.linha, p.t.lexema, "token inesperado");
            exit(1);
        }
    } else {
        report_error(ERR_SYNTACTIC, p.t.linha, p.t.lexema, "token inesperado");
        exit(1);
    }

    fclose(p.file);
    fclose(p.out);

    instrucao("finaliza_prog","","");
    return 0;
}