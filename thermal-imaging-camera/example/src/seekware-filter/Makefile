
INCFLAG		+= -I./include
DEBUGFLAG	= -g -O0 -Wall
CFLAGS		+= $(DEBUGFLAG) $(INCFLAG)
LDFLAGS	   	+= -lseekware -lSDL2 `pkg-config --libs libusb-1.0`
LIBS		+= -lcurl
CXXFLAGS	+= $(CFLAGS)
CPPFLAGS	+= $(CFLAGS)

TARGET	= seekware-sdl
OBJECTS	= objs/seekware-sdl.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDPATH) $(LDFLAGS) $(LIBS)

objs/%.o:	src/%.cpp
	@if test ! -e objs; then \
		mkdir objs  ;\
	fi;
	$(CXX) $(CXXFLAGS) -c $< -o $@

objs/%.o:	src/%.c
	@if test ! -e objs; then \
		mkdir objs  ;\
	fi;
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf core *~  $(TARGET) $(OBJECTS) $(LIBOBJ) src/*~ include/*~

