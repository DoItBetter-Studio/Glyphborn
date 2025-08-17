# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Iincludes -Iresources
LDFLAGS = -lX11 -lXext -lXrandr -lXrender -lasound -lm
TARGET = glyphborn

# Detect platform
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    PLATFORM = linux
else
    PLATFORM = unknown
endif

# Recursively find all .c files under source/, excluding Windows files on Linux
SRC = $(shell find source -name '*.c' | grep -v '_windows.c')

# Object files
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Link target
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile each .c file into .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(TARGET)
