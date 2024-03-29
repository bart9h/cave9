#{{{1  setup

TARGET = ../cave9

CFLAGS += -std=c99 -Wall -Werror -ggdb `sdl-config --cflags`
GCC_VERSION = $(shell gcc -dumpfullversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/')
GCC_SINCE_7_1 = $(shell expr $(GCC_VERSION) \>= 070100)
ifeq ($(GCC_SINCE_7_1), 1)
CFLAGS += -Wno-format-truncation
endif
LDFLAGS += -lSDL_ttf -lSDL_image `sdl-config --libs` -L/usr/X11/lib -lGL -lGLU -lm

ifdef GLOBAL_SCORE
CFLAGS += -DGLOBAL_SCORE
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
	 render.c \
	 score.c \
	 util.c \
	 detrand.c \

OBJS = $(subst .c,.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@


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
