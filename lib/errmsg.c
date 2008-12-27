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
 * You should have received a copy of the GNU Lesser General Public License
 * along with XForms.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * \file errmsg.c
 *
 *.  Copyright(c) 1993,1994 by T.C. Zhao
 *   All rights reserved.
 *.
 *
 *  Error handling routines.
 *
 *  Messages are divided into graphics and non-graphics types.
 *  For graphical error messages, a user input may be demanded,
 *  while for non-graphical messages, a string is printed and
 *  does nothing else.
 *
 *  The graphical output routine must have the following form:
 *    void (*gmout)(const char *, const char *, const char *, int);
 ***********************************************************************/

#if ! defined lint && defined F_ID
char *id_errm = "$Id: errmsg.c,v 1.16 2008/12/27 22:20:48 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>
#include "local.h"  /* up stairs */

#include "include/forms.h"
#include "flinternal.h"
#include "private/flsnprintf.h"
#include "ulib.h"

extern int errno;		/* system error no            */

#ifndef HAVE_STRERROR
extern char *sys_errlist[ ];
#endif

#define MAXESTR 2048		/* maximum error string len   */


/**********************************************************************
 * msg_threshold controls amount of message to print
 * 0: only error      1: error and warning
 * 2: info            3: more info
 * 4: debugging       5: trace
 **********************************************************************/

/************ Local variables ****************************************/

static FILE *errlog;		/* where the msg is going       */
static int threshold;		/* current threshold            */
static int req_level;		/* requested message level      */
static const char *file;	/* source file name             */
static int lineno;		    /* line no. in that file        */
static int gout;		    /* if demand graphics           */
static Gmsgout_ gmout;


/***************************************
 * set up where err is gonna go 
 ***************************************/

void
fl_set_err_logfp( FILE * fp )
{
    if ( fp )
		errlog = fp;
}


/***************************************
 ***************************************/

void
fl_set_error_handler( FL_ERROR_FUNC user_func)
{
    user_error_function_ = user_func;
}


/***************************************
 ***************************************/

const char *fli_get_syserror_msg( void )
{
    const char  *pp;

#ifdef HAVE_STRERROR
    pp = errno ? strerror( errno ) : "";
#else
    pp = errno ? sys_errlist[ errno ] : "";
#endif
    return pp;
}


/***************************************
 * Message levels
 ***************************************/

void
fli_set_msg_threshold( int mlevel )
{
    threshold = mlevel;
}


/***************************************
 * Graphics output routine
 ***************************************/

void
fli_set_err_msg_func( Gmsgout_ errf )
{
    gmout = errf;
}


FL_ERROR_FUNC efp_;			/* global pointer to shut up lint */
FL_ERROR_FUNC user_error_function_;  /* hooks for application error handler */


/********************************************************************
 * generate two strings that contain where and why an error occured
 *********************************************************************/

static void
P_errmsg( const char * func,
		  const char * fmt,
		  ... )
{
    va_list args;
    char *where,
		 *why,
		 line[ 20 ];
    const char *pp;
    static char emsg[ MAXESTR + 1 ];

    if ( ! errlog )
		errlog = stderr;

    /* if there is nothing to do, do nothing ! */

#if 0
    if ( req_level >= threshold && ( ! gout || ! gmout ) )
#else
    /*
     * by commenting out gout, graphics output is also controled by threshold
     */
    if ( req_level >= threshold )
#endif
    {
        errno = 0;
		return;
    }

	/*
	 * where comes in two varieties, one is to print everthing, i.e.,
	 * 1. FUNC [file, lineno]: why an error occured.
	 * 2. why the mesage is printed
	 *
	 * If func passed is null, 2 will be used else 1 will be used.
	 */

    if ( func )
    {
		if ( lineno > 0 )
			sprintf( line, "%d", lineno );
		else
			strcpy( line, "?" );

		where = *func ?
				fli_vstrcat( "In ", func, " [", file, ":", line, "] ",
							 ( char * ) 0 ) :
				fli_vstrcat( "In [", file, ":", line, "]: ", ( char * ) 0 );
    }
    else
    {
		line[ 0 ] = '\0';
		where = strdup( "" );
    }

    /* now find out why */

    emsg[ 0 ] = '\0';
    why = 0;

    /* parse the fmt */

    if ( fmt && *fmt )
	{
		va_start( args, fmt );
		fl_vsnprintf( emsg, sizeof emsg - 5, fmt, args );
		va_end( args );
	}

    /* check if there is any system errors */

    if ( ( pp = fli_get_syserror_msg( ) ) && *pp )
    {
		strncat( strcat( emsg, " -- " ), pp, MAXESTR );
        emsg[ MAXESTR - 1 ] = '\0';
    }

    why = emsg;

	/* have gotten the message, where and why, show it */

    if ( req_level < threshold )
		fprintf( errlog, "%s%s\n", where, why );

    if ( gout && gmout )
		gmout( "Warning", where, why,  0 );

    fli_free_vstrcat( where );

    /* reset system errors */

    errno = 0;
    return;
}


/*********************************************************************
 * get the line number in file where error occurred. gui indicates
 * if graphics output function is to be called
 ********************************************************************/

FL_ERROR_FUNC
fli_whereError( int          gui,
				int          level,
				const char * f,
				int          l )
{
    file = f;
    lineno = l;
    req_level = level;
    gout = gui;

    return user_error_function_ ? user_error_function_ : P_errmsg;
}
