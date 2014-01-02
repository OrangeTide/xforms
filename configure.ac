dnl Process with autoconf to generate configure script   -*- sh -*-

AC_PREREQ(2.59)
AC_INIT([xforms],[1.2.0],[xforms-development@nongnu.org])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(lib/forms.c)
XFORMS_CHECK_VERSION

AC_CANONICAL_TARGET
AC_CONFIG_HEADERS([lib/config.h])
AM_INIT_AUTOMAKE($PACKAGE_NAME, $PACKAGE_VERSION)
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([config])

### This defines the version number of the installed .so files
### using libtool's versioning system.

AC_SUBST(SO_VERSION, ["3:0:1"])

# Fix the value of the prefixes.

test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

# Check if the bwc-bs-hack is to be used

AC_ARG_ENABLE(bwc_bs_hack,
 [ AS_HELP_STRING( [--enable-bwc-bs-hack],
  [enable backward compatiblity hack for browser, scrollbar and tabfolder objects])])
 AM_CONDITIONAL(USE_BWC_BS_HACK,
 test x$enable_bwc_bs_hack = xyes)


# Check if old style edit keys are to be used

AC_ARG_ENABLE(classic_editkeys,
 [AS_HELP_STRING([--enable-classic-editkeys],
  [enable backward compatiblity for behaviour of input edit keys])])
 AM_CONDITIONAL(USE_CLASSIC_EDITKEYS, test x$enable_classic_editkeys = xyes)


# Check if demos are to be build

AC_ARG_ENABLE(demos,
 [AS_HELP_STRING([--enable-demos],
  [Build demo programs])])
 AM_CONDITIONAL(BUILD_DEMOS, test x$enable_demos = xyes)

# Check if docs are to be build

AC_ARG_ENABLE(docs,
 [AS_HELP_STRING([--enable-docs], 
  [build documentation (needs makeinfo)])])
 AM_CONDITIONAL(BUILD_DOCS, test x$enable_docs = xyes)

# If docs are to be built check for presence of texi2html and pdftex (or
# similar), needed for building the html and pdf versions of the docs,
# respectively

if test BUILD_DOCS; then
  XFORMS_CHECK_TEXI2HTML
  XFORMS_CHECK_TEXI2DVI
fi

AM_CONDITIONAL([BUILD_HTML_DOCS],[test x$TEXI2HTML != xno])
AM_CONDITIONAL([BUILD_PDF_DOCS],[test x$TEXI2DVI != x"no"])

### Check for programs

XFORMS_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET

#### AC_PROG_RANLIB   has become obsolete    

### Check for special systems

AC_ISC_POSIX
AC_AIX

### Check for some Cygwin-specific details.

CHECK_WITH_CYGWIN

# Work around a problem in automake 1.4: when invoking install-strip,
# INSTALL_PROGRAM is changed to 'install -s', and since
# INSTALL_SCRIPT==INSTALL_PROGRAM, we get errors with fileutils-4.0
# which returns an error condition when stripping fails.

INSTALL_SCRIPT='${INSTALL}'

### Setup libtool

AC_PROG_LIBTOOL

#AC_REQUIRE([AC_CANONICAL_HOST])_LT_SET_OPTION([LT_INIT],[win32-dll])
#AC_DIAGNOSE([obsolete],[AC_LIBTOOL_WIN32_DLL: Remove this warning and the call to _LT_SET_OPTION when you
#put the `win32-dll' option into LT_INIT's first parameter.])

LT_INIT([])

### Add extra directories to check for libraries.

XFORMS_WITH_DIR([extra-lib],[extra library directory],extra_lib, NONE)
XFORMS_LOOP_DIR($xforms_cv_extra_lib,XFORMS_ADD_LIB_DIR(xforms_ldflags,$dir))
test ! x"$xforms_ldflags" = x && LDFLAGS="$xforms_ldflags $LDFLAGS"

### Add extra directories to check for include files.

XFORMS_WITH_DIR([extra-inc],[extra include directory],extra_inc, NONE)
XFORMS_LOOP_DIR($xforms_cv_extra_inc,XFORMS_ADD_INC_DIR(xforms_cppflags,$dir))
test ! x"$xforms_cppflags" = x && CPPFLAGS="$xforms_cppflags $CPPFLAGS"

