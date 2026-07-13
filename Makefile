CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -O2 -Iinclude
LDFLAGS ?=
SRCS = src/main.c src/sensors.c src/access.c src/alert.c
OBJS = $(SRCS:.c=.o)
BIN  = rfid_access

.PHONY: all clean run demo

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) sim valid 10

demo: $(BIN)
	./$(BIN) demo

clean:
	rm -f $(OBJS) $(BIN) $(BIN).exe src/*.o
