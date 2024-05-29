CC=clang
CFLAGS=-g -Wall
BIN=prog
BINDIR=bin
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

TEST=tests
TESTS=$(wildcard $(TEST)/*.c)
TESTOBJS=$(filter-out $(OBJ)/main.o, $(OBJS))
TESTBINS=$(patsubst $(TEST)/%.c, $(TEST)/bin/%, $(TESTS))

all: $(BIN)

$(BIN): $(OBJS)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$@

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST)/bin/%: $(TEST)/%.c
	mkdir -p $(TEST)/bin
	$(CC) $(CFLAGS) -I/opt/homebrew/include -L/opt/homebrew/lib $< $(TESTOBJS) -o $@ -lcriterion

test: $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test ; done

clean:
	rm -rf $(BINDIR)/* $(OBJ)/* $(TEST)/bin/* 