### Add both includes and libraries

XFORMS_WITH_DIR([extra-prefix],[extra lib+include directory],extra_prefix, NONE, ${prefix})
XFORMS_LOOP_DIR($xforms_cv_extra_prefix,[
XFORMS_ADD_INC_DIR(CPPFLAGS,$dir/include)
  XFORMS_ADD_LIB_DIR(LDFLAGS,$dir/lib)])

# Checks for typedefs, structures, and compiler characteristics.

AC_C_CONST  dnl used
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for libraries.

dnl by testing these we check if it is ok to have
dnl -lc and -lm as args to the compiler
AC_CHECK_LIB(m, sin)
AC_CHECK_LIB(c, fopen)

# Checks for header files.

AC_CHECK_HEADERS([sys/select.h])

# Check whether we want to build the gl code

AC_ARG_ENABLE(gl,
  [AS_HELP_STRING([--disable-gl],[Do not build GL support library])])
if test x$enable_gl != xno ; then
  AC_CHECK_HEADER([GL/glx.h],, [enable_gl=no])
fi
AM_CONDITIONAL(BUILD_GL, [test x$enable_gl != xno])


dnl we have some code in lib/listdir.c that could use that...
dnl AC_HEADER_DIRENT

# Check for X, XPM and JPEG

AC_PATH_XTRA
XFORMS_PATH_XPM
XFORMS_CHECK_LIB_JPEG

# Checks for library functions.

AC_TYPE_SIGNAL
if test $ac_cv_type_signal = "void" ; then
  AC_DEFINE(RETSIGTYPE_IS_VOID, 1, [Define if the return type of signal handlers is void])
fi
AC_CHECK_FUNCS([snprintf strcasecmp strerror usleep nanosleep vsnprintf vasprintf sigaction])
XFORMS_CHECK_DECL(snprintf, stdio.h)
XFORMS_CHECK_DECL(vsnprintf, stdio.h)
XFORMS_CHECK_DECL(vasprintf, stdio.h)
XFORMS_CHECK_DECL(sigaction, signal.h)

### Emit some information on what just happened

VERSION_INFO="Configuration:\n
  Host type:                      ${host}
  Special build flags:           ${xforms_flags}
  C Compiler:                     ${CC}
  C Compiler flags:               ${CFLAGS}
  Linker flags:                   ${LDFLAGS}\n"


AC_SUBST(X_LIBS)
AC_SUBST(X_PRE_LIBS)

## Some config.h stuff

AH_TOP([
/*
 *  This file is part of the XForms library package.
 *
 *  XForms is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation; either version 2.1, or
 *  (at your option) any later version.
 *
 *  XForms is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with XForms; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 *  MA 02111-1307, USA.
 */

/* \file config.h
 *
 * This is the compilation configuration file for xforms.
 * It was generated by autoconfs configure.
 * You might want to change some of the defaults if something goes wrong
 * during the compilation.
 */

#ifndef CONFIG_H
#define CONFIG_H
])


AH_BOTTOM([

#endif /* NOT CONFIG_H */
])

### Finish the work.

AC_CONFIG_FILES([Makefile \
       libforms.spec \
       lib/include/AAA.h \
       config/Makefile \
       lib/Makefile \
       lib/bitmaps/Makefile \
       lib/fd/Makefile \
       lib/include/Makefile \
       lib/private/Makefile \
       image/Makefile
       gl/Makefile \
       fdesign/Makefile \
       fdesign/fd/Makefile \
       fdesign/spec/Makefile \
       fdesign/xpm/Makefile \
       fd2ps/Makefile \
       fd2ps/test/Makefile \
       demos/Makefile \
       demos/fd/Makefile \
	   doc/Makefile \
	   doc/xforms_images/Makefile
])

AC_OUTPUT

# show version information

echo
printf "$VERSION_INFO"
echo

# Display a final warning if there has been a XFORMS_ERROR

XFORMS_CHECK_ERRORS
