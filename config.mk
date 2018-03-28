CC := gcc
CXX := g++
TERM_ECHO :=  @
TERM_IGNR := -
CLEAN_OUTPUT := 2>/dev/null
CANCEL_ERRNO := || true
ECHO_PROG := echo 
CFLAGS := -g -pthread
CXXFLAGS := -g `pkg-config --cflags glfw3` $(CFLAGS)
CLILIBS := -lGL `pkg-config --static --libs glfw3`
SRVLIBS := 

REMOVE_FILE := rm
