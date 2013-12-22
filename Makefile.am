include $(top_srcdir)/config/common.am

DISTCLEANFILES += libforms.spec config.status config.cache config.log config.h

ACLOCAL_AMFLAGS = -I config

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

# libforms.spec is needed as well as libforms.spec.in or else 'make rpmdist'
# will fail.

EXTRA_DIST = ChangeLog COPYING.LIB Copyright INSTALL INSTALL.git README \
			 README.rpmbuild autogen.sh libforms.spec.in libforms.spec

# Builds RPM packages (using the libforms.spec file), one (architecture-
# dependend) binary package and a development, documentation and source
# package

libdist: dist
	tar -vxf $(PACKAGE)-$(VERSION).tar.gz; \
	rm -f $(PACKAGE)-$(VERSION).tar.gz; \
	mv $(PACKAGE)-$(VERSION) libforms-$(VERSION); \
	tar -czf libforms-$(VERSION).tar.gz libforms-$(VERSION); \
	rm -rf libforms-$(VERSION);

rpmdist: libdist
	rpmbuild -ta libforms-$(VERSION).tar.gz; \
	saved_status=$$?; \
	exit $$saved_status
