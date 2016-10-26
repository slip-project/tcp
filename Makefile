CXX := g++
FLAGS := -std=c++11 -Wall -pthread
INCLUDE := include
BUILD := build
SRC := src
BIN := bin

$(BIN)/udptest: $(BUILD)/udptest.o $(BUILD)/utils.o $(BUILD)/udp.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I$(INCLUDE) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p ./$(BUILD)
	$(CXX) $(FLAGS) -I$(INCLUDE) -c -o $@ $<

clean:
	rm -rfv $(BIN)/* $(BUILD)/*
