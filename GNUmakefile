all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

dep:
	$(MAKE) -C src dep

release:
	@set -e; \
	which git tar gzip grep cut >/dev/null; \
	version=`grep CAVE9_VERSION src/version.h | cut -d \" -f 2`; \
	archive=cave9_src-$$version.tgz; \
	git archive --prefix=cave9-$$version/ HEAD | gzip -9 > $$archive; \
	tar tfvz $$archive; \
	ls -sh $$archive

