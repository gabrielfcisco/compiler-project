CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = src/syntactic/parser.c \
		src/lexical/lexer.c \
		src/semantic/semantic.c \
		src/code_generator/generator.c \
		src/code_generator/instructions.c \
		src/error_UI/error.c

TARGET = parser

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
