# Makefile template originally supplied by labelled author.
# All necessary credit for original template goes to the below stated author.
# Author: Liam Echlin
# User: Kameryn Knight
#
# Modifications made by stated User:
# 	- Supports dynamic binary build directory (Placed alongside object files(*.o), or Makefile directory.)
#	- Supports library linking from multiple directories with the use of 'H_FILE_DIRS'.

.PHONY: all clean debug

TARGET_EXEC ?= a.out
EXEC_IN_BUILD ?= "false"

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src
H_FILE_DIRS ?= ./include

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(H_FILE_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
CFLAGS += -Wall -Wextra -Wpedantic -Waggregate-return \
		  -Wwrite-strings -Wvla -Wfloat-equal \
          -std=c99 -D_GNU_SOURCE
LDFLAGS += -lm -lpthread

# Determine path for executable
_EXEC_LOC ?= $(TARGET_EXEC)

# Place executable in 'BUILD_DIR' if desired
ifeq ($(EXEC_IN_BUILD), "true")
    _EXEC_LOC = $(BUILD_DIR)/$(TARGET_EXEC)
endif

# Create executable
$(_EXEC_LOC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Capture C source files
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INC_FLAGS) -c $< -o $@


# Clean command
clean:
	$(RM) -r $(BUILD_DIR)
ifeq ($(EXEC_IN_BUILD), "false")
	$(RM) $(TARGET_EXEC)
endif

# Debug command
debug: CFLAGS += -g
debug: $(_EXEC_LOC)

-include $(DEPS)

MKDIR_P ?= mkdir -p
