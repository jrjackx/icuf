CC := clang
CFLAGS := -Wall -Wextra -g -std=c23

SRC := icuf.c 
BIN_DIR := $(HOME)/bin
EXE := $(BIN_DIR)/icuf

# "build the executable which is or shall be located at $(EXE), which is ~/bin/icuf.
all: $(EXE)

# "build the file in SRC, and put it in BIN_DIR, using the following instructions:
# which compiler, which compiler flags, $< is the src file, and $@ is the EXE directory.
$(EXE): $(SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

# build this directory if it doesn't exist already.
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# running "make clean" deletes the executable directory.
clean: rm -f $(EXE)
