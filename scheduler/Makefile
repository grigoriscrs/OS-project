CC = gcc
CFLAGS = -Wall
LDFLAGS = -lm

all: scheduler_v0 scheduler_v1

scheduler_v0: scheduler_v0.c
	$(CC) $(CFLAGS) -o scheduler_v0 scheduler_v0.c $(LDFLAGS)

scheduler_v1: scheduler_v1.c
	$(CC) $(CFLAGS) -o scheduler_v1 scheduler_v1.c $(LDFLAGS)

clean:
	rm -f scheduler_v0 scheduler_v1