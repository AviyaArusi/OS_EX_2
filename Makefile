CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: stnc

stnc: stnc.c
	$(CC) $(CFLAGS) stnc.c -o stnc

clean:
	rm -f stnc
