# Compiler
CC = gcc
CFLAGS = -Wall -Werror -DMG_ENABLE_LINES
LDFLAGS = -lsqlite3 -lcjson
POSTLINK = install_name_tool -change libcjson.dylib.1.7.18 /usr/local/lib/libcjson.dylib.1.7.18 $(TARGET)
TARGET = output
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)
	$(POSTLINK)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
