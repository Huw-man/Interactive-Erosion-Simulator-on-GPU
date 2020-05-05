PROGRAM = erosion_sim
IDIR =.
CC=gcc
CFLAGS=-I$(IDIR)
INC= #-I imgui/

ODIR=obj
LDIR =../lib

LIBS=-lglew32 -lopengl32 -lglu32 -lglfw3 -lgdi32 -lstdc++

_DEPS = main.hpp common/* plane_mesh.hpp imgui/*
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o common/shader.o common/texture.o common/controls.o common/objloader.o imgui/imgui_impl_glfw.o imgui/imgui_impl_opengl3.o imgui/imgui.o imgui/imgui_draw.o imgui/imgui_widgets.o imgui/imgui_demo.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(INC)

default: $(OBJ)
	$(CC) -o ../${PROGRAM} $^ $(CFLAGS) $(INC) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 