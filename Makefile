CC = gcc
CFLAGS = -Iinclude -Wall
LDFLAGS = -Llib -lraylib -lopengl32 -lgdi32 -lwinmm

SRCS = main.c physics.c loader.c

# Default target when you just type 'make'
all: debug

# 1. Debug Target: Keeps console, adds -g flag for gdb/lldb, disables optimizations
debug: CFLAGS += -g -O0
debug: verlet.exe

# 2. Release Target: Hides console, enables -O2 optimizations
release: CFLAGS += -O2
release: LDFLAGS += -Wl,-subsystem,windows
release: verlet.exe

# Compilation rule
verlet.exe: main.c
	$(CC) $(CFLAGS) $(SRCS) -o verlet.exe $(LDFLAGS)

run: verlet.exe
	./verlet.exe

clean:
	rm -f verlet.exe
