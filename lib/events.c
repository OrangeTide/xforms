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
 * \file events.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *  Events handlers for the application window
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_evt = "$Id: events.c,v 1.17 2008/03/26 20:08:27 jtt Exp $";
#endif


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/flsnprintf.h"


static void handle_object( FL_OBJECT * obj );


/***************************************
 * Function returns 1 if the event is consumed so it will never
 * reach the application window event queue
 ***************************************/

int
fl_handle_event_callbacks( XEvent * xev )
{
    Window win = ( ( XAnyEvent * ) xev )->window;
    FL_WIN *fwin;

    for ( fwin = fl_app_win; fwin && fwin->win != win; fwin = fwin->next )
		/* empty */ ;

    if ( ! fwin )
    {
		M_warn( "fl_handle_event_callbacks", "Unknown window = 0x%lx",
				xev->xany.window );
		fl_xevent_name( "Ignored", xev );
		return 1;                         /* Changed from 0  JTT */
    }

    if (    fwin->pre_emptive
		 && fwin->pre_emptive( xev, fwin->pre_emptive_data ) == FL_PREEMPT )
		return 1;

    if ( fwin->callback[ xev->type ] )
    {
		fwin->callback[ xev->type ]( xev, fwin->user_data[ xev->type ] );
		return 1;
    }

    return 0;
}


/*** Global event handlers for all windows ******/

static FL_APPEVENT_CB fl_event_callback;
static void *fl_user_data;


/***************************************
 * Sets the call_back routine for the events
 ***************************************/

FL_APPEVENT_CB
fl_set_event_callback( FL_APPEVENT_CB callback,
					   void *         user_data )
{
    FL_APPEVENT_CB old = fl_event_callback;

    fl_event_callback = callback;
    fl_user_data = user_data;
    return old;
}


/********* End of Application Window management ***********}*****/


/*************** THE OBJECT EVENTS *************{******/
/*************** CALL-BACK ROUTINE HANDLING ***********/

/* Normally, the object queue doesn't have to be large and a default
   size is sufficient. In the case that more objects need to be
   stored the queue isn't increased by just another element but
   instead by the same number of objects we started with, reducing
   the number of calls of malloc() a bit */

#define FL_QSIZE	      64            /* chunk size of object queue */

typedef struct FL_OBJECT_QUEUE_ENTRY_ {
	FL_OBJECT *                     obj;
	struct FL_OBJECT_QUEUE_ENTRY_ * next;
} FL_OBJECT_QUEUE_ENTRY;

typedef struct FL_OBJECT_QUEUE_ {
	FL_OBJECT_QUEUE_ENTRY * head;       /* here objects get added to */
	FL_OBJECT_QUEUE_ENTRY * tail;       /* and here they get removed from */
	FL_OBJECT_QUEUE_ENTRY * empty;      /* linked list of empty entries */
	FL_OBJECT_QUEUE_ENTRY * blocks;     /* pointer to linked list of blocks */
} FL_OBJECT_QUEUE;

static FL_OBJECT_QUEUE obj_queue = { NULL, NULL, NULL, NULL };


/***************************************************
 * Function for creating/extending the object queue
 * (gets called automatically the first time an
 * object gets pushed on the queue, so no previous
 * call, e.g. from fl_initialize(), is necessary)
 ***************************************************/

static void
extend_obj_queue( void )
{
	FL_OBJECT_QUEUE_ENTRY *p = fl_malloc( ( FL_QSIZE + 1 ) * sizeof *p );
	size_t i;

	/* The first element of the (new) area is used from book-keeping purposes */

	p->next = obj_queue.blocks;
	obj_queue.blocks = p++;

	/* The rest gets added to (or makes up) the empty list */

	obj_queue.empty = p;

	for ( i = 0; i < FL_QSIZE - 1; p++, i++ )
		p->next = p + 1;

	p->next = NULL;
}


/******************************************************
 * Fuction for removing the object queue, should be
 * called when all forms and application windows have
 * been closed to get rid of allocated memory.
 ******************************************************/

void
fl_obj_queue_delete( void )
{
	FL_OBJECT_QUEUE_ENTRY *b;

	while ( ( b = obj_queue.blocks ) != NULL )
	{
		obj_queue.blocks = b->next;
		fl_free( b );
	}

	obj_queue.tail = obj_queue.head = obj_queue.empty = NULL;
}


