OPENCV_CFLAGS = `pkg-config --cflags opencv`
OPENCV_LFLAGS = `pkg-config --libs opencv` 
ARTKP_CFLAGS = `pkg-config --cflags artoolkitplus`
ARTKP_LFLAGS = `pkg-config --libs artoolkitplus`
GSTREAMER_CFLAGS=`pkg-config --cflags gstreamer-0.10 gstreamer-app-0.10`
GSTREAMER_LFLAGS=`pkg-config --libs gstreamer-0.10 gstreamer-app-0.10`

CFLAGS 	= -O2 -Wall $(OPENCV_CFLAGS) $(ARTKP_CFLAGS) $(GSTREAMER_CFLAGS)
LDFLAGS = -L. -lrt -lpthread $(OPENCV_LFLAGS) $(ARTKP_LFLAGS) $(GSTREAMER_LFLAGS)
CPP	= g++

SRCS 	= main.cpp \
	  Create.cpp 

OBJS 	= main.o \
	  Create.o 

EXECUTABLE = main

all	: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) -o $@ $*

main.o	: Packet.h Create.h main.cpp
	$(CPP) $(CFLAGS) -c $^

Create.o : Packet.h Create.h Create.cpp
	$(CPP) $(CFLAGS) -c $^

$(OBJS)	: Packet.h Create.h

clean	:
	rm -rf main *.o
	rm -rf *.*~
	rm -rf *.h.gch
