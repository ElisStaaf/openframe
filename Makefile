CC := gcc
# Removed -g from CFLAGS
CFLAGS := -I./include -Wall -Wextra -Werror -Wpedantic
LDFLAGS := -lpthread -lsqlite3
SRC_DIR := src
BIN_DIR := bin
OBJS_DIR := $(BIN_DIR)/objs
VIEWS_DIR := views
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJS_DIR)/%.o,$(SRC_FILES))
TARGET := $(BIN_DIR)/server

# Default debug flags are empty
DEBUG_FLAGS := 
RELEASE_FLAGS :=

# This target is for release builds
all: DEBUG_FLAGS := 
all: RELEASE_FLAGS := -Ofast
all: $(TARGET) copy_views

# This target is for debug builds
debug: DEBUG_FLAGS := -g -O0
debug: RELEASE_FLAGS :=
debug: $(TARGET) copy_views

$(TARGET): $(OBJ_FILES)
	$(CC) $(DEBUG_FLAGS) $^ -o $@ $(LDFLAGS)

$(OBJS_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJS_DIR)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(BIN_DIR) $(OBJS_DIR):
	mkdir -p $@

copy_views: | $(BIN_DIR)
	cp -r $(VIEWS_DIR) $(BIN_DIR)
	mkdir -p $(BIN_DIR)/sessions
	mkdir -p $(BIN_DIR)/database

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean copy_views debug
