include $(top_srcdir)/config/common.am

DISTCLEANFILES += xforms.spec config.status config.cache config.log config.h

MAINTAINERCLEANFILES += \
	$(srcdir)/aclocal.m4 \
	$(srcdir)/configure \
	$(srcdir)/acinclude.m4 \
	$(srcdir)/config.h.in

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

SUBDIRS = config lib image $(GL) fdesign fd2ps $(DEMOS)

# xforms.spec is needed as well as xforms.spec.in or else 'make rpmdist'
# will fail.
EXTRA_DIST = ChangeLog COPYING.LIB Copyright INSTALL README README.OS2 \
	autogen.sh xforms.spec.in xforms.spec

# Hack so that the targets that use tar will also work with automake 1.4
AMTAR ?= $(TAR)

rpmdist: dist
	if [ -z "`type -path rpmbuild`" ]; \
	then \
		RPMBUILD=rpm; \
	else \
		RPMBUILD=rpmbuild; \
	fi; $$RPMBUILD -ta $(PACKAGE)-$(VERSION).tar.gz ; saved_status=$$?; \
	exit $$saved_status
