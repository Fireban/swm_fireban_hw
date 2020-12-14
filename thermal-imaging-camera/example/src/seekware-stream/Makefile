
INCFLAG		= -I./include
DEBUGFLAG	= -g -O2 -Wall
CFLAGS		+= $(DEBUGFLAG) $(INCFLAG)
LDFLAGS	    	+= -lseekware -lpthread -lSDL2 `pkg-config --libs libusb-1.0`
CXXFLAGS	+= $(CFLAGS) -fpermissive
CPPFLAGS	+= $(CFLAGS)

TARGET	= seekware-stream
OBJECTS	= objs/seekware-stream.o objs/colour.o

all: $(TARGET)

install: $(TARGET)
	mkdir -p $(prefix)/bin
	cp $(TARGET) $(prefix)/bin

install_src:
	mkdir -p $(prefix)/src/$(TARGET)
	cp -fr src $(prefix)/src/$(TARGET)/
	cp -fr include $(prefix)/src/$(TARGET)/
	cp -f  Makefile $(prefix)/src/$(TARGET)/
	cp -fr imagej_luts $(prefix)/src/$(TARGET)/

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDPATH) $(LDFLAGS)

objs/%.o:	src/%.cpp
	@if test ! -e objs; then \
		mkdir objs  ;\
	fi;
	$(CXX) $(CXXFLAGS) -c $< -o $@

objs/%.o:	src/%.c
	@if test ! -e objs; then \
		mkdir objs  ;\
	fi;
	$(CXX) $(CXXFLAGS) -c $< -o $@

doc:
	doxygen

clean:
	@rm -rf core *~  $(TARGET) $(OBJECTS) $(LIB) $(LIBOBJ) src/*~ include/*~


