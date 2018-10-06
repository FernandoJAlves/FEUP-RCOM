SRC = writenoncanonical
SDR = writenoncanonical.c
RVR = noncanonical.c

all:
	gcc $(SRC).c -Wall -o run

sender:
	gcc $(SDR) -Wall -o sender

receiver:
	gcc $(RVR) -Wall -o receiver

clean:
	rm run