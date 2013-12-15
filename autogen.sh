#!/bin/sh

LIBTOOLIZE="libtoolize --force --copy"
ACLOCAL="aclocal"
AUTOHEADER="autoheader"
AUTOMAKE="automake -a -c --foreign"
AUTOCONF="autoconf"
ACINCLUDE_FILES="libtool.m4 xformsinclude.m4 cygwin.m4"

# Older versions of libtoolize doesn't copy libtool.m4, so we
# provide one from an old libttool version

if [ ! -e config/libtool.m4 ]; then
	cp config/libtool.m4.might_be_needed config/libtool.m4
fi

if ( $LIBTOOLIZE --version ) < /dev/null > /dev/null 2>&1; then
	echo "Running libtoolize"
	$LIBTOOLIZE > /dev/null
else
	echo "-> libtoolize not found, aborting"
	exit
fi

# Check tha the version of autoconf we have is supported

autoversion=`$AUTOCONF --version | head -n 1`

echo "Using $autoversion"
case $autoversion in
    *2.59)
		;;
    *2.[6-9][0-9])
		;;
    *)
		echo "This autoconf version is not supported by XForms."
		echo "Please update to autoconf 2.59 or newer."
		exit
	;;
esac

# Generate acinclude.m4

echo "Generating acinclude.m4"
rm -f acinclude.m4
( cd config; cat ${ACINCLUDE_FILES} ${EXTRA_ACINCLUDE_FILES} > ../acinclude.m4 )

# Generate the Makefiles and the configure file

if ( $ACLOCAL --version ) < /dev/null > /dev/null 2>&1; then
	echo "Building macros"
	$ACLOCAL
else
	echo "-> aclocal not found, aborting"
	exit
fi

if ( $AUTOHEADER --version ) < /dev/null > /dev/null 2>&1; then
	echo "Building config header template"
	$AUTOHEADER
else
	echo "-> autoheader not found, aborting"
	exit
fi

if ( $AUTOMAKE --version ) < /dev/null > /dev/null 2>&1; then
	echo "Building Makefile templates"
	$AUTOMAKE
else
	echo "-> automake not found, aborting"
	exit
fi

# There seem to have been versions where the install-sh script generated
# by automae didn't had execute permissions...

if [ ! -x config/install-sh ]; then
	chmod 755 config/install-sh
fi

if ( $AUTOCONF --version ) < /dev/null > /dev/null 2>&1; then
	echo "Building configure"
	$AUTOCONF
else
	echo "-> autoconf not found, aborting"
	exit
fi

echo
echo "Looks good, now run:"
echo '  ./configure && make && make install'
