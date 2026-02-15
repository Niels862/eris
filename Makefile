CC          := gcc
CCSTD       := c99

INC_DIR 	:= src
SRC_DIR     := src
BUILD_DIR   := build

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
CFLAGS  := -MMD -MP -I$(INC_DIR)

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all release debug clean

all: $(EXEC)

release:
	$(MAKE) MODE=release

debug:
	$(MAKE) MODE=debug

$(EXEC): $(OBJS) $(CTK_LIB)
	$(CC) $(CCFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

-include $(DEPS)
