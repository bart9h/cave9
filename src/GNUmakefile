#{{{1  setup

TARGET=cave9
TTF_FLAGS=-DUSE_TTF
TTF_LIBS=-lSDL_ttf

CXX = gcc
CXXFLAGS = -Wall -Werror -ggdb `sdl-config --cflags` -I/usr/include/SDL $(TTF_FLAGS)
LDFLAGS = `sdl-config --libs` -lm -L/usr/X11/lib -lGL -lGLU $(TTF_LIBS) -lSDL_image


#{{{1  build rules

SRCS=\
	 main.cpp \
	 display.cpp \
	 game.cpp \

OBJS = $(subst .cpp,.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^


#{{{1  administrative rules

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS) core*


.depend:
	$(CXX) -MM $(CXXFLAGS) $(SRCS) > $@

dep:
	rm -f .depend
	$(MAKE) .depend

ifneq ($(wildcard .depend),)
include .depend
endif


tags:
	$(CXX) -M $(CXXFLAGS) $(SRCS) \
		| sed 's/:\ /\ /;s/\ /\n/g;s/\\//' | grep -v '\.o$$' | sort | uniq \
		| ctags --c++-kinds=+px-f --c-kinds=+px-f -L -

#}}}
# vim600:fdm=marker
