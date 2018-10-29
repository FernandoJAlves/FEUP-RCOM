SRC = application_layer.c data_link.c  stateMachine.c reader.c writer.c utilities.c


all:
	gcc $(SRC) -Wall -o run

debug:
	gcc -g $(SRC) -Wall -o run

clean:
	rm run
