EXE = opengl2_example
OBJS = main.o imgui_backend/imgui_impl_glfw.o
OBJS += cimgui/imgui/imgui.o cimgui/imgui/imgui_demo.o cimgui/imgui/imgui_draw.o cimgui/cimgui/cimgui.o
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

