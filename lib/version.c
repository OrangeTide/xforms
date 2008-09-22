/*
 *
 *  This file is part of the XForms library package.
 *
 * XForms is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1, or
 * (at your option) any later version.
 *
 * XForms is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with XForms; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */


/**
 * \file version.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao
 *  All rights reserved.
 *.
 *
 *  Version info
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_ver = "$Id: version.c,v 1.18 2008/09/22 22:31:28 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "include/forms.h"
#include "flinternal.h"
#include "private/flsnprintf.h"


static const char *version =
    "(Compiled " __DATE__ ")\n"
#ifdef FL_WIN32
    "Copyright (c) 1996-2002 by T.C. Zhao, Gang Li and Mark Overmars\n"
#else
    "Copyright (c) 1996-2002 by T.C. Zhao and Mark Overmars\n"
#endif
    "Parts Copyright(c) 1999-2002 by T.C. Zhao and Steve Lamont\n"
	"GNU Lesser General Public License since 2002";


/***************************************
 ***************************************/

int
fl_library_version( int * ver,
					int * rev )
{
    if ( ver )
		*ver = FL_VERSION;
    if ( rev )
		*rev = FL_REVISION;

    return FL_VERSION * 1000 + FL_REVISION;
}


/***************************************
 ***************************************/

void
fli_print_version( int in_window )
{
	char *msg = fl_malloc(   strlen( version ) + sizeof "FORMS Library Version "
						   + 30 );

	sprintf( msg, "FORMS Library Version %d.%d.%s\n%s",
			 FL_VERSION, FL_REVISION, FL_FIXLEVEL, version );

    if ( in_window )
		fl_show_messages( msg );
	else
		fprintf( stderr, "%s\n", msg );

	fl_free( msg );
}
