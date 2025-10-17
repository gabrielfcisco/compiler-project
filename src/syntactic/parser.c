#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../lexical/lexer.h"
#include "../lexical/token.h"

token analisa_bloco(FILE* file, FILE* out);
token analisa_comandos_simples(FILE* file, FILE* out, token t);
token analisa_comandos(FILE* file, FILE* out, token t);
token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t);
token analisa_atribuicao(FILE* file, FILE* out, token t);
token analisa_escreva(FILE* file, FILE* out, token t);
token analisa_leia(FILE* file, FILE* out, token t);
token analisa_enquanto(FILE* file, FILE* out, token t);
token analisa_se(FILE* file, FILE* out, token t);
token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t);
token analisa_expressao_simples(FILE* file, FILE* out, token t);
token analisa_expressao(FILE* file, FILE* out, token t);
token analisa_termo(FILE* file, FILE* out, token t);
token analisa_chamada_funcao(FILE* file, FILE* out, token t);
token analisa_chamada_procedimento(FILE* file, FILE* out, token t);
token analisa_fator(FILE* file, FILE* out, token t);


void imprimir_token(token t) {
    printf("--- Conteúdo do Token ---\n");
    printf("Lexema: %s\n", t.lexema);
    printf("Símbolo: %s\n", t.simbolo);
    printf("Linha: %d\n", t.linha);
    printf("Erro: %d (%s)\n", t.erro, t.erro == 0 ? "ok" : "erro");
    printf("-------------------------\n\n");
}

token analisa_tipo(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sinteiro") != 0 && strcmp(t.simbolo, "sbooleano") != 0){
        // printf("\n%s", t.simbolo);
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }else{
        coloca_tipo_tabela(t.lexema);                               //semantico
    }
    return lexer(file, out);
}

