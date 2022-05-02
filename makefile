APPDIR = .
ALEDIR = ./ALE

USE_SDL := 1

CFLAGS = -std=c++11 -Wall -Wextra -Wno-deprecated -O3 -I$(APPDIR)/include 
FLAGS := -I$(ALEDIR)/src -I$(ALEDIR)/src/controllers -I$(ALEDIR)/src/os_dependent -I$(ALEDIR)/src/environment -I$(ALEDIR)/src/external 
LDFLAGS = ./lib/libale.so

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    FLAGS += -Wl,-rpath=$(ALEDIR)
endif
#ifeq ($(UNAME_S),Darwin)
#    FLAGS += -framework Cocoa
#endif

ifeq ($(strip $(USE_SDL)), 1)
  DEFINES += -D__USE_SDL -DSOUND_SUPPORT
  FLAGS += $(shell sdl-config --cflags)
  LDFLAGS += $(shell sdl-config --libs)
endif

.PHONY : clean all

all: obj/ale_ebc.o obj/example.o bin/example

clean:
	rm -f obj/* bin/*

obj/ale_ebc.o: src/ale_ebc.cpp include/ale_ebc.hpp ALE/src/ale_interface.hpp
	c++ $(CFLAGS) $(FLAGS) $(DEFINES) -c src/ale_ebc.cpp -o obj/ale_ebc.o 

obj/example.o: src/example.cpp src/ale_ebc.cpp ALE/src/ale_interface.hpp 
	c++ $(CFLAGS) $(DEFINES) $(FLAGS) -c src/example.cpp -o obj/example.o	

bin/example: obj/ale_ebc.o obj/example.o 
	c++ $(CFLAGS) $(DEFINES) -o bin/example $(FLAGS) obj/example.o obj/ale_ebc.o $(APPDIR)/lib/libale.so
