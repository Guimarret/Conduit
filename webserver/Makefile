CC = gcc
CFLAGS = -Wall
LDFLAGS = -lcivetweb -lpthread

SRCS = webserver.c ../logger.c
OBJS = $(SRCS:.c=.o)
TARGET = webserver

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
