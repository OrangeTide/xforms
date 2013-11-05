include $(top_srcdir)/config/common.am

DISTCLEANFILES += xforms.spec config.status config.cache config.log config.h

MAINTAINERCLEANFILES += \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/configure \
	$(srcdir)/acinclude.m4 \
	$(srcdir)/lib/config.h.in

if BUILD_DEMOS
DEMOS = demos
else
DEMOS =
endif

if BUILD_GL
GL = gl
else
GL =
endif

if BUILD_DOCS
DOCS = doc
else
DOCS =
endif

SUBDIRS = config lib image $(GL) fdesign fd2ps $(DEMOS) $(DOCS)

# xforms.spec is needed as well as xforms.spec.in or else 'make rpmdist'
# will fail.
EXTRA_DIST = ChangeLog COPYING.LIB Copyright INSTALL README \
			 autogen.sh xforms.spec.in xforms.spec

rpmdist: dist
	if [ -z "`type -path rpmbuild`" ]; \
	then \
		RPMBUILD=rpm; \
	else \
		RPMBUILD=rpmbuild; \
	fi; $$RPMBUILD -ta $(PACKAGE)-$(VERSION).tar.gz ; saved_status=$$?; \
	exit $$saved_status
