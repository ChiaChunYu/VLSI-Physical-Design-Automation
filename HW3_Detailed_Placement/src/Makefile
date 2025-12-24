CXX := g++
CXXFLAGS := -std=c++17 -O3 -DNDEBUG -march=native -flto -fopenmp

BIN_DIR := ../bin
OUT_DIR := ../output
VERIFIER := ../verifier/verify

SRC := main.cpp \
       parser/parser.cpp \
       design/design.cpp \
       result_writer/result_writer.cpp \
       detailed_placer/detailed_placer.cpp \
       debugger/debugger.cpp

TARGET := $(BIN_DIR)/hw3

.PHONY: all clean test test_public1 test_public2 test_public3 test_public4

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	@rm -f $(TARGET)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

test_public1: $(TARGET) $(OUT_DIR)
	@echo "========================================"
	@echo "[TEST] Starting Public Case 1"
	@echo "========================================"
	@echo "  [RUN] Executing Detailed Placer..."
	$(TARGET) ../testcase/public1.lef ../testcase/public1.def $(OUT_DIR)/public1.def
	@echo "  [VERIFY] Verifying result..."
	@$(VERIFIER) ../testcase/public1.lef ../testcase/public1.def $(OUT_DIR)/public1.def
	@echo "[PASS] Public Case 1 Finished."
	@echo ""

test_public2: $(TARGET) $(OUT_DIR)
	@echo "========================================"
	@echo "[TEST] Starting Public Case 2"
	@echo "========================================"
	@echo "  [RUN] Executing Detailed Placer..."
	$(TARGET) ../testcase/public2.lef ../testcase/public2.def $(OUT_DIR)/public2.def
	@echo "  [VERIFY] Verifying result..."
	@$(VERIFIER) ../testcase/public2.lef ../testcase/public2.def $(OUT_DIR)/public2.def
	@echo "[PASS] Public Case 2 Finished."
	@echo ""

test_public3: $(TARGET) $(OUT_DIR)
	@echo "========================================"
	@echo "[TEST] Starting Public Case 3"
	@echo "========================================"
	@echo "  [RUN] Executing Detailed Placer..."
	$(TARGET) ../testcase/public3.lef ../testcase/public3.def $(OUT_DIR)/public3.def
	@echo "  [VERIFY] Verifying result..."
	@$(VERIFIER) ../testcase/public3.lef ../testcase/public3.def $(OUT_DIR)/public3.def
	@echo "[PASS] Public Case 3 Finished."
	@echo ""

test_public4: $(TARGET) $(OUT_DIR)
	@echo "========================================"
	@echo "[TEST] Starting Public Case 4"
	@echo "========================================"
	@echo "  [RUN] Executing Detailed Placer..."
	$(TARGET) ../testcase/public4.lef ../testcase/public4.def $(OUT_DIR)/public4.def
	@echo "  [VERIFY] Verifying result..."
	@$(VERIFIER) ../testcase/public4.lef ../testcase/public4.def $(OUT_DIR)/public4.def
	@echo "[PASS] Public Case 4 Finished."
	@echo ""

test: test_public1 test_public2 test_public3 test_public4
	@echo "========================================"
	@echo "[INFO] All Tests Completed Successfully."
	@echo "========================================"