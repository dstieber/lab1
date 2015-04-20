LIB     = ./libggfonts.so
LFLAGS  = $(LIB) -lrt -lX11 -lGL -pthread -lm

all: lab1

lab1: lab1.cpp
	g++ lab1.cpp -Wall -Wextra $(LFLAGS) -olab1

clean:
	rm -f lab1
	rm -f *.o

