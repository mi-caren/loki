bin/kilo: kilo.c
	$(CC) kilo.c -o bin/kilo -Wall -Wextra -pedantic -std=c99

clean:
	rm -f bin/*

