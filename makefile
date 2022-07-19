APPDIR = .
ALEDIR = ./ALE

CFLAGS = -std=c++11 -Wall -Wextra -Wno-deprecated -O3 -I$(APPDIR)/include 
FLAGS := -I$(ALEDIR)/src -I$(ALEDIR)/src/controllers -I$(ALEDIR)/src/os_dependent -I$(ALEDIR)/src/environment -I$(ALEDIR)/src/external 
LDFLAGS = ./lib/libale.so

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    FLAGS += -Wl,-rpath=$(ALEDIR)
endif

.PHONY : clean all

all: examples python

examples: obj/ale_ebc.o obj/example.o obj/example2.o bin/example bin/example2

clean:
	rm -f obj/* bin/* build/bindings/* build/*.o build/*.so 

obj/ale_ebc.o: src/ale_ebc.cpp include/ale_ebc.hpp ALE/src/ale_interface.hpp
	g++ $(CFLAGS) $(FLAGS) $(DEFINES) -c src/ale_ebc.cpp -o obj/ale_ebc.o 

obj/example.o: examples/example.cpp src/ale_ebc.cpp ALE/src/ale_interface.hpp 
	g++ $(CFLAGS) $(DEFINES) $(FLAGS) -c examples/example.cpp -o obj/example.o

obj/example2.o: examples/example2.cpp src/ale_ebc.cpp ALE/src/ale_interface.hpp 
	g++ $(CFLAGS) $(DEFINES) $(FLAGS) -c examples/example2.cpp -o obj/example2.o		

bin/example: obj/ale_ebc.o obj/example.o 
	g++ $(CFLAGS) $(DEFINES) -o bin/example $(FLAGS) obj/example.o obj/ale_ebc.o $(APPDIR)/lib/libale.so

bin/example2: obj/ale_ebc.o obj/example2.o 
	g++ $(CFLAGS) $(DEFINES) -o bin/example2 $(FLAGS) obj/example2.o obj/ale_ebc.o $(APPDIR)/lib/libale.so


### Python bindings
ROOT = .
INC = ./include
SRC = ./src
PYBINDINGS = ./bindings
PYBUILD = build
PYBUILD_BINDINGS = $(PYBUILD)/bindings
PYBUILD_SUFFIX := $(shell python3-config --extension-suffix)
PYLIBPB =  $(PYBUILD)/ale_ebc$(PYBUILD_SUFFIX)

# Compilation Flags
CFLAGS ?= -Wall -Wextra -pipe -O3
CFLAGSBASE = $(CFLAGS) -std=c++11 -I$(INC) -I$(ALEDIR)/src 
LFLAGS = -lpthread

## Python support
PYBUILD_FLAGS := $(shell python3 -m pybind11 --includes) -I$(INC) -I$(ALEDIR)/src -I$(PYBINDINGS) \
                 -std=c++11 -flto -fPIC -O3 -fvisibility=hidden
PYBUILD_LFLAGS = -shared

# Patch symbol linkage issues for Mac OS
OS := $(strip $(shell uname -s))

ifeq ($(OS),Darwin)
    PYBUILD_LFLAGS += -undefined dynamic_lookup
endif

PYTHON_INSTALL_USER ?= true

python: $(PYLIBPB)
SOURCES = $(SRC)/ale_ebc.cpp

BINDING_SOURCES := $(PYBINDINGS)/ale_ebc.cpp

BINDING_OBJECTS := $(patsubst $(PYBINDINGS)/%.cpp,$(PYBUILD_BINDINGS)/%.o,$(BINDING_SOURCES))

PYBUILD_OBJECTS := $(patsubst $(SRC)/%.cpp,$(PYBUILD)/%.o,$(SOURCES)) \
           $(patsubst $(SRC)/%.cpp,$(PYBUILD)/%.o,$(TL_SOURCES))

PYBUILD_TL_OBJECTS := $(wildcard $(ROOT)/$(PYBUILD)/*.o)

$(BINDING_OBJECTS): $(PYBUILD_BINDINGS)/%.o : $(PYBINDINGS)/%.cpp $(HEADERS) $(TL_HEADERS) | $(PYBUILD_BINDINGS)
	c++  $(PYBUILD_FLAGS) -c $< -o $@

$(PYBUILD_OBJECTS): $(PYBUILD)/%.o : $(SRC)/%.cpp $(HEADERS) $(TL_HEADERS) | $(PYBUILD)
	c++ $(PYBUILD_FLAGS) -c $< -o $@

$(PYLIBPB): $(PYBUILD_OBJECTS) $(BINDING_OBJECTS) 
	c++ $(PYBUILD_FLAGS) $(PYBUILD_LFLAGS) $(PYBUILD_OBJECTS) $(BINDING_OBJECTS) $(PYBUILD_TL_OBJECTS) $(APPDIR)/lib/libale.so -o $@