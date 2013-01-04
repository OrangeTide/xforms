#!/bin/sh

ACLOCAL="aclocal"
AUTOHEADER="autoheader"
AUTOMAKE="automake -a -c --foreign"
AUTOCONF="autoconf"
ACINCLUDE_FILES="libtool.m4 xformsinclude.m4 cygwin.m4"

# Discover what version of autoconf we are using.
autoversion=`$AUTOCONF --version | head -n 1`

echo "Using $autoversion"
case $autoversion in
    *2.5[12346789])
		;;
    *2.[6789][0123456789])
		;;
    *)
		echo "This autoconf version is not supported by xforms."
		echo "Please update to autoconf 2.51 or newer."
		exit
	;;
esac

# Generate acinclude.m4
echo -n "Generate acinclude.m4... "
rm -f acinclude.m4
( cd config; cat ${ACINCLUDE_FILES} ${EXTRA_ACINCLUDE_FILES} > ../acinclude.m4 )
echo " done"

# Generate the Makefiles and configure files
if ( $ACLOCAL --version ) < /dev/null > /dev/null 2>&1; then
	echo -n "Building macros..."
	$ACLOCAL
	echo " done"
else
	echo "aclocal not found -- aborting"
	exit
fi

if ( $AUTOHEADER --version ) < /dev/null > /dev/null 2>&1; then
	echo -n "Building config header template..."
	$AUTOHEADER
	echo " done"
else
	echo "autoheader not found -- aborting"
	exit
fi

if ( $AUTOMAKE --version ) < /dev/null > /dev/null 2>&1; then
	echo -n "Building Makefile templates..."
	$AUTOMAKE
	echo " done"
else
	echo "automake not found -- aborting"
	exit
fi

if ( $AUTOCONF --version ) < /dev/null > /dev/null 2>&1; then
	echo -n "Building configure..."
	$AUTOCONF
	echo " done"
else
	echo "autoconf not found -- aborting"
	exit
fi

echo
echo 'Now run "./configure; make; make install"'
