CC=clang
CFLAGS=-g -Wall
IFLAGS=-I/usr/local/include/SDL3
LFLAGS=-L/usr/local/lib -lSDL3
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
	$(CC) $(CFLAGS) $(IFLAGS) $(LFLAGS) $^ -o $(BINDIR)/$@

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

$(TEST)/bin/%: $(TEST)/%.c
	mkdir -p $(TEST)/bin
	$(CC) $(CFLAGS) -I/opt/homebrew/include -L/opt/homebrew/lib $< $(TESTOBJS) -o $@ -lcriterion

test: $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test ; done

clean:
	rm -rf $(BINDIR)/* $(OBJ)/* $(TEST)/bin/* 
