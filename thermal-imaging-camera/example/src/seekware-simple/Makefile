
INCFLAG		= -I./include
DEBUGFLAG	= -g -O2 -Wall -fpermissive
CFLAGS		+= $(DEBUGFLAG) $(INCFLAG) -std=c++11
LDFLAGS	    	+= -lseekware -lstdc++ -lpthread `pkg-config --libs libusb-1.0`
CXXFLAGS	+= $(CFLAGS)
CPPFLAGS	+= $(CFLAGS)

TARGET	= seekware-simple
OBJECTS	= objs/seekware-simple.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDPATH) $(LDFLAGS)
	
install: $(TARGET)
	mkdir -p $(prefix)/bin
	cp $(TARGET) $(prefix)/bin 

install_src:
	mkdir -p $(prefix)/src/$(TARGET)
	cp -fr src $(prefix)/src/$(TARGET)/ 
	cp -fr include $(prefix)/src/$(TARGET)/ 
	cp -f  Makefile $(prefix)/src/$(TARGET)/

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

clean:
	@rm -rf core *~  $(TARGET) $(OBJECTS) $(LIB) $(LIBOBJ) src/*~ include/*~ 
	
