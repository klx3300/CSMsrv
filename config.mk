# change configurations here
# there are some precise reasons that we create this file.
# so dont do ugly stuff.

CC := gcc
CXX := g++
TERM_ECHO :=  @
# change that to print every command it executed.
TERM_IGNR := -
# change that to forcefully raise all ignored error output.
CLEAN_OUTPUT := 2>/dev/null
# change that to display all hidden output.
CANCEL_ERRNO := || true
# change that to display all ignored error code.
ECHO_PROG := echo 
# maybe you want the output to be printed to somewhere else.
# perhaps some network place?
# ECHO_PROG := network_echo some_server:some_port
CFLAGS := -g -pthread
CXXFLAGS := -g `pkg-config --cflags glfw3` $(CFLAGS)
# perhaps you wish to use some compiler flags.
# don't hesitate to add stuff here.
CLILIBS := -lGL `pkg-config --static --libs glfw3`
SRVLIBS := 

REMOVE_FILE := rm
# the interesting fact: this variable differs on operating system.
# even on some operating system, there are guys that intended to use
# this stuff to do something that may or may not help him. example:
# REMOVE_FILE := rm -i

YOUR_MODULE := se_proxy

YOUR_MODULE_TEST := se_proxy_test