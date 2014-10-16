CC = gcc
FLAGS = -Wall

PROG = main
SRCS = main.c 

OBJS = $(SRCS:.c=.o)

main:
$(PROG) : $(OBJS)
	$(CC) $(OBJS) -o $(PROG)
	

main.o: main.c
	$(CC) -c main.c $(FLAGS)

clean:
	rm -f $(OBJS) 

