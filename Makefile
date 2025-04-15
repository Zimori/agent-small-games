# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Directories
SRC_DIR = ./
BUILD_DIR = ./build

# Source files
TIC_TAC_TOE_SRC = tic_tac_toe/main.cpp
CONNECT4_SRC = connect4/main.cpp
TETRIS_SRC = tetris/main.cpp

# Object files
TIC_TAC_TOE_OBJ = $(BUILD_DIR)/tic_tac_toe.o
CONNECT4_OBJ = $(BUILD_DIR)/connect4.o
TETRIS_OBJ = $(BUILD_DIR)/tetris.o

# Update executable paths to be placed in the build directory
TIC_TAC_TOE_EXE = $(BUILD_DIR)/tic_tac_toe
CONNECT4_EXE = $(BUILD_DIR)/connect4
TETRIS_EXE = $(BUILD_DIR)/tetris

# Update targets to use the new paths
all: $(TIC_TAC_TOE_EXE) $(CONNECT4_EXE) $(TETRIS_EXE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TIC_TAC_TOE_OBJ): $(TIC_TAC_TOE_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CONNECT4_OBJ): $(CONNECT4_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TETRIS_OBJ): $(TETRIS_SRC) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build executables
$(TIC_TAC_TOE_EXE): $(TIC_TAC_TOE_OBJ)
	$(CXX) $< -o $@ $(LDFLAGS)

$(CONNECT4_EXE): $(CONNECT4_OBJ)
	$(CXX) $< -o $@ $(LDFLAGS)

$(TETRIS_EXE): $(TETRIS_OBJ)
	$(CXX) $< -o $@ $(LDFLAGS)

# Update clean target to remove executables from the build directory
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean tic_tac_toe connect4 tetris