V=1
SOURCE_DIR=src
BUILD_DIR=build
include $(N64_INST)/include/n64.mk

all: snake64.z64
.PHONY: all

OBJS = $(BUILD_DIR)/main.o

snake64.z64: N64_ROM_TITLE="snake64"
snake64.z64: $(BUILD_DIR)/snake64.dfs

$(BUILD_DIR)/snake64.dfs: $(wildcard filesystem/*)
$(BUILD_DIR)/snake64.elf: $(OBJS)

clean:
	rm -f $(BUILD_DIR)/* *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)