token analisa_variaveis(FILE* file, FILE* out, token t){
    while(strcmp(t.simbolo, "sdoispontos") != 0){
        if(strcmp(t.simbolo, "sidentificador") == 0){
            
            if(pesquisa_duplicacvar_tabela(t.lexema) == 0){ //0 = false and 1 == true  //semantico
                insere_tabela(t.lexema,"variavel","","");                              //semantico
                t = lexer(file, out);
                if(strcmp(t.simbolo, "svirgula") == 0 || strcmp(t.simbolo, "sdoispontos") == 0){
                    if(strcmp(t.simbolo, "svirgula") == 0){
                        t = lexer(file, out);
                        // printf("\naqui");
                        if(strcmp(t.simbolo, "sdoispontos") == 0){
                            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                            exit(1);
                        }
                    }
                }else{
                    printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO semantico: linha %d, token: %s", t.linha, t.lexema);   //semantico
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }

    t = lexer(file, out);
    return analisa_tipo(file, out, t);
}

token analisa_et_variaveis(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "svar") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sidentificador") == 0){
            // printf("\n%s", t.lexema);
            while(strcmp(t.simbolo, "sidentificador") == 0){
                t = analisa_variaveis(file, out, t);
                if(strcmp(t.simbolo, "sponto_virgula") == 0){
                    t = lexer(file, out);
                } else {
                    printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                    exit(1);
                }
            }
        }else{
            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }
    return t;
}

token analisa_declaracao_procedimento(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    char nivel = 'L'; //semantico    (marca o novo gatilho)
    if(strcmp(t.simbolo, "sidentificador") == 0){

        if (pesquisa_declproc_tabela(t.lexema) == 0 ){
            insere_tabela(t.lexema,"procedimento",nivel,""/*rotulo*/); // semantico ,  adicionar rotulo quando faer geração de codigo
            t = lexer(file, out);
            if(strcmp(t.simbolo, "sponto_virgula") == 0){
                t = analisa_bloco(file, out);
            }else{
                printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("ERRO semantico: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
        
    }else{
        printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }
    desempilha_ou_voltanivel(); // semantico
    return t;
}

token analisa_declaracao_funcao(FILE* file, FILE* out, token t,TABSIMB *tabela_simbolos, int *pc){
    t = lexer(file, out);
    char nivel = 'L'; //semantico

    if(strcmp(t.simbolo, "sidentificador") == 0){
        if(pesquisa_declfunc_tabela(t.lexema) == 0){
            insere_tabela(t.lexema,"",nivel,""/*rotulo*/); // semantico
            t = lexer(file, out);
            if(strcmp(t.simbolo, "sdoispontos") == 0){
                t = lexer(file, out);
                if((strcmp(t.simbolo, "sinteiro") == 0) || strcmp(t.simbolo, "sbooleano") == 0){
                    if(t.simbolo == "sinteiro"){ 
                        strcpy(tabela_simbolos[*pc].tipo, "funcao inteiro");  //semantico
                    }else{
                        strcpy(tabela_simbolos[*pc].tipo, "funcao booleano"); //semantico
                    }
                    t = lexer(file, out);
                    if(strcmp(t.simbolo, "sponto_virgula") == 0){
                        t = analisa_bloco(file, out);
                    }
                } else {
                    printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO: linha %d, token: %s", t.linha, t.lexema); // semantico
                exit(1);
            }
        }else{
            printf("\nERRO semantico: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }else{
        printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }

    desempilha_ou_voltanivel();
    return t;
}

token analisa_subrotinas(FILE* file, FILE* out, token t,TABSIMB *tabela_simbolos,int *pc){

    while((strcmp(t.simbolo, "sprocedimento") == 0) || (strcmp(t.simbolo, "sfuncao") == 0)) {
        if(strcmp(t.simbolo, "sprocedimento") == 0) {
            t = analisa_declaracao_procedimento(file, out, t);
        } else {
            t = analisa_declaracao_funcao(file, out, t, tabela_simbolos, pc);
        }
        // printf("\n%s", t.lexema);
        if(strcmp(t.simbolo, "sponto_virgula") == 0){
            t = lexer(file, out);
        }else{
            printf("ERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }

    return t;
}

token analisa_comandos_simples(FILE* file, FILE* out, token t){
    // printf("\nanalisando comando simples");
    if(strcmp(t.simbolo, "sidentificador") == 0){
        t = analisa_atrib_chprocedimento(file, out, t);
    }else if (strcmp(t.simbolo, "sse") == 0){
        t = analisa_se(file, out, t);
    } else if (strcmp(t.simbolo, "senquanto") == 0){
        t = analisa_enquanto(file, out, t);
    } else if (strcmp(t.simbolo, "sleia") == 0){
        t = analisa_leia(file, out, t);
    } else if (strcmp(t.simbolo, "sescreva") == 0){
        t = analisa_escreva(file, out, t);
    } else {
        t = analisa_comandos(file, out, t);
    }

    return t;
}

token analisa_atrib_chprocedimento(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "satribuicao") == 0){
        // imprimir_token(t);
        t = analisa_atribuicao(file, out, t);
    } else{
        t = analisa_chamada_procedimento(file, out, t);
    }
    return t;
}


token analisa_se(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    t = analisa_expressao(file, out, t);
    if (strcmp(t.simbolo, "sentao") == 0){
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
        if (strcmp(t.simbolo, "ssenao") == 0){
            t = lexer(file, out);
            t = analisa_comandos_simples(file, out, t);
        }
    } else {
        printf("\nERRO entao: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_enquanto(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    t = analisa_expressao(file, out, t);
    if (strcmp(t.simbolo, "sfaca") == 0){
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
    } else {
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_leia(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sabre_parenteses") == 0){
        t = lexer(file, out);
        if (strcmp(t.simbolo, "sidentificador") == 0){

            if(pesquisa_declvar_tabela(t.lexema) == 0 ){ // 0 = false , 1 = true  //semantico
                t = lexer(file, out);
                if (strcmp(t.simbolo, "sfecha_parenteses") == 0){
                    t = lexer(file, out);
                }else {
                    printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO semantico: linha %d, token: %s", t.linha, t.lexema);
                exit(1);
            }
        }else {
            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }else {
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_escreva(FILE* file, FILE* out, token t){
    t = lexer(file, out);
    if (strcmp(t.simbolo, "sabre_parenteses") == 0){
        t = lexer(file, out);
        if (strcmp(t.simbolo, "sidentificador") == 0){
            if (pesquisa_declvarfunc_tabela(t.lexema) == 0){ // 0 == false ;; 1 = true //semantico
                t = lexer(file, out);
                if (strcmp(t.simbolo, "sfecha_parenteses") == 0){
                    t = lexer(file, out);
                }else {
                    printf("\nERRO: token sfecha_parenteses esperado\nLinha %d, Token: %s", t.linha, t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO: token sidentificador esperado\n Linha %d, token: %s", t.linha, t.lexema);
                exit(1);
            }
        }else {
            printf("\nERRO semantico: Linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }else {
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

// token analisa_atribuicao(FILE* file, FILE* out, token t){ //o problema está nessa aqui Enzo, o token é satribuicao, ele tá tratando como
//     //se atribuicao fossem dois tokens, : e  =
//     t = lexer(file, out);
//     imprimir_token(t);
//     if (strcmp(t.simbolo, "sdoispontos") == 0){
//         t = lexer(file, out);
//         if(strcmp(t.simbolo, "sigual") == 0){
//             t = lexer(file, out);
//             t = analisa_expressao(file, out, t);
//         }else {
//             printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
//             exit(1);
//         }
//     } else {
//         printf("oi");
//         printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
//         exit(1);
//     }
//     return t;
// }

token analisa_atribuicao(FILE* file, FILE* out, token t){
    if (strcmp(t.simbolo, "satribuicao") == 0) {
        t = lexer(file, out);
        t = analisa_expressao(file, out, t);
    } else {
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }
    return t;
}


token analisa_expressao(FILE* file, FILE* out, token t){
    t = analisa_expressao_simples(file, out, t);
    if(strcmp(t.simbolo, "smaior") == 0 || strcmp(t.simbolo, "smaiorig") == 0 ||
           strcmp(t.simbolo, "smenor") == 0 || strcmp(t.simbolo, "smenorig") == 0 ||
           strcmp(t.simbolo, "sdif") == 0){
        t = lexer(file, out);
        t = analisa_expressao_simples(file, out, t);
    }
    return t;
}

token analisa_expressao_simples(FILE* file, FILE* out, token t){
    if (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0){
        t = lexer(file, out);
    }
    t = analisa_termo(file, out, t);
    while (strcmp(t.simbolo, "smais") == 0 || strcmp(t.simbolo, "smenos") == 0 || strcmp(t.simbolo, "sou") == 0){
        t = lexer(file, out);
        t = analisa_termo(file, out, t);
    }

    return t;
}

token analisa_comandos(FILE* file, FILE* out, token t){

    if(strcmp(t.simbolo, "sinicio") == 0){
        t = lexer(file, out);
        t = analisa_comandos_simples(file, out, t);
        while (strcmp(t.simbolo, "sfim") != 0){
            if (strcmp(t.simbolo, "sponto_virgula") == 0){
                t = lexer(file, out);
                if(strcmp(t.simbolo, "sfim") != 0){
                    t = analisa_comandos_simples(file, out, t);
                }
            } else {
                printf("\nERRO: token sponto_virgula esperado\nLinha %d, Token: %s", t.linha, t.lexema);
                exit(1);
            }
        }
        t = lexer(file, out);
    } else {
        printf("\nERRO: token sinicio esperado\nLinha %d, Token: %s", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_termo(FILE* file, FILE* out, token t){
    t = analisa_fator(file, out, t); // Primeiro analisa um fator

    while(strcmp(t.simbolo, "smult") == 0 ||
          strcmp(t.simbolo, "sdiv") == 0 ||
          strcmp(t.simbolo, "se") == 0){

        t = lexer(file, out);

        t = analisa_fator(file, out, t);
    }

    return t;
}

token analisa_fator(FILE* file, FILE* out, token t, int nivel,TABSIMB *tabela_simbolos){

    if(strcmp(t.simbolo, "sidentificador") == 0){
        int ind;

        if (pesquisa_tabela(t.lexema,nivel,&ind) == 1){ // semantico, verifica se é true e atribui o valor de ind no endereço passado para a função

            if (strcmp(tabela_simbolos[ind].tipo, "funcao inteiro") || strcmp(tabela_simbolos[ind].tipo == "funcao booleano")){
                t = analisa_chamada_funcao(file, out, t);
            }else{
                t = lexer(file,out);
            }
        }else{
            printf("\nERRO semantico:       Linha %d, Token: %s", t.linha, t.lexema);
            exit(1);
        }
        
    }
    else if(strcmp(t.simbolo, "snumero") == 0){
        t = lexer(file, out);
    }
    else if(strcmp(t.simbolo, "snao") == 0){
        t = lexer(file, out);
        t = analisa_fator(file, out, t,nivel,tabela_simbolos);
    }
    else if(strcmp(t.simbolo, "sabre_parenteses") == 0){
        t = lexer(file, out);
        t = analisa_expressao(file, out, t);
        if(strcmp(t.simbolo, "sfecha_parenteses") == 0){
            t = lexer(file, out);
        } else {
            printf("\nERRO: esperado fecha parênteses\nLinha %d, Token: %s", t.linha, t.lexema);
            exit(1);
        }
    }
    else if(strcmp(t.lexema, "verdadeiro") == 0 ||
            strcmp(t.lexema, "falso") == 0){
        t = lexer(file, out);
    }
    else{
        printf("\nERRO: fator inválido\nLinha %d, Token: %s", t.linha, t.lexema);
        exit(1);
    }
    return t;
}

token analisa_chamada_funcao(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sidentificador") == 0){
        t = lexer(file, out);
    }
    return t;
}

token analisa_chamada_procedimento(FILE* file, FILE* out, token t){
    if(strcmp(t.simbolo, "sidentificador") == 0){
        t = lexer(file, out);
    }
    return t;
}


token analisa_bloco(FILE* file, FILE* out,TABSIMB *tabela_simbolos, int *pc){
    token t = lexer(file, out);
    t = analisa_et_variaveis(file, out, t);
    t = analisa_subrotinas(file, out, t,tabela_simbolos,pc);
    t = analisa_comandos(file, out, t);
    return t;
}

void insere_tabela(char *lexema, char *tipo, char *escopo, char *mem){
    //todo
    // inserir no topo da tabela de simbolos
    //inserir na tabela de simbolos
}

int pesquisa_duplicacvar_tabela(char *lexema){ //boolean
    //todo:
    //verificar se o lexema do identificador nao está duplicada na tabela de smbolos
}

void coloca_tipo_tabela(char *lexema){
    //todo:
    //não sei mt o que fazer aqui, dar uma olhada melhor kk
    // pagina 47 do livro dele
}

int pesquisa_declvar_tabela(char *lexema){  //boolean
    //todo:
    // no caso, se nao estiver duplicado =  false
}

int pesquisa_declvarfunc_tabela(char *lexema){ // boolean
    //todo
}

int pesquisa_declproc_tabela(char *lexema){
    //todo: pagina 50 livro dele
}

desempilha_ou_voltanivel(){
    // todo : pagina 50 do livro dele
}

/* todos os booleans são nesse formato:
     0 = falso
     1 =  true
*/

#define TAMANHO_TAB_SIMB 100 // EXEMPLO SÓ PRA NAO DAR ERRO

typedef struct {
    char lexema[10]; //nome do identificador
    char escopo[10]; //nivel de declaração
    char tipo[10]; //padrao do identificador
    char memoria[10]; //endereço de memoria alocada
}TABSIMB;


int main(){
    TABSIMB tabela_simbolos[TAMANHO_TAB_SIMB];// lista de struct de para simbolos da tabela
    int pc = 0; //ponteiro para o topo da pilha da tabela de simbolos (= 0 porque inicia o ponteiro em 0 da pilha)
    //somente para ilustrar, depois troca pelo certo

    //repare que tem um tratamento de ponteiro com TABSIMB e pc, analisar antes de alterações


    int rotulo = 1;

    FILE* file;

    char file_name[100];

    printf("Digite o nome do arquivo a ser analisado: ");
    scanf("%s", file_name);

    file = fopen(file_name, "r");

    if (file != 0 ){
        printf("Arquivo aberto com sucesso!\n");
    } else {
        fprintf(stderr, "%s\n", "Erro ao abrir arquivo!");
    }

    lexer_init(file);

    FILE* out = fopen("output/tokens/tabela_tokens.txt", "w");
    if (!out) {
        printf("Erro ao criar arquivo da tabela de tokens!\n");
    } else {
        printf("Arquivo tabela_tokens criado com sucesso!\n");
    }
    fprintf(out, "Tabela de Tokens:\n");
    fprintf(out, "%-21s | %-20s\n","Lexema", "Símbolo");
    fprintf(out, "----------------------+-----------------------\n");

    token t = lexer(file, out);
    if(strcmp(t.simbolo, "sprograma") == 0){
        t = lexer(file, out);
        if(strcmp(t.simbolo, "sidentificador") == 0){
            insere_tabela(t.lexema, "nomedeprograma","",""); //semantico
            t = lexer(file, out);
            if(strcmp(t.simbolo, "sponto_virgula") == 0){
                t = analisa_bloco(file, out,tabela_simbolos, &pc);
                if(strcmp(t.simbolo, "sponto") == 0){
                    t = lexer(file, out);
                    char ch = fgetc(file);
                    if(ch == EOF){
                        printf("\nSucesso");
                    }else{
                        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                        exit(1);
                    }
                }else{
                    printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                    exit(1);
                }
            }else{
                printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
                exit(1);
            }
        }else{
            printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
            exit(1);
        }
    }else{
        printf("\nERRO: linha %d, token: %s", t.linha, t.lexema);
        exit(1);
    }

    return 0;
}