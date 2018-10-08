SRC = main.c noncanonical.c writenoncanonical.c
SDR = writenoncanonical.c
RVR = noncanonical.c

all:
	gcc $(SRC) -Wall -o run

sender:
	gcc $(SDR) -Wall -o sender

receiver:
	gcc $(RVR) -Wall -o receiver

clean:
	rm run