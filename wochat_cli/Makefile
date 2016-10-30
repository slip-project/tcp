CXX := g++
FLAGS := -std=c++11 -Wall -pthread
INCLUDE := ../include
BUILD := ../build
DEMOBUILD := build
SRC := ../src
DEMOSRC := src
DEMOBIN := bin

all: $(DEMOBIN)/client_test


$(DEMOBIN)/client_test: $(DEMOBUILD)/clienttest.o $(BUILD)/utils.o $(BUILD)/udp.o
	@mkdir -p ./$(DEMOBIN)
	$(CXX) $(FLAGS) -I $(INCLUDE) -o $@ $^

$(DEMOBUILD)/%.o: $(DEMOSRC)/%.cpp
	@mkdir -p ./$(DEMOBUILD)/
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p ./$(BUILD)
	$(CXX) $(FLAGS) -I $(INCLUDE) -c -o $@ $<

clean:
	rm -rfv $(DEMOBUILD)/* $(DEMOBIN)/*
