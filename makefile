CC = g++
CCFLAGS = -Wall -g -std=c++14
PREFLAGS = -Wall -c -std=c++14
INCLUDE = -I /Users/durand/Desktop/Centre/A\ propos\ de\ programmes/CPP/breakout/include
LIBS = -L /Users/durand/Desktop/Centre/A\ propos\ de\ programmes/CPP/breakout/lib-macos -lglfw3 -lfreetype -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo
DEPENDENCIESCPP = main.cpp editor.cpp shader.cpp stb_image.cpp texture.cpp resource_manager.cpp glad.cpp  text_renderer.cpp box.cpp buffer.cpp piece.cpp regex.cpp file.cpp
DEPS = editor.h resource_manager.h text_renderer.h box.h buffer.h piece.h regex.h file.h
AUX = editor.o resource_manager.o text_renderer.o shader.o stb_image.o texture.o glad.o main.o box.o buffer.o piece.o regex.o file.o


main: $(AUX) $(DEPENDENCIESCPP)
	@echo "LINK $@"
	$(CC) $(CCFLAGS) $(INCLUDE) -o $@ $(AUX) $(LIBS)

%.o: %.cpp $(DEPS)
	@echo "CC $<"
	$(CC) $(INCLUDE) $(PREFLAGS) -o $@ $<


test: test.o piece.o test.cpp piece.cpp piece.h
	@echo "LINK $@"
	$(CC) $(CCFLAGS) test.o piece.o -o test

re: regex.o regex.cpp regex.h piece.o piece.cpp piece.h
	@echo "LINK $@"
	$(CC) $(CCFLAGS) -o regex regex.o piece.o

$(V).SILENT:
