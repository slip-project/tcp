CXX := g++
FLAGS := -std=c++11 -Wall -pthread
INCLUDE := include
BUILD := build
SRC := src
TEST := test
BIN := bin
WOCHAT := wochat_cli

<<<<<<< HEAD
all: $(BIN)/server $(BIN)/udptest $(BIN)/tcptest
=======
all: $(BIN)/udptest
>>>>>>> 14504bef3215530956660239f2359f121e5b167c

$(BIN)/server: $(BUILD)/$(WOCHAT)/server.o $(BUILD)/tcp.o $(BUILD)/udp.o $(BUILD)/utils.o 
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BIN)/tcptest: $(BUILD)/$(TEST)/tcptest.o $(BUILD)/utils.o $(BUILD)/tcp.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BIN)/udptest: $(BUILD)/$(TEST)/udptest.o $(BUILD)/utils.o $(BUILD)/udp.o
	@mkdir -p ./$(BIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(BUILD)/$(WOCHAT)/%.o: $(WOCHAT)/%.cpp
	@mkdir -p ./$(BUILD)/$(WOCHAT)
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<

$(BUILD)/$(TEST)/%.o: $(TEST)/%.cpp
	@mkdir -p ./$(BUILD)/$(TEST)
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p ./$(BUILD)
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<


clean:
	rm -rfv $(BIN)/* $(BUILD)/*
