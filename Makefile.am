include $(top_srcdir)/config/common.am

DISTCLEANFILES += xforms.spec config.status config.cache config.log config.h

MAINTAINERCLEANFILES += \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/configure \
	$(srcdir)/acinclude.m4 \
	$(srcdir)/config.h.in

SUBDIRS = config lib image gl fd2ps fdesign

# Putting 'demos' in EXTRA_DIST rather than SUBDIRS prevents its contents
# from being built from the top level directory.
# (cd demos && make) works fine.
EXTRA_DIST = ChangeLog COPYING.LIB Copyright README README.OS2 \
	autogen.sh xforms.spec.in xforms.spec demos

# Hack so that the targets that use tar will also work with automake 1.4
AMTAR ?= $(TAR)

dist-hook:
	cd $(distdir) ; \
	rm -rf `find images -name \*CVS\*`

rpmdist: dist
	if [ -z "`type -path rpmbuild`" ]; \
	then \
		RPMBUILD=rpm; \
	else \
		RPMBUILD=rpmbuild; \
	fi; $$RPMBUILD -ta $(PACKAGE)-$(VERSION).tar.gz ; saved_status=$$?; \
	exit $$saved_status
