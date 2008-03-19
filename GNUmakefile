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
	{AUTHORS,README,GNUmakefile} \
	src/GNUmakefile \
	src/{main,display,game}.cpp src/{display,game,vec}.h \
	src/{Makefile.cross,sfcave9.pl,cave9-global.pl} \
	| tar -C $$dir -xf -; \
	archive=cave9_src.$$date.tgz; \
	tar -cvzf $$archive $$dir; \
	rm -rf $$dir; \
	ls -sh $$archive

