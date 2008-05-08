VERSION=src/version.h
CODE_VERSION=$(shell grep CODE_VERSION $(VERSION) | cut -d \" -f 2)
DATA_VERSION=$(shell grep DATA_VERSION $(VERSION) | cut -d \" -f 2)

DATA = \
	data/hit.wav \
	data/icon.png \
	data/wall.jpg \
	data/thrust.wav \
	data/hud.ttf \
	data/README.txt

build: $(DATA)
	$(MAKE) -C src

DATA_PKG=cave9_data-$(DATA_VERSION).tgz
$(DATA):
	wget -c http://cave9.googlecode.com/files/$(DATA_PKG)
	tar xzkf $(DATA_PKG)
$(DATA_PKG): $(DATA)
	tar c $(DATA) | gzip -9 > $(DATA_PKG)
	tar tfvz $(DATA_PKG)


SRC_PKG=cave9_src-$(CODE_VERSION).tgz
$(SRC_PKG):
	git archive --prefix=cave9-$(CODE_VERSION)/ HEAD | gzip -9 > $(SRC_PKG)
	tar tfvz $(SRC_PKG)


WIN_TARGET=cave9.exe
WIN_PKG=cave9_win-$(CODE_VERSION)-$(DATA_VERSION).zip
$(WIN_TARGET): 
	make -C src -f Makefile.cross
$(WIN_PKG): $(WIN_TARGET) $(DATA)
	zip -rp -9 $(WIN_PKG) $(WIN_TARGET) $(DATA)
	unzip -t $(WIN_PKG)


RELEASE = \
	$(shell make -C src -f Makefile.cross check 2>/dev/null 1>/dev/null && echo $(WIN_PKG) ) \
	$(DATA_PKG) \
	$(SRC_PKG) \

release: $(RELEASE)
	ls -1sh $^

check:
	which git tar gzip grep cut zip unzip wget >/dev/null 

dep:
	$(MAKE) -C src dep

clean:
	$(MAKE) -C src clean
	$(MAKE) -C src -f Makefile.cross clean
	rm -f $(WIN_PKG) $(DATA_PKG) $(SRC_PKG)

all: build release

test:
	@echo -n "dep: " && make clean dep 2>/dev/null 1>/dev/null && echo OK
	@echo -n "build: " && make clean build 2>/dev/null 1>/dev/null && echo OK
	@echo -n "release: " && make clean release 2>/dev/null 1>/dev/null && echo OK && ls -1sh $(RELEASE)
	@make clean 2>/dev/null 1>/dev/null
