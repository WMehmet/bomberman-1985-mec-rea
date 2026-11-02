CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -std=c99 $(shell sdl2-config --cflags)
LDFLAGS ?= $(shell sdl2-config --libs) -lm

TARGET = bomberman
SRC = src/main.c src/game.c src/graphics.c src/audio.c src/settings.c src/font.c src/scores.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
