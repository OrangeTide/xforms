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
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \file vstrcat.c
 *
 *   Copyright(c) 1993,1994 by T.C. Zhao
 *   All rights reserved.
 *
 *   Similar to strcat, but takes variable number of argument
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "include/forms.h"
#include "flinternal.h"
#include "ulib.h"


/***************************************
 ***************************************/

char *
fli_vstrcat( const char * s1,
			 ... )
{
    size_t total = 0;
    char *ret,
		 *p;
    va_list ap;

    if ( ! s1 )
		return NULL;

    total = strlen( s1 );

    /* record total length */

    va_start( ap, s1 );
    while ( ( p = va_arg( ap, char * ) ) )
		total += strlen( p );
    va_end( ap );

    if ( ! ( ret = fl_malloc( total + 1 ) ))
		return NULL;

    strcpy( ret, s1 );
    va_start( ap, s1 );
    while ( ( p = va_arg( ap, char * ) ) )
		strcat( ret, p );
    va_end( ap );

    return ret;
}


/***************************************
 ****** so to protect from M_DBG *******
 ***************************************/

void
fli_free_vstrcat( void * p )
{
    fl_safe_free( p );
}
