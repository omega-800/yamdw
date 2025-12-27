.PHONY: all clean test debug release directories


INSTR ?= -fsanitize=address,leak,undefined,pointer-compare,pointer-subtract -fno-omit-frame-pointer

LDFLAGS ?= 

CFLAGS ?= -Wfatal-errors -Wall -Wextra -Werror -std=c99 -Wl,-z,execstack

SRCDIR ?= src
OBJDIR ?= .build
TARGETDIR ?= bin
SRC := $(shell find $(SRCDIR) -type f -name *.c)
OBJ := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SRC:.c=.o))
OUT ?= $(notdir $(CURDIR))

ifeq ($(CC),gcc)
	CFLAGS += -D_DEFAULT_SOURCE
endif

ifeq ($(origin CC),default)
	CC = tcc
endif

all: debug

directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(TARGETDIR)

clean:
	rm -rf $(OBJDIR)
	rm -rf $(TARGETDIR)

-include $(OBJ:.o=.d)

$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(TARGETDIR)/$(OUT) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -g $(INSTR)
debug: LDFLAGS += $(INSTR)
debug: $(OUT)

release: CFLAGS += -O3 -Ofast
release: $(OUT)
