CXX := g++
FLAGS := -std=c++11 -Wall -pthread
INCLUDE := include
BUILD := build
SRC := src
BIN := bin

all:$(BIN)/tcptest $(BIN)/udptest

$(BIN)/tcptest: $(BUILD)/tcptest.o $(BUILD)/utils.o $(BUILD)/tcp.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I$(INCLUDE) -o $@ $^

$(BIN)/udptest: $(BUILD)/udptest.o $(BUILD)/utils.o $(BUILD)/udp.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I$(INCLUDE) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p ./$(BUILD)
	$(CXX) $(FLAGS) -I$(INCLUDE) -c -o $@ $<

clean:
	rm -rfv $(BIN)/* $(BUILD)/*
