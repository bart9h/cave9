TARGET=cave9

SRCS=\
	 main.c \
	 display.c \
	 game.c \

CFLAGS = -Wall -Werror -ggdb `sdl-config --cflags` -I/usr/include/SDL
LDFLAGS = `sdl-config --libs` -lSDL_ttf -lm -L/usr/X11/lib -lGL -lGLU

OBJS = $(subst .c,.o,$(SRCS))

$(TARGET): $(OBJS)
	gcc $(LDFLAGS) -o $@ $^

#gcc -Wall -g -I/usr/include/SDL `sdl-config --cflags --libs` \
	-lGL -L/usr/X11/lib -lGLU $0 -o $bin && ./$bin

clean:
	rm -f $(TARGET) *.o core*

game.o: game.c game.h vec.h
display.o: display.c display.h vec.h
main.o: main.c vec.h display.h game.h
