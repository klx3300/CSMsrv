EXE = cli
OBJS = climain.o imgui_backend/imgui_impl_glfw.o
OBJS += imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o
OBJS += advanced_network/advanced_network.o
OBJS += zhwkre/bss/bss.o zhwkre/concurrent/mutex.o zhwkre/concurrent/threading.o
OBJS += zhwkre/containers/list.o zhwkre/containers/unordered_map.o zhwkre/network/socket.o
OBJS += zhwkre/network/tcp.o zhwkre/serialization/serialization.o zhwkre/utils/utils.o
OBJS += protocol/protocol.o clinet/clinet.o
ECHO_MESSAGE = "Linux"
LIBS = -lGL `pkg-config --static --libs glfw3`
CXXFLAGS =  -g `pkg-config --cflags glfw3`
CFLAGS = $(CXXFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS) $(LIBS)

clean:
	rm $(EXE) $(OBJS)

