#!/bin/sh

LIBTOOLIZE="libtoolize --force --copy"
ACLOCAL="aclocal"
AUTOHEADER="autoheader"
AUTOMAKE="automake -a -c --foreign"
AUTOCONF="autoconf"
ACINCLUDE_FILES="libtool.m4 xformsinclude.m4 cygwin.m4"

# Make sure there's no file named 'libtool.m4' in the 'config'
# subdirectory, otherwise there might be trouble (wrong content
# of the file for the libtool version installed on the machine
# and subsequent failure to create a working'configure' script)

rm -f config/libtool.m4

# Let libtoolize do its magic, preparing everything for
# working with libtool.

if ( $LIBTOOLIZE --version ) < /dev/null > /dev/null 2>&1; then
	echo "Running libtoolize"
	$LIBTOOLIZE > /dev/null
else
	echo "-> libtoolize not found, aborting"
	exit
fi

# Older versions of libtoolize don't copy libtool.m4, so we try to get
# it manually from the directory where aclocal should have stored it

if [ ! -e config/libtool.m4 ]; then
	echo "Copying file libtool.m4"
	if ( $ACLOCAL --version ) < /dev/null > /dev/null 2>&1; then
		LM4=$($ACLOCAL --print-ac-dir)/libtool.m4
		if [ -e $LM4 ]; then
			cp $LM4 config
		else
			echo "-> Can't find required file $LM4"
			exit
		fi
	else
		echo "-> aclocal not found, aborting"
		exit
	fi
fi

# Check that the version of autoconf we have is supported

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
echo '  ./configure && make && sudo make install'
