EXE = opengl2_example
OBJS = climain.o imgui_backend/imgui_impl_glfw.o
OBJS += imgui/imgui.o imgui/imgui_demo.o imgui/imgui_draw.o
ECHO_MESSAGE = "Linux"
LIBS = -lGL `pkg-config --static --libs glfw3`
CXXFLAGS = -I../../ `pkg-config --cflags glfw3`
CXXFLAGS += -Wall -Wformat
CFLAGS = $(CXXFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $(EXE) $(OBJS) $(CXXFLAGS) $(LIBS)

clean:
	rm $(EXE) $(OBJS)

