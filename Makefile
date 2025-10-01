CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
SRC_DIR = src
BUILD_DIR = bin
OUTPUT_DIR = output

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/lexical/lexer.c \
          $(SRC_DIR)/syntactic/parser.c \
          $(SRC_DIR)/semantic/semantic.c \
          $(SRC_DIR)/codegen/codegen.c \
          $(SRC_DIR)/utils/error.c

TARGET = $(BUILD_DIR)/compilador

all: $(TARGET)

$(TARGET): $(SOURCES)
    @mkdir -p $(BUILD_DIR)
    @mkdir -p $(OUTPUT_DIR)/tokens
    @mkdir -p $(OUTPUT_DIR)/symbols
    @mkdir -p $(OUTPUT_DIR)/code
    $(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
    rm -rf $(BUILD_DIR) $(OUTPUT_DIR)/*

test-lexical:
    ./$(TARGET) --lexical tests/lexical/teste_1.txt

test-all:
    @for test in tests/lexical/*.txt; do \
        echo "Testando $$test..."; \
        ./$(TARGET) $$test; \
    done