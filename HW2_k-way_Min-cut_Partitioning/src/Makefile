CXX      := g++
CXXFLAGS := -O3 -std=c++17
SRC      := main.cpp
BIN_DIR  := ../bin
TARGET   := $(BIN_DIR)/hw2

OUT_DIR  := ../output
TEST_DIR := ../testcase
VERIFIER := ../verifier/verify

.PHONY: all clean test test_public1 test_public2

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

$(OUT_DIR):
	@mkdir -p $(OUT_DIR)

test_public1: $(TARGET) $(OUT_DIR)
	@echo "========================================"
	@echo "[TEST] Starting Public Case 1"
	@echo "========================================"
	@echo "  [RUN] Running 2-way partition..."
	$(TARGET) $(TEST_DIR)/public1.txt $(OUT_DIR)/public1.2way.out 2
	@echo "  [VERIFY] Verifying 2-way result..."
	@$(VERIFIER) $(TEST_DIR)/public1.txt $(OUT_DIR)/public1.2way.out 2
	@echo ""
	@echo "  [RUN] Running 4-way partition..."
	$(TARGET) $(TEST_DIR)/public1.txt $(OUT_DIR)/public1.4way.out 4
	@echo "  [VERIFY] Verifying 4-way result..."
	@$(VERIFIER) $(TEST_DIR)/public1.txt $(OUT_DIR)/public1.4way.out 4
	@echo "[PASS] Public Case 1 Finished."
	@echo ""

test_public2: $(TARGET) $(OUT_DIR)
	@echo "========================================"
	@echo "[TEST] Starting Public Case 2"
	@echo "========================================"
	@echo "  [RUN] Running 2-way partition..."
	$(TARGET) $(TEST_DIR)/public2.txt $(OUT_DIR)/public2.2way.out 2
	@echo "  [VERIFY] Verifying 2-way result..."
	@$(VERIFIER) $(TEST_DIR)/public2.txt $(OUT_DIR)/public2.2way.out 2
	@echo ""
	@echo "  [RUN] Running 4-way partition..."
	$(TARGET) $(TEST_DIR)/public2.txt $(OUT_DIR)/public2.4way.out 4
	@echo "  [VERIFY] Verifying 4-way result..."
	@$(VERIFIER) $(TEST_DIR)/public2.txt $(OUT_DIR)/public2.4way.out 4
	@echo "[PASS] Public Case 2 Finished."
	@echo ""

test: test_public1 test_public2
	@echo "========================================"
	@echo "[INFO] All Tests Completed Successfully."
	@echo "========================================"