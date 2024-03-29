CXX := g++
OPENCV_INSTALL_DIR := /usr
#CXXFLAGS := -Iinclude -I$(OPENCV_INSTALL_DIR)/include/opencv4 -std=c++11 -Wall
LDFLAGS := -L$(OPENCV_INSTALL_DIR)/lib/ -L$(OPENCV_INSTALL_DIR)/lib/x86_64-linux-gnu/ -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lpthread 
#GTESTFLAGS := -lgtest -lgtest_main -pthread
CXXFLAGS := -Iinclude $(shell pkg-config --cflags opencv4) -std=c++17 -Wall
#LDFLAGS := -L$(OPENCV_INSTALL_DIR)/lib/ -L$(OPENCV_INSTALL_DIR)/lib/x86_64-linux-gnu/ $(shell pkg-config --libs opencv4) -lpthread
GTESTFLAGS := -lopencv_core -lopencv_imgproc -lgtest -lgtest_main -std=c++17 -pthread


SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := tests
INCLUDE_DIR := include
GTEST_DIR := lib/googletest

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(OBJ_DIR)/%_test.o)
DEPS := $(wildcard $(INCLUDE_DIR)/*.h)

.PHONY: all clean test

all: $(BIN_DIR)/go

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/go: $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%_test.o: $(TEST_DIR)/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR)/include -c $< -o $@

test: $(TEST_OBJS) obj/utils.o
	@echo "TEST_OBJS: $(TEST_OBJS)"
	$(CXX) $(LDFLAGS) $^ -o $(BIN_DIR)/test -I$(GTEST_DIR)/include -L$(GTEST_DIR)/lib $(GTESTFLAGS)

run: test
	./$(BIN_DIR)/test

clean:
	rm -f $(BIN_DIR)/* $(OBJ_DIR)/*