/***********************************************************
 * Function for appending a new element to the object queue
 ***********************************************************/

static void
add_to_obj_queue( FL_OBJECT * obj )
{
	if ( obj == NULL )
		return;

	if ( obj_queue.empty == NULL )
		extend_obj_queue( );

	if ( obj_queue.head )
		obj_queue.head = obj_queue.head->next = obj_queue.empty;
	else
		obj_queue.tail = obj_queue.head = obj_queue.empty;

	obj_queue.empty = obj_queue.empty->next;

	obj_queue.head->next = NULL;
	obj_queue.head->obj = obj;
}


/*****************************************************************
 * Function for fetching the oldest element form the object queue
 *****************************************************************/

static FL_OBJECT *
get_from_obj_queue( void )
{
	FL_OBJECT_QUEUE_ENTRY *t = obj_queue.tail;

	if ( t == NULL )
		return NULL;

	if ( t->next == NULL )
		obj_queue.tail = obj_queue.head = NULL;
	else
		obj_queue.tail = t->next;

	t->next = obj_queue.empty;
	obj_queue.empty = t;

	return t->obj;
}
	

/*************************************************************************
 * Function for removing all entries for a certain object from the queue.
 * This routine is called as part of the deletion of an object.
 *************************************************************************/

void
fl_object_qflush_object( FL_OBJECT * obj )
{
	FL_OBJECT_QUEUE_ENTRY *c,
		                  *p;

	while ( obj_queue.tail && obj_queue.tail->obj == obj )
		get_from_obj_queue( );

	if ( ! obj_queue.tail )
		return;

	p = obj_queue.tail;
	c = p->next;

	while ( c )
	{
		if ( c->obj == obj )
		{
			p->next = c->next;
			c->next = obj_queue.empty;
			obj_queue.empty = c;
		}
		else
			p = c;

		c = p->next;
	}
}


/**********************************************************************
 * Function for removing all entries for a certain form from the queue
 * - here the object handler must be executed for FL_INPUT objects.
 * This should be called as part of free_form process.
 **********************************************************************/

void
fl_object_qflush( FL_FORM * form )
{
	FL_OBJECT_QUEUE_ENTRY *c,
		                  *p;

	while (    obj_queue.tail
			&& obj_queue.tail->obj != FL_EVENT
			&& obj_queue.tail->obj->form == form )
	{
		if ( obj_queue.tail->obj->objclass == FL_INPUT )
			handle_object( obj_queue.tail->obj );
		get_from_obj_queue( );
	}

	if ( ! obj_queue.tail )
		return;

	for ( p = obj_queue.tail, c = p->next; c != NULL; c = p->next )
		if ( c->obj != FL_EVENT && c->obj->form == form )
		{
			if ( c->obj->objclass == FL_INPUT )
				handle_object( c->obj );

			p->next = c->next;
			c->next = obj_queue.empty;
			obj_queue.empty = c;
		}
		else
			p = c;
}


/***************************************
 * Adds an object to the queue
 ***************************************/

