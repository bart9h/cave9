all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

release:
	@set -e; \
	which git tar gzip date >/dev/null; \
	version=`date +%Y-%m-%d`; \
	archive=cave9_src.$$version.tgz; \
	git archive --prefix=cave9.$$version/ HEAD | gzip -9 > $$archive; \
	tar tfvz $$archive; \
	ls -sh $$archive

