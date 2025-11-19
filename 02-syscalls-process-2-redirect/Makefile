SRC_DIR := src
INC_DIR := include
OBJ_DIR := build
BIN_DIR := bin
GCOV_DIR := gcov

CC := gcc
CC_SANITIZER_FLAGS := -fsanitize=address -fsanitize=undefined
CFLAGS := -Wall -Wextra -Wfloat-equal -Wundef -Wno-unused-variable -O3 -c
LDFLAGS := ${CC_SANITIZER_FLAGS} -I ./${INC_DIR}/

REDIRECT_SRC := $(SRC_DIR)/main.c
PRINT_SRC := $(SRC_DIR)/print.c

REDIRECT_OBJ := $(OBJ_DIR)/main.o
PRINT_OBJ := $(OBJ_DIR)/print.o

.PHONY: all clean run run-redirect run-print

all: $(BIN_DIR)/redirect $(BIN_DIR)/print

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -I ./${INC_DIR}/ -o $@

$(BIN_DIR)/redirect: $(REDIRECT_OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(BIN_DIR)/print: $(PRINT_OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

$(BIN_DIR) $(OBJ_DIR) $(GCOV_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(GCOV_DIR)

.PHONY: lint
lint:
	clang-format --Werror ${REDIRECT_SRC} ${PRINT_SRC}

# run helpers
run-redirect: $(BIN_DIR)/redirect
	./$(BIN_DIR)/redirect $(ARGS)

run-print: $(BIN_DIR)/print
	./$(BIN_DIR)/print $(ARGS)
