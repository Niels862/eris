CC          := gcc
CCSTD       := c99

SRC_DIR     := src
BUILD_DIR   := build

CTK_DIR     := libs/ctk
CTK_LIB     := $(CTK_DIR)/libctk.a
CTK_INCLUDE := $(CTK_DIR)/include

EXEC        := eris
MODE        ?= debug

ifeq ($(MODE),release)
  CCFLAGS := -O3 -DNDEBUG
else ifeq ($(MODE),debug)
  CCFLAGS := -O2 -g
else
  $(error Unknown build mode '$(MODE)'; use MODE=release or MODE=debug)
endif

CCFLAGS += -Wall -Wextra -Werror -pedantic -std=$(CCSTD)
CFLAGS  := -MMD -MP -I$(CTK_INCLUDE)

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all release debug clean

all: $(EXEC)

release:
	$(MAKE) MODE=release

debug:
	$(MAKE) MODE=debug

# Executable:
$(EXEC): $(OBJS) $(CTK_LIB)
	$(CC) $(CCFLAGS) $(OBJS) -L$(CTK_DIR) -lctk -o $@

# Object files:
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CCFLAGS) -c $< -o $@

# Build libctk if missing:
$(CTK_LIB):
	@echo "Building libctk.a..."
	$(MAKE) -C $(CTK_DIR) MODE=$(MODE)

clean:
	rm -rf $(BUILD_DIR) $(EXEC)
	$(MAKE) -C $(CTK_DIR) clean

-include $(DEPS)
