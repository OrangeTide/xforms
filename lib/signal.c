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
 * \file signal.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao
 *  All rights reserved.
 *.
 *
 *
 *  Simple minded quick&dirty signal handling.
 *
 *  Only permit a specific signal to have one handler
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include <stdlib.h>
#include <signal.h>


/***************************************
 ***************************************/

static void
handle_signal( void )
{
    FL_SIGNAL_REC *rec = fl_context->signal_rec;

    for ( ; rec; rec = rec->next )
		if ( rec->expired )
		{
			rec->expired = 0;
			rec->callback( rec->signum, rec->data );
		}
}


#ifndef FL_WIN32
#define IsDangerous( s ) (    ( s ) == SIGBUS   \
						   || ( s ) == SIGSEGV  \
						   || ( s ) == SIGILL   \
						   || ( s ) == SIGFPE )
#else
#define IsDangerous( s )  0
#endif

static int sig_direct;


/***************************************
 ***************************************/

static RETSIGTYPE
default_signal_handler( int sig )
{
    fl_signal_caught( sig );

    /* if the signal is one of those that can cause infinite loops, handle it
       and then exit */

    if ( IsDangerous( sig ) )
    {
		handle_signal( );
		fprintf( stderr, "Can't continue upon receiving signal %d\n", sig );
		exit( sig );
    }

#ifndef RETSIGTYPE_IS_VOID
    return 0;
#endif
}


/***************************************
 ***************************************/

void
fl_add_signal_callback( int                 s,
						FL_SIGNAL_HANDLER   cb,
						void              * data )
{
    FL_SIGNAL_REC *sig_rec,
		          *rec = fl_context->signal_rec;

    if ( ! fl_handle_signal )
		fl_handle_signal = handle_signal;

    for ( ; rec && rec->signum != s; rec = rec->next )
		/* empty */ ;

    if ( rec )
    {
		rec->data = data;
		rec->callback = cb;
    }
    else
    {
		sig_rec = fl_calloc( 1, sizeof *sig_rec );
		sig_rec->next = NULL;
		sig_rec->data = data;
		sig_rec->callback = cb;
		sig_rec->signum = s;

		if ( ! sig_direct )
		{
			errno = 0;
			sig_rec->ocallback = signal( s, default_signal_handler );
			if ( sig_rec->ocallback == ( FL_OSSIG_HANDLER ) - 1L || errno )
			{
				M_err( "AddSignal", "Can't add" );
				fl_free( sig_rec );
				return;
			}
		}

		if ( fl_context->signal_rec )
			sig_rec->next = fl_context->signal_rec;
		fl_context->signal_rec = sig_rec;
    }
}


/***************************************
 ***************************************/

void
fl_remove_signal_callback( int s )
{
    FL_SIGNAL_REC *last,
		          *rec = fl_context->signal_rec;

    for ( last = rec; rec && rec->signum != s; last = rec, rec = rec->next )
		/* empty */ ;

    if ( ! rec )
    {
		M_err( "fl_remove_signal_callback", "No handler exists for signal %d",
			   s );
		return;
	}

	if ( rec == fl_context->signal_rec )
		fl_context->signal_rec = rec->next;
	else
		last->next = rec->next;

	fl_addto_freelist( rec );
	if ( ! sig_direct )
		signal( s, rec->ocallback );
}


/***************************************
 ***************************************/

void
fl_signal_caught( int s )
{
    FL_SIGNAL_REC *rec = fl_context->signal_rec;

    for ( ; rec && rec->signum != s; rec = rec->next )
		/* empty */ ;

    if ( ! rec )
	{
		M_err( "fl_signal_caught", "Caught bogus signal %d", s );
		return;
	}

	rec->expired = 1;
	if ( ! sig_direct && ! IsDangerous( s ) )
		signal( s, default_signal_handler );
}


/***************************************
 ***************************************/

void
fl_app_signal_direct( int y )
{
    if ( ! fl_handle_signal )
		fl_handle_signal = handle_signal;
    sig_direct = y;
}
