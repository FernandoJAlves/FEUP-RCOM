SRC = application_layer.c data_link.c  stateMachine.c


all:
	gcc $(SRC) -Wall -o run
clean:
	rm run
