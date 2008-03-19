all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

release:
	@set -e; \
	dir=cave9.`date +%Y-%m-%d`; \
	rm -rf $$dir; mkdir $$dir; \
	tar -cf - \
	{AUTHORS,README,GNUmakefile} \
	src/GNUmakefile \
	src/{main,display,game}.cpp src/{display,game,vec}.h \
	src/{Makefile.cross,sfcave9.pl,cave9-global.pl} \
	| tar -C $$dir -xf -; \
	tar -cvzf $$dir.tgz $$dir; \
	rm -rf $$dir; \
	ls -sh $$dir.tgz

