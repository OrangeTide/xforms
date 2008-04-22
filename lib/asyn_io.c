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
 * \file asyn_io.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao
 *  All rights reserved.
 *.
 *
 *
 *  Handle input other than the X event queue. Mostly maintanance
 *  here. Actual input/output handling is triggered in the main loop
 *  via fl_watch_io.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include <sys/types.h>

#ifndef FL_WIN32
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef __sgi
#include <bstring.h>
#endif

#ifdef FL_WIN32
#include <X11/Xpoll.h>
#endif


static fd_set st_rfds,
              st_wfds,
              st_efds;


static void fl_add_to_freelist( FL_IO_REC * io );
static void fl_clear_freelist( void );

/***************************************
 * collect all the fd_sets so we don't do it inside the
 * critical select inner loop
 ***************************************/

static void
collect_fd( void )
{
    FL_IO_REC *p;
    int nf = 0;

    /* initialize the sets */

    FD_ZERO( &st_rfds );
    FD_ZERO( &st_wfds );
    FD_ZERO( &st_efds );

    /* loop through all requested IOs */

    for ( p = fl_context->io_rec; p; p = p->next )
    {
		if ( p->source < 0 )
		{
			M_err( "select", "source < 0\n" );
			continue;
		}

		if ( p->mask & FL_READ )
			FD_SET( p->source, &st_rfds );
		if ( p->mask & FL_WRITE )
			FD_SET( p->source, &st_wfds );
		if ( p->mask & FL_EXCEPT )
			FD_SET( p->source, &st_efds );
		if ( nf < p->source + 1 )
			nf = p->source + 1;
    }

    fl_context->num_io = nf;
}


/***************************************
 * Register a callback function for file descriptor fd
 ***************************************/

void
fl_add_io_callback( int              fd,
					unsigned int     mask,
					FL_IO_CALLBACK   callback,
					void           * data )
{
    FL_IO_REC *io_rec;

    /* create new record and make it the start of the list */

    io_rec = fl_malloc( sizeof *io_rec );
    io_rec->next     = fl_context->io_rec;
    io_rec->callback = callback;
    io_rec->data     = data;
    io_rec->source   = fd;
    io_rec->mask     = mask;

    fl_context->io_rec = io_rec;

	collect_fd( );
}


/***************************************
 ***************************************/

void
fl_remove_io_callback( int            fd,
					   unsigned       int mask,
					   FL_IO_CALLBACK cb )
{
    FL_IO_REC *io,
		      *last;

    for ( last = io = fl_context->io_rec;
		  io && ! ( io->source == fd && io->callback == cb && io->mask & mask );
		  last = io, io = io->next )
		/* empty */ ;

	if ( ! io )
    {
		M_err( "fl_remove_io_callback", "Non-existent handler for %d", fd );
		return;
	}

	io->mask &= ~mask;

	/* Special case: if after removal fd does not do anything
	   anymore, i.e. mask == 0, remove it from global record */

	if ( io->mask == 0 )
	{
		if ( io == fl_context->io_rec )
			fl_context->io_rec = io->next;
		else
			last->next = io->next;

		fl_add_to_freelist( io );
	}

	collect_fd( );
}


/***************************************
 * Watch for activities using select or poll. Timeout in milli-second
 ***************************************/

void
fl_watch_io( FL_IO_REC * io_rec,
			 long        msec )
{
    fd_set rfds,
		   wfds,
		   efds;
    struct timeval timeout;
    FL_IO_REC *p;
    int nf;

	fl_clear_freelist( );

    if ( ! io_rec )
    {
		if ( msec > 0 )
			fl_msleep( msec );
		return;
    }

    timeout.tv_usec = 1000 * ( msec % 1000 );
    timeout.tv_sec  = msec / 1000;

    /* initialize the sets */

    rfds = st_rfds;
    wfds = st_wfds;
    efds = st_efds;

    /* now watch it. HP defines rfds to be ints. Althought compiler will
       bark, it is harmless. */

	nf = select( fl_context->num_io, &rfds, &wfds, &efds, &timeout );

    if ( nf < 0 )     /* something is wrong. */
    {
		if ( errno == EINTR )
			M_warn( "WatchIO", "select interrupted by signal" );

		/* select() on some platforms returns -1 with errno == 0 */

		else if ( errno != 0 )
			M_err( "select", fl_get_syserror_msg( ) );
    }

    /* time expired */

    if ( nf <= 0 )
		return;

    /* handle it */

    for ( p = io_rec; p; p = p->next )
    {
		if ( ! p->callback || p->source < 0 || p->mask == 0 )
			continue;

		if ( p->mask & FL_READ && FD_ISSET( p->source, &rfds ) )
			p->callback( p->source, p->data );

		if ( p->mask & FL_WRITE && FD_ISSET( p->source, &wfds ) )
			p->callback( p->source, p->data );

		if ( p->mask & FL_EXCEPT && FD_ISSET( p->source, &efds ) )
			p->callback( p->source, p->data );
    }

	fl_clear_freelist( );
}


/***************************************
 ***************************************/

int
fl_is_watched_io( int fd )
{
    FL_IO_REC *p;

    for ( p = fl_context->io_rec; p; p = p->next )
		if ( p->source == fd && p->mask )
			return 1;

    return 0;
}


/***************************************
 ***************************************/

typedef struct free_list_
{
	struct free_list_ * next;
	FL_IO_REC         * io;
} Free_List_T;

static Free_List_T *fl = NULL;


static void
fl_add_to_freelist( FL_IO_REC * io )
{
	Free_List_T *cur;

    cur = malloc( sizeof *cur );
	cur->next = fl;
	cur->io   = io;

	fl = cur;
}


/***************************************
 ***************************************/

static void
fl_clear_freelist( void )
{
	Free_List_T *cur;

	while ( fl )
	{
		fl_free( fl->io );
		cur = fl;
		fl = fl->next;
		fl_free( cur );
	}
}
