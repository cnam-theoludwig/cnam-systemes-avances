EXEC=threaded_sort
SRC_DIR=src
INC_DIR=include
OBJ_DIR=build
BIN_DIR=bin
DOC_DIR=doc
GCOV_DIR=gcov

CC_SANITIZER_FLAGS=-fsanitize=address -fsanitize=undefined
# CC_SANITIZER_FLAGS =

CC=gcc
PTHREAD_FLAGS = -pthread
# CFLAGS=-Wall -Wextra -Wfloat-equal -Wundef -Wno-unused-variable -std=c17 -pedantic -pedantic-errors -O3 -c
CFLAGS=-Wall -Wextra -Wfloat-equal -Wundef -Wno-unused-variable -O3 -c $(PTHREAD_FLAGS)
LDFLAGS=${CC_SANITIZER_FLAGS} -I ./${INC_DIR}/ $(PTHREAD_FLAGS)
GCOVFLAGS=-O0 --coverage -Wall -g -I ./${INC_DIR}/
LCOV_REPORT=report.info

SRC=$(wildcard $(SRC_DIR)/*.c)
INC=$(wildcard $(INC_DIR)/*.h)
OBJ=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
GOBJ=$(patsubst $(SRC_DIR)/%.c, $(GCOV_DIR)/%.o, $(SRC))

GEXEC=$(EXEC).cov
AR_NAME=archive_$(EXEC).tar.gz

all: $(BIN_DIR)/$(EXEC)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

$(BIN_DIR)/$(EXEC): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

$(GCOV_DIR)/%.o: $(SRC_DIR)/%.c | $(GCOV_DIR)
	$(CC) $(GCOVFLAGS) -c $< -o $@

$(GCOV_DIR)/$(GEXEC): $(GOBJ)
	$(CC) $(GCOVFLAGS) -o $@ $^ $(PTHREAD_FLAGS)

$(BIN_DIR) $(OBJ_DIR) $(GCOV_DIR):
	mkdir --parents $@

doc:
	doxygen $(DOC_DIR)/doxygen.conf

coverage: $(GCOV_DIR)/$(GEXEC)
	./$(GCOV_DIR)/$(GEXEC) -h
	./$(GCOV_DIR)/$(GEXEC) ${ARGS}
	lcov -o $(GCOV_DIR)/$(LCOV_REPORT) -c -f -d $(GCOV_DIR)
	genhtml -o $(GCOV_DIR)/report $(GCOV_DIR)/$(LCOV_REPORT)

package: coverage doc all
	rm -rf $(AR_NAME)
	tar cvfz $(AR_NAME) ./*

clean:
	rm --recursive --force $(OBJ_DIR) $(BIN_DIR) $(DOC_DIR)/latex/ \
	$(DOC_DIR)/html/ $(GCOV_DIR)

.PHONY: run
run: $(BIN_DIR)/$(EXEC)
	./bin/$(EXEC) ${ARGS}

.PHONY: lint
lint:
	clang-format --Werror --dry-run ${SRC} ${INC}

.PHONY: doc coverage
