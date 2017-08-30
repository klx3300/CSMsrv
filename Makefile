
#EXE = cli
#OBJS = climain.o imgui_backend/imgui_impl_glfw.o
#OBJS += imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o
#OBJS += advanced_network/advanced_network.o
#OBJS += zhwkre/bss/bss.o zhwkre/concurrent/mutex.o zhwkre/concurrent/threading.o
#OBJS += zhwkre/containers/list.o zhwkre/containers/unordered_map.o zhwkre/network/socket.o
#OBJS += zhwkre/network/tcp.o zhwkre/serialization/serialization.o zhwkre/utils/utils.o
#OBJS += protocol/protocol.o clinet/clinet.o
#ECHO_MESSAGE = "Linux"
#LIBS = -lGL `pkg-config --static --libs glfw3`
#CXXFLAGS =  -g `pkg-config --cflags glfw3`
#CFLAGS = $(CXXFLAGS)

#.cpp.o:
#	$(CXX) $(CXXFLAGS) -c -o $@ $<

#all: $(EXE)
#	@echo Build complete for $(ECHO_MESSAGE)

#$(EXE): $(OBJS)
#	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS) $(LIBS)

#clean:
#	rm $(EXE) $(OBJS)

include config.mk

main : cli srv
	$(TERM_ECHO)$(ECHO_PROG) "     --   destination main built."

cli : advanced_network.o bss.o mutex.o threading.o list.o unordered_map.o socket.o tcp.o serialization.o utils.o imgui_impl_glfw.o imgui.o imgui_demo.o imgui_draw.o protocol.o clinet.o climain.o permissionctl.o
	$(TERM_ECHO)$(ECHO_PROG) "     LD   cli"
	$(TERM_ECHO)$(CXX) -o cli permissionctl.o advanced_network.o bss.o mutex.o threading.o list.o unordered_map.o socket.o tcp.o serialization.o utils.o imgui_impl_glfw.o imgui.o imgui_demo.o imgui_draw.o protocol.o clinet.o climain.o $(CXXFLAGS) $(CLILIBS)
	$(TERM_ECHO)$(ECHO_PROG) "     --   destination cli built."

srv : advanced_network.o bss.o mutex.o threading.o list.o unordered_map.o socket.o tcp.o serialization.o utils.o protocol.o permissionctl.o srvmain.o
	$(TERM_ECHO)$(ECHO_PROG) "     LD   srv"
	$(TERM_ECHO)$(CC) -o srv advanced_network.o bss.o mutex.o threading.o list.o unordered_map.o socket.o tcp.o serialization.o utils.o protocol.o permissionctl.o srvmain.o $(CFLAGS) $(SRVLIBS)
	$(TERM_ECHO)$(ECHO_PROG) "     --   destination srv built."

clean : clean_intermediate clean_object
	$(TERM_ECHO)$(ECHO_PROG) "  CLEAN   done"

clean_intermediate :
	$(TERM_ECHO)$(ECHO_PROG) "  CLEAN   intermediate files"
	$(TERM_IGNR)$(TERM_ECHO)$(REMOVE_FILE) *.o $(CLEAN_OUTPUT) $(CANCEL_ERRNO)

clean_object :
	$(TERM_ECHO)$(ECHO_PROG) "  CLEAN   destination files"
	$(TERM_IGNR)$(TERM_ECHO)$(REMOVE_FILE) cli srv $(CLEAN_OUTPUT) $(CANCEL_ERRNO)

imgui.o : imgui/imgui.cpp
	$(TERM_ECHO)$(ECHO_PROG) "    CXX   imgui.o"
	$(TERM_ECHO)$(CXX) $(CXXFLAGS) -c -o imgui.o imgui/imgui.cpp

imgui_demo.o : imgui/imgui_demo.cpp
	$(TERM_ECHO)$(ECHO_PROG) "    CXX   imgui_demo.o"
	$(TERM_ECHO)$(CXX) $(CXXFLAGS) -c -o imgui_demo.o imgui/imgui_demo.cpp

imgui_draw.o : imgui/imgui_draw.cpp
	$(TERM_ECHO)$(ECHO_PROG) "    CXX   imgui_draw.o"
	$(TERM_ECHO)$(CXX) $(CXXFLAGS) -c -o imgui_draw.o imgui/imgui_draw.cpp

imgui_impl_glfw.o : imgui_backend/imgui_impl_glfw.cpp
	$(TERM_ECHO)$(ECHO_PROG) "    CXX   imgui_impl_glfw.o"
	$(TERM_ECHO)$(CXX) $(CXXFLAGS) -c -o imgui_impl_glfw.o imgui_backend/imgui_impl_glfw.cpp

advanced_network.o : advanced_network/advanced_network.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   advanced_network.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o advanced_network.o advanced_network/advanced_network.c

clinet.o : clinet/clinet.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   clinet.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o clinet.o clinet/clinet.c

permissionctl.o : permissionctl/permissionctl.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   permissionctl.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o permissionctl.o permissionctl/permissionctl.c

protocol.o : protocol/protocol.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   protocol.c"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o protocol.o protocol/protocol.c

bss.o : zhwkre/bss/bss.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   bss.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o bss.o zhwkre/bss/bss.c

mutex.o: zhwkre/concurrent/mutex.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   mutex.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o mutex.o zhwkre/concurrent/mutex.c

threading.o : zhwkre/concurrent/threading.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   threading.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o threading.o zhwkre/concurrent/threading.c

list.o : zhwkre/containers/list.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   list.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o list.o zhwkre/containers/list.c

unordered_map.o : zhwkre/containers/unordered_map.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   unordered_map.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o unordered_map.o zhwkre/containers/unordered_map.c

socket.o : zhwkre/network/socket.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   socket.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o socket.o zhwkre/network/socket.c

tcp.o : zhwkre/network/tcp.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   tcp.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o tcp.o zhwkre/network/tcp.c

serialization.o: zhwkre/serialization/serialization.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   serialization.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o serialization.o zhwkre/serialization/serialization.c

utils.o: zhwkre/utils/utils.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   utils.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o utils.o zhwkre/utils/utils.c

climain.o: climain.cpp
	$(TERM_ECHO)$(ECHO_PROG) "    CXX   climain.o"
	$(TERM_ECHO)$(CXX) $(CXXFLAGS) -c -o climain.o climain.cpp

srvmain.o: srvmain.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   srvmain.o"
	$(TERM_ECHO)$(CC) $(CFLAGS) -c -o srvmain.o srvmain.c