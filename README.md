### Para compilar:
```sh
gcc -Wall -Wextra -g src/syntactic/parser.c src/lexical/lexer.c src/semantic/semantic.c src/code_generator/generator.c -o parser
```
### Para executar:
```sh
./parser

> Depois execute seu programa teste:
tests/syntactic/sint1.txt
```
