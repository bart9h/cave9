TARGET=cave9
TTF_FLAGS=-DUSE_TTF
TTF_LIBS=-lSDL_ttf

SRCS=\
	 main.cpp \
	 display.cpp \
	 game.cpp \

CXXFLAGS = -Wall -Werror -ggdb `sdl-config --cflags` -I/usr/include/SDL $(TTF_FLAGS)
LDFLAGS = `sdl-config --libs` -lm -L/usr/X11/lib -lGL -lGLU $(TTF_LIBS)

CXX = gcc

OBJS = $(subst .cpp,.o,$(SRCS))

$(TARGET): $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

#gcc -Wall -g -I/usr/include/SDL `sdl-config --cflags --libs` \
	-lGL -L/usr/X11/lib -lGLU $0 -o $bin && ./$bin

clean:
	rm -f $(TARGET) *.o core*

game.o: game.cpp game.h vec.h
display.o: display.cpp display.h vec.h
main.o: main.cpp vec.h display.h game.h

