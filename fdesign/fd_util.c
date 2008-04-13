/*
 *
 * This file is part of XForms.
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
 * \file fd_util.c
 *
 * Eliminate the emission of duplicate info. This is necessary as
 * some #include define data (pixmap for example).
 *
 * We should eventually move the functionality of already_emited in
 * fd_printC.c into this function so callback is also checked. This
 * probably means we need make struct CodeInfo more efficient.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"

#define MAXREC   80
#define MAXLEN   79

typedef struct
{
    FL_FORM * form;
    char      buf[ MAXLEN + 1 ];
} CodeInfo;

static CodeInfo info[ MAXREC ];
static int ninfo;


/***************************************
 ***************************************/

void
reset_dupinfo_cache( void )
{
    ninfo = 0;
}


/***************************************
 ***************************************/

int
is_duplicate_info( FL_OBJECT  * ob  FL_UNUSED_ARG,
				   const char * s )
{
    int i;

    for ( i = 0; i < ninfo; i++ )
		if ( strcmp( s, info[ i ].buf ) == 0 )
			return 1;

    if ( ninfo == MAXREC )
    {
		fprintf( stderr, "dupinfo cache overflown\n" );
		ninfo--;
    }

    strncpy( info[ ninfo ].buf, s, MAXLEN );
    info[ ninfo ].buf[ MAXLEN ] = '\0';

    ninfo++;

    return 0;
}
