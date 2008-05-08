#{{{1  setup

TARGET = ../cave9

CFLAGS += -std=c99 -Wall -Werror -ggdb `sdl-config --cflags`
LDFLAGS += `sdl-config --libs` -lm -L/usr/X11/lib -lGL -lGLU -lSDL_ttf -lSDL_image

ifdef USE_SDLNET
CFLAGS += -DUSE_SDLNET
LDFLAGS += -lSDL_net
endif

# necessary when some SDL libs are on /usr/local, and others on /usr
CFLAGS += -I/usr/local/include/SDL -I/usr/include/SDL


#{{{1  build rules

SRCS = \
	 main.c \
	 audio.c \
	 control.c \
	 display.c \
	 game.c \
	 score.c \
	 util.c \

OBJS = $(subst .c,.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^


#{{{1  administrative rules

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) core* .depend


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
