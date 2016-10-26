CXX := g++
FLAGS := -std=c++11 -Wall -g
INCLUDE := include
BUILD := build
SRC := src
BIN := bin

$(BIN)/main: $(BUILD)/main.o $(BUILD)/functionAdd.o $(BUILD)/functionSubtract.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I$(INCLUDE) -o $@ $^


$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p ./$(BUILD)
	$(CXX) $(FLAGS) -I$(INCLUDE) -c -o $@ $<
clean:
	rm -rfv $(BIN)/main $(BUILD)/*
