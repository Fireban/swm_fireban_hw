CC = gcc
CXX = g++
INCFLAG		+= -I./include
DEBUGFLAG	= -g -O0 -Wall
CFLAGS		+= $(DEBUGFLAG) $(INCFLAG) $(LIBS)
LIBS		+= -lcurl

TARGET = /home/pi/autorun 
SRCS = PmodGPS.c process.c
OBJECTS = objs/PmodGPS.o objs/process.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDPATH) $(LIBS)


objs/%.o:	src/%.cpp
	@if test ! -e objs; then \
		mkdir objs ;\
	fi;
	$(CXX) $(CFLAGS) -c $< -o $@

objs/%.o:	src/%.c
	@if test ! -e objs; then \
		mkdir objs	;\
	fi;
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf core *~ $(TARGET) $(OBJECTS) src/*~ include/*~
