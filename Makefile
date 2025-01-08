# ROOT configurations
ROOTCFLAGS    := $(shell root-config --cflags)
ROOTLIBS      := $(shell root-config --libs)
ROOTGLIBS     := $(shell root-config --glibs) -lRooFit -lRooFitCore

# Directories and files
TARGET        := read_pscan
INCDIR        := include
SRCDIR        := src
SRC           := main.cpp $(wildcard $(SRCDIR)/*.cpp)
OBJ           := $(SRC:.cpp=.o)

# Compiler and flags
CXX           := g++
CXXFLAGS      := -I$(INCDIR) $(ROOTCFLAGS) -pthread -std=c++20 -Wall -Wextra -g
LDFLAGS       := $(ROOTLIBS) $(ROOTGLIBS)

# Targets
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean
