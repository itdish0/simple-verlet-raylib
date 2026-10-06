CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS = -Llib -lraylib -lopengl32 -lgdi32 -lwinmm

SRCS = main.c physics.c loader.c

# Default target when you just type 'make'
all: debug

# 1. Debug Target: Keeps console, adds -g flag for gdb/lldb, disables optimizations
debug: CFLAGS += -g -O0
debug: game.exe

# 2. Release Target: Hides console, enables -O2 optimizations
release: CFLAGS += -O2
release: LDFLAGS += -Wl,-subsystem,windows
release: game.exe

# Compilation rule
game.exe: main.c
	$(CC) $(CFLAGS) $(SRCS) -o game.exe $(LDFLAGS)

run: game.exe
	./game.exe

clean:
	rm -f game.exe