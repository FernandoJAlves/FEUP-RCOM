SRC = noncanonical

all:
	gcc $(SRC).c -Wall -o run

clean:
	rm run