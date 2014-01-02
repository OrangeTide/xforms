dnl Some useful functions for xforms configure.in                 -*- sh -*-
dnl Author: Jean-Marc Lasgouttes (lasgouttes@lyx.org)
dnl         Lars Gullik Bjønnes (larsbj@lyx.org)
dnl         Allan Rae (rae@lyx.org)
dnl         Jens Thoms Törring (jt@toerring.de)


### Check for version and warn when a development version is used
dnl Usage XFORMS_CHECK_VERSION
dnl Sets variables VERSION, FL_VERSION, FL_REVISION, FL_FIXLEVEL and 
dnl xforms_devel_version.
AC_DEFUN([XFORMS_CHECK_VERSION],[
AC_SUBST(PACKAGE, $PACKAGE_NAME)
AC_SUBST(VERSION, $PACKAGE_VERSION)
[eval] `echo $PACKAGE_VERSION | sed -e 's/\(.*\)\.\(.*\)\.\(.*\)/FL_VERSION=\1; FL_REVISION=\2; FL_FIXLEVEL=\3/'`
AC_SUBST(FL_VERSION)
AC_SUBST(FL_REVISION)
AC_SUBST(FL_FIXLEVEL)
echo "configuring xforms version $VERSION"
xforms_devel_version=no
case $FL_REVISION in
	*[[13579]])  xforms_devel_version=yes
          AC_DEFINE(DEVEL_VERSION, 1, Define if you are building a development version of xforms)
          echo "WARNING: This is a development version. Expect bugs." ;;
esac
])

dnl Usage XFORMS_CHECK_LIB_JPEG: Checks for jpeg library
AC_DEFUN([XFORMS_CHECK_LIB_JPEG],[
### Check for Jpeg library
AC_CHECK_LIB(jpeg, jpeg_CreateDecompress, JPEG_LIB="-ljpeg",
	[XFORMS_LIB_ERROR(libjpeg,jpeg)])
AC_SUBST(JPEG_LIB)
])

dnl Usage XFORMS_PATH_XPM: Checks for xpm library and header
AC_DEFUN([XFORMS_PATH_XPM],[
### Check for Xpm library
SAVE_LIBS="$LIBS"
LIBS="$X_PRE_LIBS $LIBS $X_LIBS -lX11 $X_EXTRA_LIBS"
AC_CHECK_LIB(Xpm, XpmCreateBufferFromImage, XPM_LIB="-lXpm",
	[XFORMS_LIB_ERROR(libXpm,Xpm)])
AC_SUBST(XPM_LIB)
LIBS="$SAVE_LIBS"

### Check for Xpm headers
SAVE_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
xforms_cv_xpm_h_location="<xpm.h>"
AC_CHECK_HEADER(X11/xpm.h,[
  ac_cv_header_xpm_h=yes
  xforms_cv_xpm_h_location="<X11/xpm.h>"],[
AC_CHECK_HEADER(xpm.h,[],[
XFORMS_LIB_ERROR(xpm.h,Xpm)])])
AC_DEFINE_UNQUOTED(XPM_H_LOCATION,$xforms_cv_xpm_h_location,
  [define this to the location of xpm.h to be used with #include, e.g. <xpm.h>])
CPPFLAGS="$SAVE_CPPFLAGS"

### Test for the header version
if test $ac_cv_header_xpm_h = yes; then
  AC_CACHE_CHECK([xpm header version],xforms_cv_xpmversion,
  [ cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"

#include XPM_H_LOCATION
"%%%"xforms_cv_xpmv=XpmVersion;xforms_cv_xpmr=XpmRevision"%%%"
EOF
    eval `(eval "$ac_cpp conftest.$ac_ext") 2>&5 | \
      grep '^"%%%"'  2>/dev/null | \
      sed -e 's/^"%%%"\(.*\)"%%%"/\1/' -e 's/ //g'`
    case "$xforms_cv_xpmr" in
changequote(,)
     [0-9]) xformsxpmr_alpha=`echo $xforms_cv_xpmr |tr 123456789 abcdefghi`
	    xformsxpmv_alpha=" (aka 3.$xforms_cv_xpmv$xformsxpmr_alpha)";;
changequote([,])
	 *) ;;
    esac
    xforms_cv_xpmversion="$xforms_cv_xpmv.$xforms_cv_xpmr$xformsxpmv_alpha"
    rm -f conftest*])
  XPM_VERSION=${xforms_cv_xpmversion}
  case "$xforms_cv_xpmr" in
changequote(,)
	[789]|[0-9][0-9]*) ;;
changequote([,])
	*) XFORMS_WARNING([Version $xforms_cv_xpmversion of the Xpm library is a bit old.
   If you experience strange crashes with xforms, try to upgrade
   to at least version 4.7 (aka 3.4g).
   If you have installed a newer version of the library, check whether you
   have an old xpm.h header file in your include path.]);;
  esac
