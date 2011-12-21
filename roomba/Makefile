OPENCV_CFLAGS = `pkg-config --cflags opencv`
OPENCV_LFLAGS = `pkg-config --libs opencv` 
ARTKP_CFLAGS = `pkg-config --cflags artoolkitplus`
ARTKP_LFLAGS = `pkg-config --libs artoolkitplus`
GSTREAMER_CFLAGS=`pkg-config --cflags gstreamer-0.10 gstreamer-app-0.10`
GSTREAMER_LFLAGS=`pkg-config --libs gstreamer-0.10 gstreamer-app-0.10`

CFLAGS 	= -O2 -Wall $(OPENCV_CFLAGS) $(ARTKP_CFLAGS) $(GSTREAMER_CFLAGS)
LDFLAGS = -L. -lpthread $(OPENCV_LFLAGS) $(ARTKP_LFLAGS) $(GSTREAMER_LFLAGS)
CPP	= g++

SRCS 	= main.cpp \
	  ARtag.cpp \
	  ARtagLocalizer.cpp \
	  CreateSerialReceiver.cpp \
	  CreateSerial.cpp

OBJS 	= main.o \
	  ARtag.o \
	  ARtagLocalizer.o \
	  CreateSerialReceiver.o \
	  CreateSerial.o

EXECUTABLE = main

all	: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJS)
	$(CPP) $(OBJS) $(LDFLAGS) -o $@ $*

main.o	: defs.h all.h thread.h main.cpp
	$(CPP) $(CFLAGS) -c $^

ARtag.o	: ARtag.h ARtag.cpp
	$(CPP) $(CFLAGS) -c $^

ARtagLocalizer.o : ARtagLocalizer.h ARtag.h ARtagLocalizer.cpp
	$(CPP) $(CFLAGS) -c $^

CreateSerialReceiver.o : defs.h all.h Packet.h thread.h CreateSerialReceiver.h CreateSerialReceiver.cpp
	$(CPP) $(CFLAGS) -c $^
	
CreateSerial.o : defs.h all.h Packet.h thread.h CreateSerialReceiver.h CreateSerial.h CreateSerial.cpp
	$(CPP) $(CFLAGS) -c $^

$(OBJS)	: defs.h all.h Packet.h thread.h ARtag.h ARtagLocalizer.h CreateSerialReceiver.h CreateSerial.h

clean	:
	rm -rf main *.o
	rm -rf *.*~
	rm -rf *.h.gch
