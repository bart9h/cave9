all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

release:
	@set -e; \
	date=`date +%Y-%m-%d`; \
	dir=cave9.$$date; \
	rm -rf $$dir; mkdir $$dir; \
	tar -cf - \
		AUTHORS \
		README \
		GNUmakefile \
		src/GNUmakefile \
		src/main.c \
		src/display.h \
		src/display.c \
		src/game.h \
		src/game.c \
		src/vec.h \
		src/Makefile.cross \
		src/sfcave9.pl \
		src/cave9-global.pl \
	| tar -C $$dir -xf -; \
	archive=cave9_src.$$date.tgz; \
	tar -cvzf $$archive $$dir; \
	rm -rf $$dir; \
	ls -sh $$archive