fi])


dnl Macro tests if 'texi2dvi' is installed and usable. Also test for 'convert'
dnl and checks if PNG files must be converted to PDF files. 

AC_DEFUN([XFORMS_CHECK_TEXI2DVI],[
AC_CHECK_PROGS(TEXI2DVI,[texi2dvi],no)
export TEXI2DVI
AC_CHECK_PROGS(CONVERT,[convert],no)
export CONVERT

dnl If 'texi2dvi' doesn't exist there' nothing we can do. Otherwise we need to
dnl check if the installed version of 'texi2dvi' uses programs that can't deal
dnl with PNG files (as some older versions did). For this we create a minimal
dnl texi file that just contains an image and run 'texi2dvi' on it. If this
dnl fails we check the log file for a line complaining about a missing image.
dnl If such a line doesn't exist 'texi2dvi' must be broken in a way we can't
dnl deal with. Otherwise we need to convert the PNF files to PDF. This in turn
dnl requires 'convert'. If it doesn't exist we can't create the PD
dnl documentation, otherwise we set a variable that tells us in the Makefile
dnl that all PNG files have to be converted to PDF.

if test $TEXI2DVI = "no"
then
	XFORMS_WARNING([Unable to find application 'texi2dvi', PDF documentation won't be built])
else
	cat > texi2dvi_test.texi <<EOF
\input texinfo
@center @image{doc/xforms_images/pushme,5cm}
@bye
EOF
	$TEXI2DVI --pdf --batch texi2dvi_test.texi > /dev/null 2>&1;
	if test $? != 0;
	then
		grep "cannot find image file" texi2dvi_test.log > /dev/null 2>&1
		if test $? = 0;
		then
			if test $CONVERT = "no";
			then
				TEXI2DVI="no"
				XFORMS_WARNING(['texi2dvi' requires the 'convert' application but which doesn't exist, so PDF documentation won't be built])
	        else
	           TEXI2DVI_NEEDS_PDF="yes"
			   export TEXI2DVI_NEEDS_PDF
            fi
		else
			TEXI2DVI="no"
			XFORMS_WARNING([Unable to find a working application 'texi2dvi', PDF documentation won't be built])
		fi
	fi
	rm -f texi2dvi_test.*
fi
AC_SUBST(TEXI2DVI)
AC_SUBST(CONVERT)
AC_SUBST(TEXI2DVI_NEEDS_PDF)
])


dnl Macro tests if texi2html is installed.
dnl
AC_DEFUN([XFORMS_CHECK_TEXI2HTML],[
AC_CHECK_PROGS(TEXI2HTML,[texi2html],no)
export TEXI2HTML;
if test $TEXI2HTML = "no" ;
then
	XFORMS_WARNING([Unable to find a 'texi2html' application, HTML documentation  won't be built]);
fi
AC_SUBST(TEXI2HTML)
])


dnl Usage: XFORMS_ERROR(message)  Displays the warning "message" and sets the
dnl flag xforms_error to yes.
AC_DEFUN([XFORMS_ERROR],[
xforms_error_txt="$xforms_error_txt
** $1
"
xforms_error=yes])


dnl Usage: XFORMS_WARNING(message)  Displays the warning "message" and sets the
dnl flag xforms_warning to yes.
AC_DEFUN([XFORMS_WARNING],[
xforms_warning_txt="$xforms_warning_txt
== $1
"
xforms_warning=yes])


dnl Usage: XFORMS_LIB_ERROR(file,library)  Displays an error message indication
dnl  that 'file' cannot be found because 'lib' may be uncorrectly installed.
AC_DEFUN([XFORMS_LIB_ERROR],[
XFORMS_ERROR([Cannot find $1. Please check that the $2 library
   is correctly installed on your system.])])


dnl Usage: XFORMS_CHECK_ERRORS  Displays a warning message if a XFORMS_ERROR
dnl   has occured previously.
AC_DEFUN([XFORMS_CHECK_ERRORS],[
if test x$xforms_error = xyes; then
cat <<EOF
**** The following problems have been detected by configure.
**** Please check the messages below before running 'make'.
**** (see the section 'Problems' in the INSTALL file)
$xforms_error_txt
$xforms_warning_txt
EOF
else

if test x$xforms_warning = xyes; then
cat <<EOF
=== The following minor issues have been detected by configure.
=== Please check the messages below before running 'make'.
=== (see the section 'Problems' in the INSTALL file)
$xforms_warning_txt
EOF
fi
cat <<EOF
Configuration of xforms was successful.
Type 'make' to compile the program,
and then 'make install' to install it.
EOF
fi])


AC_DEFUN([XFORMS_PROG_CC],
[
AC_LANG_COMPILER(C)


### We might want to get or shut up warnings.

AC_ARG_ENABLE(warnings,
  [  --enable-warnings       tell the compiler to display more warnings],,
  [ if test $xforms_devel_version = yes && test $ac_cv_c_compiler_gnu = yes ; then
	enable_warnings=yes;
    else
	enable_warnings=no;
    fi;])

if test x$enable_warnings = xyes ; then
  xforms_flags="$xforms_flags warnings"
  AC_DEFINE(WITH_WARNINGS, 1,
  [Define this if you want to see the warning directives put here and
   there by the developpers to get attention])
fi

### We might want to disable debug
AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debug information],,
  [ if test $xforms_devel_version = yes && test $ac_cv_c_compiler_gnu = yes ; then
	enable_debug=yes;
    else
	enable_debug=no;
    fi;])

### set up optimization

AC_ARG_ENABLE(optimization,
  [  --enable-optimization[=value]   enable compiler optimisation default=-O],,
	enable_optimization=yes;)
case $enable_optimization in
  yes) xforms_opt=-O;;
   no) xforms_opt=;;
    *) xforms_opt=${enable_optimization};;
esac

# set the debug flags correctly.

if test x$GCC = xyes; then
    if test "$ac_test_CFLAGS" = set; then
	CFLAGS=$ac_save_CFLAGS
    else
	CFLAGS="$xforms_opt"
	if test x$enable_debug = xyes ; then
	    CFLAGS="-g $CFLAGS"
	fi
    fi
    if test x$enable_warnings = xyes ; then
	CFLAGS="$CFLAGS -W -Wall"
    fi
fi
])


dnl Usage: XFORMS_WITH_DIR(dir-name,desc,dir-var-name,default-value,
dnl                       [default-yes-value])
dnl  Adds a --with-'dir-name' option (described by 'desc') and puts the
dnl  resulting directory name in 'dir-var-name'.
AC_DEFUN([XFORMS_WITH_DIR],[
  AC_ARG_WITH($1,[  --with-$1        specify $2])
  AC_MSG_CHECKING([for $2])
  if test -z "$with_$3"; then
     AC_CACHE_VAL(xforms_cv_$3, xforms_cv_$3=$4)
  else
    test "x$with_$3" = xyes && with_$3=$5
    xforms_cv_$3="$with_$3"
  fi
  AC_MSG_RESULT($xforms_cv_$3)])


dnl Usage: XFORMS_LOOP_DIR(value,action)
dnl Executes action for values of variable `dir' in `values'. `values' can
dnl use ":" as a separator.
AC_DEFUN([XFORMS_LOOP_DIR],[
IFS="${IFS=	}"; ac_save_ifs="$IFS"; IFS="${IFS}:"
for dir in `eval "echo $1"`; do
  if test ! "$dir" = NONE; then
    test ! -d "$dir" && AC_MSG_ERROR([\"$dir\" is not a directory])
    $2
  fi
done
IFS=$ac_save_ifs
])


dnl Usage: XFORMS_ADD_LIB_DIR(var-name,dir) Adds a -L directive to variable
dnl var-name.
AC_DEFUN([XFORMS_ADD_LIB_DIR],[
$1="${$1} -L$2"
if test "`(uname) 2>/dev/null`" = SunOS &&
    uname -r | grep '^5' >/dev/null; then
  if test $ac_cv_cxx_compiler_gnu = yes ; then
    $1="${$1} -Wl[,]-R$2"
  else
    $1="${$1} -R$2"
  fi
fi])


dnl Usage: XFORMS_ADD_INC_DIR(var-name,dir) Adds a -I directive to variable
dnl var-name.
AC_DEFUN([XFORMS_ADD_INC_DIR],[$1="${$1} -I$2 "])


dnl this is used by the macro below to general a proper config.h.in entry
m4_define([XFORMS_AH_CHECK_DECL],
[AH_TEMPLATE(AS_TR_CPP(HAVE_DECL_$1),
  [Define if you have the prototype for function `$1'])])

dnl Check things are declared in headers to avoid errors or warnings.
dnl Called like XFORMS_CHECK_DECL(function, header1 header2...)
dnl Defines HAVE_DECL_{FUNCTION}
AC_DEFUN([XFORMS_CHECK_DECL],
[XFORMS_AH_CHECK_DECL($1)
for ac_header in $2
do
  AC_MSG_CHECKING([if $1 is declared by header $ac_header])
  AC_EGREP_HEADER($1, $ac_header,
      [AC_MSG_RESULT(yes)
       AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_DECL_$1))
       break],
      [AC_MSG_RESULT(no)])
done])
