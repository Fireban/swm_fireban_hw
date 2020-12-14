CC = gcc
CXX = g++
INCFLAG		+= -I./include
DEBUGFLAG	= -g -O0 -Wall
CFLAGS		+= $(DEBUGFLAG) $(INCFLAG) $(LIBS)
LIBS		+= -lcurl

TARGET = gps_sensor
SRCS = PmodGPS.c gps_post.c
OBJECTS = objs/PmodGPS.o objs/gps_post.o
#OBJECTS = $(SRCS:.c=.o) $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDPATH) $(LIBS)

#$(CC) $(CFLAGS) -o $@ $(OBJECTS)

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
#rm -rf *.o $(TARGET) $(OBJECTS)
