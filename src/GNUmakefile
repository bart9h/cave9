#{{{1  setup

TARGET = ../cave9

CC = gcc
CFLAGS = -std=c99 -Wall -Werror -ggdb `sdl-config --cflags` -I/usr/include/SDL
LDFLAGS = `sdl-config --libs` -lm -L/usr/X11/lib -lGL -lGLU -lSDL_ttf -lSDL_image -lSDL_net


#{{{1  build rules

SRCS = \
	 main.c \
	 display.c \
	 game.c \

OBJS = $(subst .c,.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^


#{{{1  administrative rules

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) core*


.depend:
	$(CC) -MM $(CFLAGS) $(SRCS) > $@

dep:
	rm -f .depend
	$(MAKE) .depend

ifneq ($(wildcard .depend),)
include .depend
endif


tags:
	$(CC) -M $(CFLAGS) $(SRCS) \
		| sed 's/:\ /\ /;s/\ /\n/g;s/\\//' | grep -v '\.o$$' | sort | uniq \
		| ctags --c++-kinds=+px-f --c-kinds=+px-f -L -

#}}}
# vim600:fdm=marker
