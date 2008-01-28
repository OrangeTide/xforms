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
 * \file vstrcat.c
 *
 *.  Copyright(c) 1993,1994 by T.C. Zhao
 *   All rights reserved.
 *.
 *
 *   Similar to strcat, but takes variable number of argument
 *
 */

#if ! defined lint && defined F_ID
char *id_vstrcat = "$Id: vstrcat.c,v 1.6 2008/01/28 23:24:48 jtt Exp $";
#endif

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
vstrcat( const char * s1,
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

    if ( ! ( ret = malloc( total + 1 ) ))
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
free_vstrcat( void * p )
{
    free( p );
}
