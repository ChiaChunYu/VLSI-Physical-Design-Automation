CXX := g++
CXXFLAGS := -std=c++17 -O3 -DNDEBUG -march=native -flto 

BIN_DIR := ../bin
OUT_DIR := ../output
VERIFIER := ../verifier/verify

SRC := main.cpp \
       parser/parser.cpp \
       design/design.cpp \
       writer/writer.cpp \
       global_router/global_router.cpp \

TARGET := $(BIN_DIR)/hw4

.PHONY: all clean 

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	@rm -f $(TARGET)