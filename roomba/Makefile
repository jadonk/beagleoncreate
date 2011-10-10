CFLAGS 	= -O2 -Wall
LDFLAGS = -L. -lpthread
CPP	= g++

SRCS 	= main.cpp \
	  Thread.cpp

OBJS 	= main.o \
	  Thread.o

EXECUTABLE = main

all	: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) -o $@ $*

main.o	: main.cpp
	$(CPP) $(CFLAGS) -c $^

Thread.o : Thread.h Thread.cpp
	$(CPP) $(CFLAGS) -c $^

$(OBJS)	: Thread.h Packet.h

clean	:
	rm -rf main *.o
	rm -rf *.*~
	rm -rf *.h.gch
