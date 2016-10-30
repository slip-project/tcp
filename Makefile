CXX := g++
FLAGS := -std=c++11 -Wall -pthread
INCLUDE := include
BUILD := build
SRC := src
TEST := test
BIN := bin


all: $(BIN)/udptest $(BIN)/tcptest


$(BIN)/tcptest: $(BUILD)/$(TEST)/tcptest.o $(BUILD)/utils.o $(BUILD)/tcp.o $(BUILD)/timeout.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BIN)/udptest: $(BUILD)/$(TEST)/udptest.o $(BUILD)/utils.o $(BUILD)/udp.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BUILD)/$(TEST)/%.o: $(TEST)/%.cpp
	@mkdir -p ./$(BUILD)/$(TEST)
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p ./$(BUILD)
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<


clean:
	rm -rfv $(BIN)/* $(BUILD)/*