void
fl_object_qenter( FL_OBJECT * obj )
{
	if ( ! obj )
	{
		M_err( "Qenter", "NULL object" );
		return;
	}

#ifndef DELAYED_ACTION

    if (    obj != FL_EVENT
		 && ( ! obj->form || ! obj->visible || obj->active <= 0 ) )
    {
#if FL_DEBUG >= ML_DEBUG
		M_err( "Qenter", "Bad object" );
#endif
		return;
    }

	if ( obj != FL_EVENT )
	{
		if ( obj->object_callback )
		{
			XFlush( flx->display );
			obj->object_callback( obj, obj->argument );
			return;
		}
		else if ( obj->form->form_callback )
		{
			XFlush( flx->display );
			obj->form->form_callback( obj, obj->form->form_cb_data );
			return;
		}
    }
#endif /* ! DELAYED_ACTION */

	add_to_obj_queue( obj );
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_object_qtest( void )
{
    if ( obj_queue.tail == NULL )
		return NULL;
    return obj_queue.tail->obj;
}


/***************************************
 * reads an object from the queue.
 ***************************************/

FL_OBJECT *
fl_object_qread( void )
{
    FL_OBJECT *obj = get_from_obj_queue( );

	if ( ! obj || obj == FL_EVENT )
		return obj;

	if ( ! obj->form )
		return NULL;

	if ( obj->object_callback )
	{
		obj->object_callback( obj, obj->argument );
		return NULL;
	}
	else if ( obj->form->form_callback )
	{
		obj->form->form_callback( obj, obj->form->form_cb_data );
		return NULL;
	}

    return obj;
}


/***************************************
 * this is mainly used to handle the input correctly when a form
 * is being hidden
 ***************************************/

static void
handle_object( FL_OBJECT * obj )
{
	if ( obj != FL_EVENT || ! obj->form )
		return;

    if ( obj->object_callback )
		obj->object_callback( obj, obj->argument );
    else if ( obj->form->form_callback )
		obj->form->form_callback( obj, obj->form->form_cb_data );
}


/** End of object flush ************* */


/***************** End of object Q *****************/



/**************** Normal Events ********************/

typedef struct FL_EVENT_QUEUE_ENTRY_ {
	XEvent                         xev;
	struct FL_EVENT_QUEUE_ENTRY_ * next;
} FL_EVENT_QUEUE_ENTRY;

typedef struct FL_EVENT_QUEUE_ {
	FL_EVENT_QUEUE_ENTRY * head;       /* here events get added to */
	FL_EVENT_QUEUE_ENTRY * tail;       /* and here they get removed from */
	FL_EVENT_QUEUE_ENTRY * empty;      /* linked list of empty entries */
	FL_EVENT_QUEUE_ENTRY * blocks;     /* pointer to linked list of blocks */
	unsigned long          count;
} FL_EVENT_QUEUE;

static FL_EVENT_QUEUE event_queue = { NULL, NULL, NULL, NULL, 0 };


/***************************************************
 * Function for creating/extending the event queue
 * (gets called automatically the first time an
 * event gets pushed on the queue, so no previous
 * call, e.g. from fl_initialize(), is necessary)
 ***************************************************/

static void
extend_event_queue( void )
{
	FL_EVENT_QUEUE_ENTRY *p = fl_malloc( ( FL_QSIZE + 1 ) * sizeof *p );
	size_t i;

	/* The first element of the area gets used from book-keeping purposes */

	p->next = event_queue.blocks;
	event_queue.blocks = p++;

	/* The rest gets added to (or makes up) the empty list */

	event_queue.empty = p;

	for ( i = 0; i < FL_QSIZE - 1; p++, i++ )
		p->next = p + 1;

	p->next = NULL;
}


/******************************************************
 * Fuction for removing the event queue, should be
 * called when all forms and application windows have
 * been closed to get rid of allocated memory.
 ******************************************************/

void
fl_event_queue_delete( void )
{
	FL_EVENT_QUEUE_ENTRY *b;

	while ( ( b = event_queue.blocks ) != NULL )
	{
		event_queue.blocks = b->next;
		fl_free( b );
	}

	event_queue.tail = event_queue.head = event_queue.empty = NULL;
}


/***********************************************************
 * Function for appending a new element to the event queue
 ***********************************************************/

static void
add_to_event_queue( XEvent *xev )
{
	if ( event_queue.empty == NULL )
		extend_event_queue( );

	if ( event_queue.head )
		event_queue.head = event_queue.head->next = event_queue.empty;
	else
		event_queue.tail = event_queue.head = event_queue.empty;

	event_queue.empty = event_queue.empty->next;

	event_queue.head->next = NULL;
	event_queue.head->xev = *xev;
	event_queue.count++;
}


/****************************************************************
 * Function for removing the oldest element form the event queue
 ****************************************************************/

static XEvent
get_from_event_queue( void )
{
	FL_EVENT_QUEUE_ENTRY *t = event_queue.tail;

	if ( t->next == NULL )
		event_queue.tail = event_queue.head = NULL;
	else
		event_queue.tail = t->next;

	t->next = event_queue.empty;
	event_queue.empty = t;

	return t->xev;
}
	

/***************************************
 * Replacement for the Xlib XPutBackEvent() function: allows to
 * push back an event onto the queue
 ***************************************/

void
fl_XPutBackEvent( XEvent * xev )
{
    static int mm;

    if ( fl_handle_event_callbacks( xev ) )
		return;

    if ( fl_event_callback )
    {
		fl_event_callback( xev, fl_user_data );
		return;
    }

    /* these must be from simulating double buffering, throw them away */

    if ( xev->type == NoExpose )
    {
		if ( ++mm % 20 == 0 )
			M_warn( "XPutbackEvent", "20 NoExpose discarded" );
		return;
    }

    fl_xevent_name( "fl_XPutBackEvent", xev );
	add_to_event_queue( xev );
}


/***************************************
 * Replacement for the Xlib XEventsQueued() function: returns
 * if there are any events in the event queue.
 ***************************************/

int
fl_XEventsQueued( int mode  FL_UNUSED_ARG )
{
    if ( event_queue.tail == NULL )
    {
		fl_treat_interaction_events( 0 );
		fl_treat_user_events( );
    }

    return event_queue.tail != NULL;
}


/***************************************
 * Replacement for the Xlib XNextEvent() function: copies the oldest
 * event into the XEvent structure and removes it from the queue. If
 * the queue is empty it blocks until an event has been received.
 ***************************************/

int
fl_XNextEvent( XEvent * xev )
{
    while ( event_queue.tail == NULL )
    {
		fl_treat_interaction_events( 1 );
		fl_treat_user_events( );
    }

	*xev = get_from_event_queue( );
	return 1;
}


/***************************************
 * Replacement for the Xlib XPeekEvent() function: returns the the
 * first event avaialable without removing it from the queue and
 * blocks until an event is received.
 ***************************************/

int
fl_XPeekEvent( XEvent * xev )
{
    while ( event_queue.tail == NULL )
    {
		fl_treat_interaction_events( 1 );
		fl_treat_user_events( );
    }

	*xev = event_queue.tail->xev;
    return 1;
}


/***************************************
 * get all user events and treat them: either consume by calling
 * the callback routine or put into FL internal Q for later
 * retrival
 ***************************************/

void
fl_treat_user_events( void )
{
    XEvent xev;

	while ( event_queue.count )
	{
		if ( fl_event_callback )
		{
			fl_XNextEvent( &xev );
			fl_event_callback( &xev, 0 );
		}
		else
			fl_object_qenter( FL_EVENT );

		event_queue.count--;
	}
}


/******************** DEBUG use only *****************/


#define NV( a ) { #a, a }

typedef struct
{
    const char * name;
    int          type;
} ev_name;


static ev_name evname[ ] =
{
	NV( 0 ),
	NV( 1 ),
    NV( KeyPress ),
	NV( KeyRelease ),
	NV( ButtonPress ),
    NV( ButtonRelease ),
	NV( MotionNotify ),
	NV( EnterNotify ),
    NV( LeaveNotify ),
	NV( FocusIn ),
	NV( FocusOut ),
    NV( KeymapNotify ),
	NV( Expose ),
	NV( GraphicsExpose ),
    NV( NoExpose ),
	NV( VisibilityNotify ),
	NV( CreateNotify ),
    NV( DestroyNotify ),
	NV( UnmapNotify ),
	NV( MapNotify ),
    NV( MapRequest ),
	NV( ReparentNotify ),
	NV( ConfigureNotify ),
    NV( ConfigureRequest ),
	NV( GravityNotify ),
	NV( ResizeRequest ),
    NV( CirculateNotify ),
	NV( CirculateRequest ),
	NV( PropertyNotify ),
    NV( SelectionClear ),
	NV( SelectionRequest ),
	NV( SelectionNotify ),
    NV( ColormapNotify ),
	NV( ClientMessage ),
	NV( MappingNotify )
};


/***************************************
 ***************************************/

const char *
fl_get_xevent_name( const XEvent *xev )
{
      size_t i;
      static char buf[ 128 ];

      for ( i = KeyPress; i < LASTEvent; i++ )
      {
          if ( evname[ i ].type == xev->type )
          {
			  fl_snprintf( buf, sizeof buf, "%s(0x%x)",
						   evname[ i ].name, xev->type );
			  return buf;
          }
      }

      return "unknown event";
}


/***************************************
 ***************************************/

XEvent *
fl_print_xevent_name( const char *   where,
					  const XEvent * xev )
{
    size_t i,
		   known;
    Window win = ( ( XAnyEvent * ) xev )->window;

    for ( i = KeyPress, known = 0; ! known && i < LASTEvent; i++ )
		if ( evname[ i ].type == xev->type )
		{
			fprintf( stderr, "%s Event (%d,w=0x%lx s=%ld) %s ",
					 where ? where : "",
					 xev->type, win, ( ( XAnyEvent * ) xev)->serial,
					 evname[ i ].name );

			if ( xev->type == Expose )
				fprintf( stderr, "count=%d serial=%lx\n",
						 xev->xexpose.count, xev->xexpose.serial );
			else if ( xev->type == LeaveNotify || xev->type == EnterNotify )
				fprintf(stderr, "Mode %s\n", xev->xcrossing.mode == NotifyGrab ?
						"Grab" :
						( xev->xcrossing.mode == NotifyNormal ?
						  "Normal" : "UnGrab" ) );
			else if ( xev->type == MotionNotify )
				fprintf(stderr, "Mode %s\n",
						xev->xmotion.is_hint ? "Hint" : "Normal" );
			else if ( xev->type == ConfigureNotify )
				fprintf( stderr, "(x=%d y=%d w=%d h=%d) %s\n",
						 xev->xconfigure.x, xev->xconfigure.y,
						 xev->xconfigure.width, xev->xconfigure.height,
						 xev->xconfigure.send_event ? "Syn" : "Non-Syn" );
			else if ( xev->type == ButtonPress )
				fprintf( stderr, "button: %d\n", xev->xbutton.button );
			else if ( xev->type == ButtonRelease )
				fprintf( stderr, "button: %d\n", xev->xbutton.button );
			else
				fputc( '\n', stderr );
			known = 1;
		}

    if ( ! known )
		fprintf( stderr, "Unknown event %d, win=%lu\n", xev->type, win );

    return ( XEvent * ) xev;
}


/***************************************
 ***************************************/

XEvent *
fl_xevent_name( const char *   where,
				const XEvent * xev )
{

    if ( fl_cntl.debug >= 2 )
		fl_print_xevent_name( where, xev );

    return ( XEvent * ) xev;
}


/***************************************
 ***************************************/

static int
badwin_handler( Display *     dpy  FL_UNUSED_ARG,
				XErrorEvent * xev )
{
	if ( xev->type != BadWindow && xev->type != BadDrawable )
		M_err( "badwin_handler",
			   "X error happened when expecting only BadWindow/Drawable\n" );
	return 0;
}


/***********************************************************************
 * Received an Expose event ev, see if next event is the same as the
 * the current one, drop it if it is, but we need consolidate all the
 * dirty rectangles into one.
 *
 * Must not block.
 ************************************************************************/

static void
compress_redraw( XEvent * ev )
{
	XEvent expose_ev;
    Window win = ev->xexpose.window;
    Region reg = XCreateRegion( );
	XRectangle rec;

    /* This is theoretically not correct as we can't peek ahead and ignore
       the events in between, but it works in XForms as we always update the
       form size and position when dealing with Expose event.

	   This has been changed a bit since 1.0.90: There was a problem with
	   e.g. KDE or Gnome when they were set up to redraw also during resizing
	   and the mouse was moved around rather fast. We collect now not only
	   Expose events, compressing them to a single one, covering the combined
	   area of all of them, but also ConfigureNotify events. If there was one
	   or more ConfigureNotify events we put back the "consolidated" Expose
	   event onto the event queue and return the last ConfigureNotify event
	   instead of the original Expose event we got started with. This hope-
	   fully is not only a solution that covers all cases but also keeps
	   the numbers of redraws to a minimum. The only drawback is that the
	   function do_interaction_step(), handling the Expose event, one has to
	   check if the area specified by the event isn't larger than the (new)
	   size of the window and prune it if necessary.                  JTT */
		
	/* Collect all Expose events, combining their areas */

	do {
		rec.x      = ev->xexpose.x;
		rec.y      = ev->xexpose.y;
		rec.width  = ev->xexpose.width;
		rec.height = ev->xexpose.height;

		XUnionRectWithRegion( &rec, reg, reg );

	} while ( XCheckTypedWindowEvent( flx->display, win, Expose, ev ) );

	/* Set the area of the last events to that of the "consolidated" event
	   and make a backup copy */

	XClipBox( reg, &rec );

	ev->xexpose.x = rec.x;
	ev->xexpose.y = rec.y;
	ev->xexpose.width = rec.width;
	ev->xexpose.height = rec.height;

	expose_ev = *ev;

    XDestroyRegion( reg );

	/* Now get all ConfigureNotify events */

	while ( XCheckTypedWindowEvent( flx->display, win, ConfigureNotify, ev ) )
		/*empty */ ;

	/* If there was at least one put the "consolidated" Expose event back
	   onto the event queue and return the last ConfigureNotify event we got,
	   otherwise the Expose event itself.

	   Since e.g. KDE and Gnome can send the ConfigureNotify event artificially
	   to achieve an update of the display while resizing is still going on,
	   the 'send_event' member of the XEvent structure might be set. On the
	   other hand, in do_interaction_step(), where the events are handled,
	   this member is checked for to get around a bug in mwm. So we got to
	   reset it here to avoid the event getting flagged as spurious. This
	   hopefully won't interfere we the mwm bug detection since it's for
	   cases were a ConfigureNotify gets send, but no corresponding Expose
	   events, and in this case we wouldn't have gotten here... */

	if ( ev->type == ConfigureNotify )
	{
		XPutBackEvent( flx->display, &expose_ev );
		ev->xconfigure.send_event = 0;
	}
}


/***************************************
 ***************************************/

static void
fl_compress_motion( XEvent * xme )
{
    Window win = xme->xmotion.window;
    unsigned long evm = PointerMotionMask | ButtonMotionMask;

	if ( xme->type != MotionNotify )
		return;

    do
    {
#if FL_DEBUG >= ML_DEBUG
		M_info2( "CompressMotion", "win=0x%lx (%d,%d) %s",
				 xme->xany.window, xme->xmotion.x, xme->xmotion.y,
				 xme->xmotion.is_hint ? "hint" : "" )
#endif
			/* empty */ ;
    } while ( XCheckWindowEvent( flx->display, win, evm, xme ) );

    if ( xme->xmotion.is_hint )
    {
        int ( *old )( Display *, XErrorEvent * );

        /* We must protect against BadWindow here, because we have only
         * looked for Motion events, and there could be a Destroy event
         * which makes the XQueryPointer fail as the window is deleted. */

        old = XSetErrorHandler( badwin_handler );
		fl_get_win_mouse( xme->xmotion.window,
						  &xme->xmotion.x, &xme->xmotion.y,
						  &xme->xmotion.state );
        XSetErrorHandler( old );
		xme->xmotion.is_hint = 0;
    }
}


/***************************************
 ***************************************/

void
fl_compress_event( XEvent *      xev,
				   unsigned long mask )
{
    if ( xev->type == Expose && mask & ExposureMask )
		compress_redraw( xev );
    else if (    xev->type == MotionNotify
			  && mask & ( PointerMotionMask | ButtonMotionMask ) )
		fl_compress_motion( xev );
}


/***************************************
 ***************************************/

int
fl_keysym_pressed( KeySym k )
{
    char kvec[ 32 ];
    KeyCode code;

    if ( ( code = XKeysymToKeycode( flx->display, k ) ) == NoSymbol )
    {
		M_warn( "fl_keysym_pressed", "Bad KeySym %d", (int) k );
		return 0;
    }

    XQueryKeymap( flx->display, kvec );
    return 1 & ( kvec[ code / 8 ] >> ( code & 7 ) );
}


/***************************************
 * add an event
 ***************************************/

long
fl_addto_selected_xevent( Window win,
						  long   mask )
{
    XWindowAttributes xwa;

    XGetWindowAttributes( flx->display, win, &xwa );
    xwa.your_event_mask |= mask;

    /* on some SGI machines, 'your_event_mask' has bogus value 0x80??????,
	   causing an X protocol error. Fix this here */

    xwa.your_event_mask &= ( OwnerGrabButtonMask << 1 ) - 1;
    XSelectInput( flx->display, win, xwa.your_event_mask );

    return xwa.your_event_mask;
}


/***************************************
 ***************************************/

long
fl_remove_selected_xevent( Window win,
						   long   mask )
{
    XWindowAttributes xwa;

    XGetWindowAttributes( flx->display, win, &xwa );
    xwa.your_event_mask &= ~mask;

    /* on some SGI machines, your_event_mask has bogus value 0x80??????,
	   causing an X protocol error. Fix this here */

    xwa.your_event_mask &= ( OwnerGrabButtonMask << 1 ) - 1;
    XSelectInput( flx->display, win, xwa.your_event_mask );

    return xwa.your_event_mask;
}
