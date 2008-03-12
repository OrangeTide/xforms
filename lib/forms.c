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
 * \file forms.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *  Main event dispatcher.
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_fm = "$Id: forms.c,v 1.18 2008/03/12 16:00:24 jtt Exp $";
#endif


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "global.h"


static FL_FORM * select_form_event( Display  *,
									XEvent * );
static int get_next_event( int,
						   FL_FORM **,
						   XEvent * );
static void force_visible( FL_FORM *,
						   int,
						   int );

static FL_FORM * fl_mainform;
static int nomainform;
static int reopened_group;
static int do_x_only;

static int fl_XLookupString( XKeyEvent *,
							 char *,
							 int,
							 KeySym * );

void fl_redraw_form_using_xevent( FL_FORM *,
								  int,
								  XEvent * );


#define SHORT_PAUSE   10	      /* check_form wait (in ms) */

#define TRANY( ob, form )      ( form->h - ob->h - ob->y )


/***************************************
 ***************************************/

void
fl_set_no_connection( int yes )
{
    if ( ( fl_no_connection = yes ) )
      fl_internal_init( );
}


/***************************************
 * Starts a form definition
 ***************************************/

FL_FORM *
fl_bgn_form( int      type,
			 FL_Coord w,
			 FL_Coord h )
{
    if ( ! fl_no_connection && ! flx->display )
    {
		M_err( "fl_bgn_form", "Missing or failed fl_initialize()" );
		exit( 1 );
    }

    if ( fl_current_form )
    {
		/* error actually is serious and can't be fixed easily as it might
		   be due to a bad recursion */

		M_err( "fl_bgn_form", "You forgot to call fl_end_form" );
		exit( 1 );
    }

    fl_current_form = fl_make_form( w, h );
    fl_add_box( type, 0, 0, w, h, "" );

    return fl_current_form;
}


/***************************************
 * Ends a form definition
 ***************************************/

void
fl_end_form( void )
{
    if ( fl_current_form == NULL )
		fl_error( "fl_end_form", "Ending form definition of NULL form." );
    fl_current_form = NULL;
}


/***************************************
 * Reopens a form for input
 ***************************************/

void
fl_addto_form( FL_FORM * form )
{
    if ( fl_current_form != NULL )
		fl_error( "fl_addto_form", "You forgot to call fl_end_form" );

    if ( form == NULL )
    {
		fl_error( "fl_addto_form", "Adding to NULL form." );
		return;
    }

    fl_current_form = form;
}


/***************************************
 * Starts a group definition
 ***************************************/

FL_OBJECT *
fl_bgn_group( void )
{
    static int id = 1;

    if ( ! fl_current_form )
    {
		fl_error( "fl_bgn_group", "Starting group in NULL form." );
		return NULL;
    }

    if ( fl_current_group )
    {
		fl_error( "fl_bgn_group", "You forgot to call fl_end_group." );
		fl_end_group( );
    }

    fl_current_group = fl_make_object( FL_BEGIN_GROUP, 0, 0, 10, 10, 0,
									   "", NULL );
    fl_current_group->group_id = id++;
    fl_add_object( fl_current_form, fl_current_group );

    return fl_current_group;
}


/***************************************
 * Ends a group definition
 ***************************************/

FL_OBJECT *
fl_end_group( void )
{
    FL_OBJECT *ob = fl_current_group;
    int id;

    if ( fl_current_form == NULL )
    {
		fl_error( "fl_end_group", "Ending group in NULL form." );
		return NULL;
    }

    if ( fl_current_group == NULL )
    {
		fl_error( "fl_end_group", "Ending NULL group." );
		return NULL;
    }

    id = fl_current_group->group_id;
    fl_current_group = NULL;

    if ( ! reopened_group )
    {
		ob = fl_make_object( FL_END_GROUP, 0, 0, 0, 0, 0, "", NULL );
		ob->group_id = id;
		fl_add_object( fl_current_form, ob );
    }

    if ( reopened_group == 3 )
		fl_end_form( );
    reopened_group = 0;

    return ob;
}


/************************ Doing the Interaction *************************/

static FL_FORM **forms = NULL;	    /* The forms being shown. */
static size_t formnumb = 0;	        /* Their number. */
static FL_FORM * mouseform = NULL;  /* The current form under mouse */
static FL_FORM * keyform = NULL;    /* keyboard focus form */
FL_OBJECT * fl_pushobj = NULL;	    /* latest pushed object */
FL_OBJECT * fl_mouseobj = NULL;	    /* object under the mouse */


/***************************************
 ***************************************/

void
fl_freeze_all_forms( void )
{
    size_t i;

    for ( i = 0; i < formnumb; i++ )
		fl_freeze_form( forms[ i ] );
}


/***************************************
 ***************************************/

void
fl_unfreeze_all_forms( void )
{
    size_t i;

    for ( i = 0; i < formnumb; i++ )
		fl_unfreeze_form( forms[ i ] );
}


/***************************************
 * Corrects the shape of the form based on the shape of its window
 ***************************************/

static void
reshape_form( FL_FORM * form )
{
    FL_Coord w,
		     h;

    fl_get_wingeometry( form->window, &form->x, &form->y, &w, &h );
    fl_set_form_size( form, w, h );
}


/***************************************
 ***************************************/

static void
move_form( FL_FORM * form )
{
    if ( form->visible > 0 )
		XMoveWindow( flx->display, form->window, form->x, form->y );
}


/***************************************
 ***************************************/

static void
resize_form_win( FL_FORM  * form,
				 FL_Coord   neww,
				 FL_Coord   newh )
{
    if ( form->visible > 0 )
		fl_winresize( form->window, neww, newh );
}


/***************************************
 * Scale a form with the given scaling factors and take care of object
 * gravity. This one differs from fl_scale_form() in the fact that we
 * don't reshape the window in any way. Most useful as a follow up to
 * ConfigureNotify event
 ***************************************/

static void
scale_form( FL_FORM * form,
			double    xsc,
			double    ysc )
{
    FL_OBJECT *obj;
    double neww = form->w_hr * xsc,
		   newh = form->h_hr * ysc;

    if ( FL_abs( neww - form->w ) < 1 && FL_abs( newh - form->h ) < 1 )
		return;

	form->w_hr = neww;
	form->h_hr = newh;

	form->w = FL_crnd( neww );
	form->h = FL_crnd( newh );

    if ( form->hotx >= 0 || form->hoty >= 0 )
    {
		form->hotx = form->hotx * xsc;
		form->hoty = form->hoty * ysc;
    }

    /* need to handle different resizing request */

    for ( obj = form->first; obj; obj = obj->next )
    {
		double oldw = obj->fl2 - obj->fl1;
		double oldh = obj->ft2 - obj->ft1;

		/* Resizing of scrollbars and textboxes gets dealt with by their
		   parent objects on redraw */

		if (    obj->objclass == FL_TEXTBOX
			 || obj->objclass == FL_SCROLLBAR )
			continue;

		/* Special case to keep the center of gravity of obejcts that have
		   no gravity set and aren't to be resized */

		if (    obj->resize == FL_RESIZE_NONE
			 && obj->segravity == FL_NoGravity
			 && obj->nwgravity == FL_NoGravity )
		{
			obj->fl1 += ( xsc - 1 ) * ( obj->fl1 + 0.5 * oldw );
			obj->ft1 += ( ysc - 1 ) * ( obj->ft1 + 0.5 * oldh );
			obj->fr1 = neww - obj->fl1;
			obj->fb1 = newh - obj->ft1;

			obj->fl2 = obj->fl1 + oldw;
			obj->ft2 = obj->ft1 + oldh;
			obj->fr2 = neww - obj->fl2;
			obj->fb2 = newh - obj->ft2;
		}
		else
		{
			/* In all other cases we recalculate the position of the upper left
			   hand and the lower right hand corner of the object relative to
			   all the borders of the form enclosing it, taking gravity and
			   resizing setting into account. The results sometimes can be
			   unexpected but hopefully are logically correct;-) */

			if ( ULC_POS_LEFT_FIXED( obj ) )
				obj->fr1 = neww - obj->fl1;
			else if ( ULC_POS_RIGHT_FIXED( obj ) )
				obj->fl1 = neww - obj->fr1;

			if ( LRC_POS_LEFT_FIXED( obj ) )
				obj->fr2 = neww - obj->fl2;
			else if ( LRC_POS_RIGHT_FIXED( obj ) )
				obj->fl2 = neww - obj->fr2;

			if ( ! HAS_FIXED_HORI_ULC_POS( obj ) )
			{
				if ( HAS_FIXED_HORI_LRC_POS( obj ) )
				{
					if ( obj->resize & FL_RESIZE_X )
						obj->fl1 = obj->fl2 - xsc * oldw;
					else
						obj->fl1 = obj->fl2 - oldw;
				}
				else
					obj->fl1 *= xsc;
					
				obj->fr1 = neww - obj->fl1;
			}

			if ( ! HAS_FIXED_HORI_LRC_POS( obj ) )
			{
				if ( obj->resize & FL_RESIZE_X )
					obj->fl2 = obj->fl1 + xsc * oldw;
				else
					obj->fl2 = obj->fl1 + oldw;
	
				obj->fr2 = neww - obj->fl2;
			}
			
			if ( ULC_POS_TOP_FIXED( obj ) )
				obj->fb1 = newh - obj->ft1;
			else if ( ULC_POS_BOTTOM_FIXED( obj ) )
				obj->ft1 = newh - obj->fb1;

			if ( LRC_POS_TOP_FIXED( obj ) )
				obj->fb2 = newh - obj->ft2;
			else if ( LRC_POS_BOTTOM_FIXED( obj ) )
				obj->ft2 = newh - obj->fb2;

			if ( ! HAS_FIXED_VERT_ULC_POS( obj ) )
			{
				if ( HAS_FIXED_VERT_LRC_POS( obj ) )
				{
					if ( obj->resize & FL_RESIZE_Y )
						obj->ft1 = obj->ft2 - ysc * oldh;
					else
						obj->ft1 = obj->ft2 - oldh;
				}
				else
					obj->ft1 *= ysc;

				obj->fb1 = newh - obj->ft1;
			}

			if ( ! HAS_FIXED_VERT_LRC_POS( obj ) )
			{
				if ( obj->resize & FL_RESIZE_Y )
					obj->ft2 = obj->ft1 + ysc * oldh;
				else
					obj->ft2 = obj->ft1 + oldh;
	
				obj->fb2 = newh - obj->ft2;
			}
		}

		obj->x = FL_crnd( obj->fl1 );
		obj->y = FL_crnd( obj->ft1 );
		obj->w = FL_crnd( obj->fl2 - obj->fl1 );
		obj->h = FL_crnd( obj->ft2 - obj->ft1 );

		if ( fl_inverted_y )
			obj->y = TRANY( obj, form );

		fl_handle_object_direct( obj, FL_RESIZED, 0, 0, 0, 0 );
    }
}


/***************************************
 * Externally visible routine to scale a form. Need to reshape the window
 ***************************************/

void
fl_scale_form( FL_FORM * form,
			   double    xsc,
			   double    ysc )
{
    if ( form == NULL )
    {
		fl_error( "fl_scale_form", "Scaling NULL form." );
		return;
    }

    if (    FL_crnd( form->w_hr * xsc ) == form->w
		 && FL_crnd( form->h_hr * ysc ) == form->h )
		return;

#if 0
    /* If form is visible, after reshaping of the window, a ConfigureNotify
       will be generated, which will then call scale_form. So don't have to
       do it here. */

    if ( ! form->visible )
		scale_form( form, xsc, ysc );
	else
	{
		form->w_hr *= xsc;
		form->h_hr *= ysc;
		form->w = FL_crnd( form->w_hr );
		form->h = FL_crnd( form->h_hr );
	}
#else
	/* The above claim doesn't seem to be correct for e.g. fvwm2. Moreover,
	   which part of the program would rescale the objects in the form?  JTT */

	scale_form( form, xsc, ysc );
#endif

    /* resize the window */

	resize_form_win( form, form->w, form->h );
}


/***************************************
 ***************************************/

void
fl_set_form_minsize( FL_FORM * form,
					 FL_Coord  w,
					 FL_Coord  h )
{
    if ( ! form )
    {
		Bark( "fl_set_form_minsize", "Null Form" );
		return;
    }

    fl_winminsize( form->window, w, h );
}


/***************************************
 ***************************************/

void
fl_set_form_maxsize( FL_FORM * form,
					 FL_Coord  w,
					 FL_Coord  h )
{
    if ( ! form )
    {
		Bark( "fl_set_form_maxsize", "Null Form" );
		return;
    }

    fl_winmaxsize( form->window, w, h );
}


/***************************************
 ***************************************/

void
fl_set_form_dblbuffer( FL_FORM * form,
					   int       y )
{
    form->use_pixmap = y;
}


/***************************************
 * Sets the size of the form on the screen.
 ***************************************/

void
fl_set_form_size( FL_FORM * form,
				  FL_Coord  w,
				  FL_Coord  h )
{
    if ( form == NULL )
    {
		fl_error( "fl_set_form_size", "Changing size of NULL form." );
		return;
    }

    if ( w != form->w || h != form->h )
		fl_scale_form( form, w / form->w_hr, h / form->h_hr );
}


/***************************************
 * Sets the position of the form on the screen. Note that the location
 * is specified relative to lower-left corner
 ***************************************/

void
fl_set_form_position( FL_FORM * form,
					  FL_Coord  x,
					  FL_Coord  y )
{
    FL_Coord oldx,
		     oldy;

    if ( form == NULL )
    {
		fl_error( "fl_set_form_position", "Changing position NULL form." );
		return;
    }

    oldx = form->x;
    oldy = form->y;
    form->x = x >= 0 ? x : ( fl_scrw + x );
    form->y = y >= 0 ? y : ( fl_scrh + y );

    if ( form->visible && ( oldx != form->x || oldy != form->y ) )
		move_form( form );
}


/***************************************
 * Sets the position of the form on the screen.
 ***************************************/

void
fl_set_form_hotspot( FL_FORM * form,
					 FL_Coord  x,
					 FL_Coord  y )
{
    if ( form == NULL )
    {
		fl_error( "fl_set_form_hotspot", "setting hotspot for NULL form." );
		return;
    }

    form->hotx = x;
    form->hoty = y;
}


/***************************************
 ***************************************/

void
fl_set_form_hotobject( FL_FORM   * form,
					   FL_OBJECT * ob )
{
    if ( ob )
		fl_set_form_hotspot( form, ob->x + ob->w / 2, ob->y + ob->h / 2 );
}


/***************************************
 * make sure a form is completely visible
 ***************************************/

static void
force_visible( FL_FORM * form,
			   int       itx,
			   int       ity )
{
    if ( form->x < itx )
		form->x = itx;

    if ( form->x > fl_scrw - form->w - 2 * itx )
	form->x = fl_scrw - form->w - 2 * itx;

    if ( form->y < ity )
		form->y = ity;

    if ( form->y > fl_scrh - form->h - itx )
		form->y = fl_scrh - form->h - 2 * itx;

    /* be a paranoid */
    if ( form->x < 0 || form->x > fl_scrw - form->w )
    {
		if ( form->w < fl_scrw - 20 )
			M_err( "force_visible", "Can't happen x=%d", form->x );
		form->x = 10;
    }

    if ( form->y < 0 || form->y > fl_scrh - form->h )
    {
		M_warn( "force_visible", "Can't happen y=%d", form->y );
		form->y = 20;
    }
}


/***************************************
 ***************************************/

void
fl_set_form_title( FL_FORM *    form,
				   const char * name )
{
    if ( form->label != name )
    {
		if ( form->label )
			fl_free( form->label );
		form->label = fl_strdup( name ? name : "" );
    }

    if ( form->window )
		fl_wintitle( form->window, form->label );
}


/***************************************
 * Displays a particular form. Returns window handle.
 ***************************************/

static int has_initial;
static int unmanaged_count;
static size_t auto_count = 0;


long
fl_prepare_form_window( FL_FORM *    form,
						int          place,
						int          border,
						const char * name )
{
    long screenw,
		 screenh;
    int itx = 0,
		ity = 0,
		dont_fix_size = 0;
    FL_Coord mx,
		     my,
		     nmx,
		     nmy;

    if ( border == 0 )
		border = FL_FULLBORDER;

    if ( fl_current_form )
    {
		M_err( "fl_prepare_form_window", "You forgot to call fl_end_form %s",
			   name ? name : "" );
		fl_current_form = 0;
    }

    if ( form == NULL )
    {
		fl_error( "fl_show_form", "Trying to display NULL form." );
		return -1;
    }

    if ( form->visible )
		return form->window;

	forms = fl_realloc( forms, ++formnumb * sizeof *forms );
	forms[ formnumb - 1 ] = form;

    if ( form->label != name )
    {
		if ( form->label )
			fl_free( form->label );
		form->label = fl_strdup( name ? name : "" );
    }

    /* if we are using private colormap or non-default visual, unmanaged
       window will not get correct colormap installed by the WM
       automatically. Make life easier by forcing a managed window. fl_vroot
       stuff is a workaround for tvtwm */

#if 0
    if (    border != FL_FULLBORDER
		 && (    fl_state[fl_vmode].pcm
			  || fl_visual( fl_vmode ) !=
				                    DefaultVisual( flx->display, fl_screen ) ) )
/*            || fl_root != fl_vroot ) )  */
    {
		border = FL_TRANSIENT;
    }
#endif

    if ( border != FL_NOBORDER )
    {
		FL_WM_STUFF *fb = &fl_wmstuff;

		itx = fb->bw + ( border == FL_TRANSIENT ? fb->trpx : fb->rpx );
		ity = fb->bw + ( border == FL_TRANSIENT ? fb->trpy : fb->rpy );
    }
    else
		unmanaged_count++;

    /* see if automatic */

    auto_count += form->has_auto > 0;

    form->wm_border = border;
    form->deactivated = 0;
    screenw = fl_scrw;
    screenh = fl_scrh;

    fl_get_mouse( &mx, &my, &fl_keymask );

    if ( ( dont_fix_size = place & FL_FREE_SIZE ) )
		place &= ~ FL_FREE_SIZE;

    if ( place == FL_PLACE_SIZE )
		fl_pref_winsize( form->w, form->h );
    else if ( place == FL_PLACE_ASPECT )
		fl_winaspect( 0, form->w, form->h );
    else if ( place == FL_PLACE_POSITION )
    {
		if ( fl_wmstuff.rep_method == FL_WM_SHIFT && border != FL_NOBORDER )
		{
			form->x -= itx;
			form->y -= ity;
		}
		fl_pref_winposition( form->x, form->y );
		fl_initial_winsize( form->w, form->h );
    }
    else if ( place != FL_PLACE_FREE )
    {
		switch ( place )
		{
			case FL_PLACE_CENTER:
			case FL_PLACE_FREE_CENTER:
				form->x = ( screenw - form->w ) / 2;
				form->y = ( screenh - form->h ) / 2;
				break;

			case FL_PLACE_MOUSE:
				form->x = mx - form->w / 2;
				form->y = my - form->h / 2;
				break;

			case FL_PLACE_FULLSCREEN:
				form->x = 0;
				form->y = 0;
				fl_set_form_size( form, screenw, screenh );
				break;

			case FL_PLACE_HOTSPOT:
				if ( form->hotx < 0 )
				{			                    /* never set */
					form->hotx = form->w / 2;
					form->hoty = form->h / 2;
				}

				nmx = mx;
				nmy = my;
				form->x = mx - form->hotx;
				form->y = my - form->hoty;
				force_visible( form, itx, ity );
				nmx = form->x + form->hotx;
				nmy = form->y + form->hoty;
				if ( nmx != mx || nmy != my )
					fl_set_mouse( nmx, nmy );
				break;
		}

		if ( place == FL_PLACE_GEOMETRY )
		{
			/* Correct form position. X < 0 means measure from right */

			if ( form->x < 0 )
				form->x = screenw + form->x - 2 - itx;

			/* y < 0 means from right */

			if ( form->y < 0 )
				form->y = screenh + form->y - 2 - ity;
		}

		/* final check. Make sure form is visible */

		force_visible( form, itx, ity );

		/* take care of reparenting stuff */

		if ( fl_wmstuff.rep_method == FL_WM_SHIFT && border != FL_NOBORDER )
		{
			form->x -= itx;
			form->y -= ity;
		}

		if ( dont_fix_size && place != FL_PLACE_GEOMETRY )
			fl_initial_wingeometry( form->x, form->y, form->w, form->h );
		else
			fl_pref_wingeometry( form->x, form->y, form->w, form->h );
    }
    else if ( place == FL_PLACE_FREE )
    {
		fl_initial_winsize( form->w, form->h );
		if ( has_initial )
		{
			if ( fl_wmstuff.rep_method == FL_WM_SHIFT && border != FL_NOBORDER )
			{
				form->x -= itx;
				form->y -= ity;
			}
			fl_initial_wingeometry( form->x, form->y, form->w, form->h );
		}
    }
    else
    {
		M_err( "fl_prepare_form_window", "Unknown requests: %d", place );
		fl_initial_wingeometry( form->x, form->y, form->w, form->h );
    }

    /* WM typically does not allow dragging transient windows */

    if ( border != FL_FULLBORDER )
    {
		if ( place == FL_PLACE_ASPECT || place == FL_PLACE_FREE )
		{
			form->x = mx - form->w / 2;
			form->y = my - form->h / 2;
			force_visible( form, itx, ity );
			fl_initial_winposition( form->x, form->y );
		}

		if ( border == FL_NOBORDER )
			fl_noborder( );
		else
			fl_transient( );
    }

    form->vmode = fl_vmode;

    if ( place == FL_PLACE_ICONIC )
		fl_initial_winstate( IconicState );
    if ( form->icon_pixmap )
		fl_winicon( 0, form->icon_pixmap, form->icon_mask );

    has_initial = 0;
    fl_init_colormap( fl_vmode );

    form->window = fl_create_window( fl_root, fl_colormap( fl_vmode ), name );
    fl_winicontitle( form->window, name );

    if ( border == FL_FULLBORDER || form->prop & FL_COMMAND_PROP )
		fl_set_form_property( form, FL_COMMAND_PROP );

    return form->window;
}


/***************************************
 ***************************************/

long
fl_show_form_window( FL_FORM * form )
{
    if ( form->window == None || form->visible )
		return form->window;

    fl_winshow( form->window );
    form->visible = 1;
    reshape_form( form );
	fl_redraw_form( form );

    return form->window;
}


/***************************************
 ***************************************/

long
fl_show_form( FL_FORM *    form,
			  int          place,
			  int          border,
			  const char * name )
{
    fl_prepare_form_window( form, place, border, name );
    return fl_show_form_window( form );
}


/***************************************
 * Hides a particular form
 ***************************************/

static void
close_form_win( Window win )
{
    XEvent xev;

    XUnmapWindow( flx->display, win );
    XDestroyWindow( flx->display, win );

    XSync( flx->display, 0 );
    while ( XCheckWindowEvent( flx->display, win, FL_ALL_MASKS, &xev ) )
		fl_xevent_name( "Eaten", &xev );

    /* this gives subwindow a chance to handle destroy event promptly */

#if 1
    /* TODO: consolidate event handling into fl_dispatch_XEvent(xev) and
       absorb this piece of code in that. Also the function should be of
       general use */

    while ( XCheckTypedEvent( flx->display, DestroyNotify, &xev ) )
    {
		FL_FORM *form;

		if ( ( form = select_form_event( flx->display, &xev ) ) )
		{
			size_t i;

			form->visible = 0;
			form->window = 0;
			for ( i = 0; i < formnumb; i++ )
				if ( form == forms[ i ] )
				{
					forms[ i ] = forms[ --formnumb ];
					forms = fl_realloc( forms, formnumb * sizeof *forms );
					break;
				}
		}
		else
			fl_XPutBackEvent( &xev );
    }
#else
    /* suspend timeout and IO stuff. need to complete hide_form without
       re-entring the main loop */

    do_x_only = 1;

    if ( fl_check_forms( ) )
		fl_XNextEvent( &xev );
    if ( fl_check_forms( ) )
		fl_XNextEvent( &xev );

    do_x_only = 0;
#endif
}


/***************************************
 ***************************************/

FL_FORM *
fl_property_set( unsigned int prop )
{
    size_t i;

    for ( i = 0; i < formnumb; i++ )
		if ( forms[ i ]->prop & prop && forms[ i ]->prop & FL_PROP_SET )
			return forms[ i ];

    return NULL;
}


/***************************************
 ***************************************/

void
fl_set_form_property( FL_FORM *    form,
					  unsigned int prop )
{
    if ( ! form || fl_property_set( prop ) )
		return;

    if ( ! ( prop & FL_COMMAND_PROP ) )
	{
		M_err( "fl_set_form_property", "Unknown form property request %u",
			   prop );
		return;
	}

	if ( form->window )
	{
		fl_set_property( form->window, FL_COMMAND_PROP );
		form->prop |= FL_PROP_SET;
	}

	form->prop |= FL_COMMAND_PROP;
	fl_mainform = form;
}


/***************************************
 ***************************************/

void
fl_hide_form( FL_FORM * form )
{
    size_t i;
    FL_OBJECT *obj;
    Window owin;

    if ( form == NULL )
    {
		fl_error( "fl_hide_form", "Hiding NULL form" );
		return;
    }

    if ( ! fl_is_good_form( form ) )
    {
		M_err( "fl_hide_form", "Hiding invisible/freeed form" );
		return;
    }

    if ( form->visible == FL_BEING_HIDDEN )
    {
		M_err( "fl_hide_form", "recursive call ?" );
		/* return; */
    }

    form->visible = FL_BEING_HIDDEN;
    fl_set_form_window( form );

    /* checking mouseobj->form is necessary as it might be deleted from a
       form */

    if ( fl_mouseobj != NULL && fl_mouseobj->form == form )
    {
#if FL_DEBUG >= ML_WARN
		if ( ! fl_mouseobj->visible )
			M_err( "fl_hide_form", "Outdated mouseobj %s",
				   fl_mouseobj->label ? fl_mouseobj->label : "" );
#endif
		obj = fl_mouseobj;
		fl_mouseobj = NULL;
		fl_handle_object( obj, FL_LEAVE, 0, 0, 0, 0 );
    }

    if ( fl_pushobj != NULL && fl_pushobj->form == form )
    {
		obj = fl_pushobj;
		fl_pushobj = NULL;
		fl_handle_object( obj, FL_RELEASE, 0, 0, 0, 0 );
    }

    if ( form->focusobj != NULL )
    {
		obj = form->focusobj;
		fl_handle_object( form->focusobj, FL_UNFOCUS, 0, 0, 0, 0 );
		fl_handle_object( obj, FL_FOCUS, 0, 0, 0, 0 );
    }

#ifdef DELAYED_ACTION
    fl_object_qflush( form );
#endif

    /* free backing store pixmap but keep the pointer */

    fl_free_flpixmap( form->flpixmap );

    if ( mouseform && mouseform->window == form->window )
		mouseform = NULL;

    form->deactivated = 1;
    form->visible = FL_INVISIBLE;
    owin = form->window;
    form->window = 0;

    fl_hide_tooltip( );

    close_form_win( owin );
	if ( flx->win == owin )
		flx->win = 0;

    for ( i = 0; i < formnumb; i++ )
		if ( form == forms[ i ] )
		{
			forms[ i ] = forms[ --formnumb ];
			forms = fl_realloc( forms, formnumb * sizeof *forms );
			break;
		}

    if ( form->wm_border == FL_NOBORDER )
    {
		unmanaged_count--;
		if ( unmanaged_count < 0 )
		{
			M_err( "fl_hide_form", "Bad unmanaged count" );
			unmanaged_count = 0;
		}
    }

    if ( form->has_auto )
    {
		if ( auto_count == 0 )
			M_err( "fl_hide_form", "Bad auto count" );
		else
			auto_count--;
    }

    /* need to re-establish command property */

    if ( formnumb && form->prop & FL_COMMAND_PROP )
		fl_set_form_property( forms[ 0 ], FL_COMMAND_PROP );

    if ( form == keyform )
		keyform = NULL;
}


/***************************************
 * Frees the memory used by a form, together with all its objects.
 ***************************************/

void
fl_free_form( FL_FORM * form )
{
    FL_OBJECT *current,
		      *next;

    /* check whether ok to free */

    if ( form == NULL )
    {
		fl_error( "fl_free_form", "Trying to free NULL form." );
		return;
    }

    if ( form->visible == FL_VISIBLE )
    {
		M_err( "fl_free_form", "Freeing visible form." );
		fl_hide_form( form );
    }

    /* free the objects */

    for ( current = next = form->first; next != NULL; current = next )
    {
		next = current->next;
		fl_free_object( current );
    }

    form->first = 0;

    if ( form->flpixmap )
    {
		fl_free_flpixmap( form->flpixmap );
		fl_free( form->flpixmap );
		form->flpixmap = NULL;
    }

    if ( form->label )
    {
		fl_free( form->label );
		form->label = NULL;
    }

    if ( form == fl_mainform )
		fl_mainform = NULL;

    /* free the form structure */
    fl_addto_freelist( form );
}


/***************************************
 * activates a form
 ***************************************/

void
fl_activate_form( FL_FORM * form )
{
    if ( form == NULL )
    {
		fl_error( "fl_activate_form", "Activating NULL form." );
		return;
    }

    if ( form->deactivated )
    {
		form->deactivated--;

		if ( ! form->deactivated && form->activate_callback )
			form->activate_callback( form, form->activate_data );
    }

    if ( form->child )
		fl_activate_form( form->child );
}


/***************************************
 * deactivates a form
 ***************************************/

void
fl_deactivate_form( FL_FORM * form )
{
    if ( form == NULL )
    {
		fl_error( "fl_deactivate_form", "Deactivating NULL form." );
		return;
    }

    if (    ! form->deactivated
		 && fl_mouseobj != NULL
		 && fl_mouseobj->form == form )
		fl_handle_object( fl_mouseobj, FL_LEAVE, 0, 0, 0, 0 );

    if ( ! form->deactivated && form->deactivate_callback )
		form->deactivate_callback( form, form->deactivate_data );

    form->deactivated++;

    if ( form->child )
		fl_deactivate_form( form->child );
}


/***************************************
 ***************************************/

FL_FORM_ATACTIVATE
fl_set_form_atactivate( FL_FORM *          form,
						FL_FORM_ATACTIVATE cb,
						void *             data )
{
    FL_FORM_ATACTIVATE old = NULL;

    if ( form )
    {
		old = form->activate_callback;
		form->activate_callback = cb;
		form->activate_data = data;
    }
    return old;
}


/***************************************
 ***************************************/

FL_FORM_ATDEACTIVATE
fl_set_form_atdeactivate( FL_FORM *            form,
						  FL_FORM_ATDEACTIVATE cb,
						  void *               data )
{
    FL_FORM_ATDEACTIVATE old = NULL;

    if ( form )
    {
		old = form->deactivate_callback;
		form->deactivate_callback = cb;
		form->deactivate_data = data;
    }

    return old;
}


/***************************************
 * activates all forms
 ***************************************/

void
fl_activate_all_forms( void )
{
    size_t i;

    for ( i = 0; i < formnumb; i++ )
		fl_activate_form( forms[ i ] );
}


/***************************************
 * deactivates all forms
 ***************************************/

void
fl_deactivate_all_forms( void )
{
    size_t i;

    for ( i = 0; i < formnumb; i++ )
		fl_deactivate_form( forms[ i ] );
}


static FL_Coord fl_mousex,	  /* last known mouse pos */
                fl_mousey;

#include <ctype.h>


/***************************************
 ***************************************/

static int
do_radio( FL_OBJECT * ob,
		  void *      d )
{
    FL_OBJECT *p = d;

    if ( ob->pushed && ob->radio && ob != d && ob->group_id == p->group_id )
    {
		fl_handle_object_direct( ob, FL_PUSH, ob->x, ob->y, 0, 0 );
		fl_handle_object_direct( ob, FL_RELEASE, ob->x, ob->y, 0, 0 );
		ob->pushed = 0;
    }

    return 0;
}


/***************************************
 * a radio object is pushed
 ***************************************/

void
fl_do_radio_push( FL_OBJECT * obj,
				  FL_Coord    xx,
				  FL_Coord    yy,
				  int         key,
				  void *      xev )
{
    FL_OBJECT *obj1 = obj;

    /* If this radio button does not belong to any group, have to search the
       entire form */

    if ( obj->group_id == 0 )
		fl_for_all_objects( obj->form, do_radio, obj );
    else
    {
		/* Find the begining of the group the current object belongs to */

		while ( obj1->prev != NULL && obj1->objclass != FL_BEGIN_GROUP )
			obj1 = obj1->prev;

		while ( obj1 != NULL && obj1->objclass != FL_END_GROUP )
		{
			if ( obj1->radio && obj1->pushed )
			{
				fl_handle_object_direct( obj1, FL_PUSH, xx, yy, key, xev );
				fl_handle_object_direct( obj1, FL_RELEASE, xx, yy, key, xev );
				obj1->pushed = 0;
			}

			obj1 = obj1->next;
		}
    }
}


/***************************************
 ***************************************/

static int
do_shortcut( FL_FORM * form,
			 int       key,
			 FL_Coord  x,
			 FL_Coord  y,
			 XEvent *  xev )
{
    int key1,
		key2;
    FL_OBJECT *obj;
	long *s;

    /* Check whether the <Alt> key is pressed */

    key1 = key2 = key;

    if ( fl_keypressed( XK_Alt_L ) || fl_keypressed( XK_Alt_R ) )
    {
		if ( key < 256 )
		{
			/* always a good idea to make Alt_k case insensitive */

			key1 = FL_ALT_VAL
				   + ( islower( key ) ? toupper( key ) : tolower( key ) );
			key2 = key + FL_ALT_VAL;
		}
		else
			key1 = key2 = key + FL_ALT_VAL;
    }

    M_info( "do_shortcut", "win=%lu key=%d %d %d",
			form->window, key, key1, key2 );

    /* Check whether an object has this as a shortcut */

    for ( obj = form->first; obj; obj = obj->next )
    {
		if ( ! obj->visible || obj->active <= 0 || ! obj->shortcut )
			continue;

		for ( s = obj->shortcut; *s; s++ )
		{
			if ( ! ( *s == key1 || *s == key2 ) )
				continue;

			if ( obj->objclass == FL_INPUT )
			{
				if ( obj != form->focusobj )
				{
					fl_handle_object( form->focusobj, FL_UNFOCUS,
									  x, y, 0, xev );
					fl_handle_object( obj, FL_FOCUS, x, y, 0, xev );
				}
			}
			else
			{
				if ( obj->radio )
					fl_do_radio_push( obj, x, y, key1, xev );

				XAutoRepeatOff( flx->display );
				fl_handle_object( obj, FL_SHORTCUT, x, y, key1, xev );
				fl_context->mouse_button = FL_SHORTCUT + key1;

				/* this is not exactly correct as shortcut might quit,
				   fl_finish will restore the keyboard state */

				if ( fl_keybdcontrol.auto_repeat_mode == AutoRepeatModeOn )
					XAutoRepeatOn( flx->display );
			}

			return 1;
		}
    }

    return 0;
}


/***************************************
 ***************************************/

int
fl_do_shortcut( FL_FORM * form,
				int       key,
				FL_Coord  x,
				FL_Coord  y,
				XEvent *  xev )
{
    int ret = do_shortcut( form, key, x, y, xev );

    if ( ! ret )
    {
		if ( form->child )
			ret = do_shortcut( form->child, key, x, y, xev );
		if ( ! ret && form->parent )
			ret = do_shortcut( form->parent, key, x, y, xev );
    }

    return ret;
}


/***************************************
 ***************************************/

static void
fl_keyboard( FL_FORM * form,
			 int       key,
			 FL_Coord  x,
			 FL_Coord  y,
			 void *    xev )
{
    FL_OBJECT *obj,
		      *obj1,
		      *special;

    /* always check shortcut first */

    if ( fl_do_shortcut( form, key, x, y, xev ) )
		return;

    /* focus policy is done as follows: Input object has the highiest
       priority. Next is the object that wants special keys which is followed
       by mouseobj that has the lowest. Focusobj == FL_INPUT OBJ */

    special = fl_find_first( form, FL_FIND_KEYSPECIAL, 0, 0 );
    obj1 = special ?
		   fl_find_object( special->next, FL_FIND_KEYSPECIAL, 0, 0 ) : NULL;

    /* if two or more objects that want keyboard input, none will get it and
       keyboard input will go to mouseobj instead */

    if ( obj1 && obj1 != special )
		special = fl_mouseobj;

    if ( form->focusobj )
    {
		FL_OBJECT *focusobj = form->focusobj;

		/* handle special keys first */
		if ( key > 255 )
		{
			if (    IsLeft( key )
				 || IsRight( key )
				 || IsHome( key )
				 || IsEnd( key ) )
				fl_handle_object( focusobj, FL_KEYBOARD, x, y, key, xev );
			else if (    (    IsUp( key )
						   || IsDown( key )
						   || IsPageUp( key )
						   || IsPageDown( key ) )
					  && focusobj->wantkey & FL_KEY_TAB )
				fl_handle_object( focusobj, FL_KEYBOARD, x, y, key, xev );
			else if ( special && special->wantkey & FL_KEY_SPECIAL )
			{
				/* moving the cursor in input field that does not have focus
				   looks weird */

				if ( special->objclass != FL_INPUT )
					fl_handle_object( special, FL_KEYBOARD, x, y, key, xev );
			}
			else if ( key == XK_BackSpace || key == XK_Delete )
				fl_handle_object( focusobj, FL_KEYBOARD, x, y, key, xev );
			return;
		}

		/* dispatch tab & return switches focus if not FL_KEY_TAB */

		if ( ( key == 9 || key == 13 ) && ! ( focusobj->wantkey & FL_KEY_TAB ) )
		{
			if ( ( ( XKeyEvent * ) xev )->state & fl_context->navigate_mask )
			{
				if ( focusobj == fl_find_first( form, FL_FIND_INPUT, 0, 0 ) )
					obj = fl_find_last( form, FL_FIND_INPUT, 0, 0 );
				else
					obj = fl_find_object( focusobj->prev, FL_FIND_INPUT, 0, 0 );
			}
			else
				obj = fl_find_object( focusobj->next, FL_FIND_INPUT, 0, 0 );

			if ( obj == NULL )
				obj = fl_find_first( form, FL_FIND_INPUT, 0, 0 );

			fl_handle_object( focusobj, FL_UNFOCUS, x, y, 0, xev );
			fl_handle_object( obj, FL_FOCUS, x, y, 0, xev );
		}
		else if ( focusobj->wantkey != FL_KEY_SPECIAL )
			fl_handle_object( focusobj, FL_KEYBOARD, x, y, key, xev );
		return;
    }

    /* keyboard input is not wanted */

    if ( ! special || special->wantkey == 0 )
		return;

    /* space is an exception for browser */

    if ( ( key > 255 || key == ' ' ) && special->wantkey & FL_KEY_SPECIAL )
		fl_handle_object( special, FL_KEYBOARD, x, y, key, xev );
    else if ( key < 255 && special->wantkey & FL_KEY_NORMAL )
		fl_handle_object( special, FL_KEYBOARD, x, y, key, xev );
    else if ( special->wantkey == FL_KEY_ALL )
		fl_handle_object( special, FL_KEYBOARD, x, y, key, xev );

#if FL_DEBUG >= ML_INFO1
    M_info( "do_shortcut", "(%d %d)pushing %d to %s\n",
			x, y, key, special->label );
#endif
}


/***************************************
 * updates a form according to an event
 ***************************************/

void
fl_handle_form( FL_FORM * form,
				int       event,
				int       key,
				XEvent *  xev )
{
    FL_OBJECT *obj = 0,
		      *obj1;
    FL_Coord xx,
		     yy;

    if ( ! form || ! form->visible )
		return;

    if ( form->deactivated && event != FL_DRAW )
		return;

    if (    form->parent_obj
		 && form->parent_obj->active == DEACTIVATED
		 && event != FL_DRAW )
		return;

    if ( event != FL_STEP )
		fl_set_form_window( form );

    xx = fl_mousex;
    yy = fl_mousey;

    if ( event != FL_STEP && event != FL_DRAW )
    {
		if (    event == FL_PUSH
			 || event == FL_RELEASE
			 || event == FL_ENTER
			 || event == FL_LEAVE
			 || event == FL_KEYBOARD
			 || event == FL_MOUSE )
		{
			xx = fl_mousex;
			yy = fl_mousey;
		}
		else
			fl_get_form_mouse( form, &xx, &yy, &fl_keymask );

		/* obj under the mouse */

		obj = fl_find_last( form, FL_FIND_MOUSE, xx, yy );
    }

#if 0
    /* this is an ugly hack. This is necessary due to popup pointer grab
       where a button release is eaten. Really should do a send event from
       the pop-up routines. */

    if (    fl_pushobj
		 && ! button_down( fl_keymask )
		 /* && event == FL_ENTER */ )
    {
		obj = fl_pushobj;
		fl_pushobj = NULL;
		fl_handle_object( obj, FL_RELEASE, xx, yy, key, xev );
    }
#endif

    switch ( event )
    {
		case FL_DRAW:		/* form must be redrawn */
			fl_redraw_form_using_xevent( form, key, xev );
			break;

		case FL_ENTER:		/* Mouse did enter the form */
			fl_mouseobj = obj;
			fl_handle_object( fl_mouseobj, FL_ENTER, xx, yy, 0, xev );
			break;

		case FL_LEAVE:		/* Mouse did leave the form */
			fl_handle_object( fl_mouseobj, FL_LEAVE, xx, yy, 0, xev );
			fl_mouseobj = NULL;
			break;

		case FL_MOUSE:		/* Mouse position changed in the form */
			if ( fl_pushobj != NULL )
				fl_handle_object( fl_pushobj, FL_MOUSE, xx, yy, key, xev );
			else if ( obj != fl_mouseobj )
			{
				fl_handle_object( fl_mouseobj, FL_LEAVE, xx, yy, 0, xev );
				fl_handle_object( fl_mouseobj = obj, FL_ENTER, xx, yy, 0, xev );
			}
#if 1
			/* can't remember why this is here, probably does something
			   required */

			else if ( fl_mouseobj != NULL )
				fl_handle_object( fl_mouseobj, FL_MOTION, xx, yy, 0, xev );
#endif
			break;

		case FL_PUSH:		/* Mouse was pushed inside the form */
			/* change input focus */
#if 1
			if ( obj != NULL && obj->input && form->focusobj != obj )
			{
				fl_handle_object( form->focusobj, FL_UNFOCUS,
								  xx, yy, key, xev );
				fl_handle_object( obj, FL_FOCUS, xx, yy, key, xev );
			}
#else
			/* 05/29/99 Always force unfocus. Could be risky just before
			   release */

			if ( obj != NULL && form->focusobj != obj )
			{
				FL_OBJECT *tmpobj = form->focusobj;

				fl_handle_object( form->focusobj, FL_UNFOCUS,
								  xx, yy, key, xev );
				if ( obj->input )
					fl_handle_object( obj, FL_FOCUS, xx, yy, key, xev );
				else
					fl_handle_object( tmpobj, FL_FOCUS, xx, yy, key, xev );
			}
#endif

			if ( form->focusobj )
				keyform = form;

			/* handle a radio button */

			if ( obj != NULL && obj->radio )
				fl_do_radio_push( obj, xx, yy, key, xev );

			/* push the object except when focus is overriden  */

			if ( obj && ( ! obj->input || ( obj->input && obj->focus ) ) )
				fl_handle_object( obj, FL_PUSH, xx, yy, key, xev );
			fl_pushobj = obj;
			break;

		case FL_RELEASE:		/* Mouse was released inside the form */
			if ( fl_pushobj )
			{
				obj = fl_pushobj;
				fl_pushobj = NULL;
				fl_handle_object( obj, FL_RELEASE, xx, yy, key, xev );
			}
			break;

		case FL_KEYBOARD:		/* A key was pressed */
			fl_keyboard( form, key, xx, yy, xev );
			break;

		case FL_STEP:		/* A simple step */
			obj1 = fl_find_first( form, FL_FIND_AUTOMATIC, 0, 0 );
			if ( obj1 != NULL )
				fl_set_form_window( form );	/* set only if required */
			while ( obj1 != NULL )
			{
				fl_handle_object( obj1, FL_STEP, xx, yy, 0, xev );
				obj1 = fl_find_object( obj1->next, FL_FIND_AUTOMATIC, 0, 0 );
			}
			break;

		case FL_MOVEORIGIN:
		case FL_OTHER:
			/* need to dispatch it thru all objects and monitor the status of
			   forms as it may get closed */

			for ( obj1 = form->first; obj1 && form->visible; obj1 = obj1->next )
				if ( obj1->visible )
					fl_handle_object( obj1, event, xx, yy, key, xev );
			break;
    }
}


/***************
  Routine to check for events
***************/

#define NTIMER  FL_NTIMER
static long lastsec[ NTIMER ];
static long lastusec[ NTIMER ];

typedef struct
{
    long sec;
    long usec;
} TimeVal;

static TimeVal tp;


/***************************************
 * Resets the timer
 ***************************************/

void
fl_reset_time( int n )
{
    n %= NTIMER;
    fl_gettime( lastsec + n, lastusec + n );
}


/***************************************
 * Returns the time passed since the last call
 ***************************************/

double
fl_time_passed( int n )
{
    n %= NTIMER;
    fl_gettime( &tp.sec, &tp.usec );
    return tp.sec - lastsec[ n ] + 1.0e-6 * ( tp.usec - lastusec[ n ] );
}


/***************************************
 ***************************************/

#if FL_DEBUG >= ML_DEBUG	/* { */
static void
hack_test( void )
{
    int ff,
		jj = 0;
    static int k;
    FL_FORM *f[ 20 ] = { NULL };


    fl_set_graphics_mode( ++k % 6, 1 );

    /* force the action */
    for ( ff = 0; ff < MAX_FORM; ff++ )
    {
		if ( forms[ ff ] && forms[ ff ]->visible )
			fl_hide_form( f[ jj++ ] = forms[ ff ] );
    }

    for ( ff = 0; ff < jj; ff++ )
		if ( f[ ff ] )
			fl_show_form( f[ ff ], FL_PLACE_GEOMETRY, 1, f[ff]->label );
}

#endif /* } */


/***************************************
 * formevent is either FL_KEYPRESS or FL_KEYRELEASE
 ***************************************/

static void
do_keyboard( XEvent * xev,
			 int      formevent )
{
    Window win;
    int kbuflen;

    /* before doing anything, save the current modifier key for the handlers */

    win = xev->xkey.window;
    fl_keymask = xev->xkey.state;

    if ( win && ( ! keyform || ! fl_is_good_form( keyform ) ) )
		keyform = fl_win_to_form( win );

    /* switch keyboard input only if different top-level form */

    if ( keyform && keyform->window != win )
    {
		M_warn( "do_keyboard", "pointer/keybd focus differ" );
		if (    ( keyform->child && keyform->child->window == win )
			 || ( keyform->parent && keyform->parent->window == win ) )
			;
		else
			keyform = fl_win_to_form( win );
    }

    if ( keyform )
	{
		KeySym keysym = 0;
		unsigned char keybuf[ 227 ];

		kbuflen = fl_XLookupString( ( XKeyEvent * ) xev, ( char * ) keybuf,
									sizeof keybuf, &keysym );

		if ( kbuflen < 0 )
		{
			if ( kbuflen != INT_MIN )
			{
				/* buffer overflow, should not happen */
				M_err( "do_keyboard", "keyboad buffer overflow ?" );
			}
			else
			{
				M_err( "do_keyboard", "fl_XLookupString failed ?" );
			}
		}
		else
		{
			/* ignore modifier keys as they don't cause action and are
			   taken care of by the lookupstring routine */

			if ( IsModifierKey( keysym ) )
				/* empty */ ;
			else if ( IsTab( keysym ) )
			{
				/* fake a tab key. */
				/* some system shift+tab does not generate tab */

				fl_handle_form( keyform, formevent, 9, xev );
			}
#if FL_DEBUG >= ML_DEBUG
			else if ( keysym == XK_F10 )   /* && controlkey_down(fl_keymask)) */
				hack_test( );
#endif
			/* pass all keys to the handler */
			else if ( IsCursorKey( keysym ) || kbuflen == 0 )
				fl_handle_form( keyform, formevent, keysym, xev );
			else
			{
				unsigned char *ch;

				/* all regular keys, including mapped strings */
				for ( ch = keybuf; ch < keybuf + kbuflen && keyform; ch++ )
					fl_handle_form( keyform, formevent, *ch, xev );
			}
		}
    }
}


/***************************************
 ***************************************/

FL_FORM_ATCLOSE
fl_set_form_atclose( FL_FORM *       form,
					 FL_FORM_ATCLOSE fmclose,
					 void *          data )
{
    FL_FORM_ATCLOSE old = form->close_callback;

    form->close_callback = fmclose;
    form->close_data = data;
    return old;
}


/***************************************
 ***************************************/

FL_FORM_ATCLOSE
fl_set_atclose( FL_FORM_ATCLOSE fmclose,
				void *          data )
{
    FL_FORM_ATCLOSE old = fl_context->atclose;

    fl_context->atclose = fmclose;
    fl_context->close_data = data;
    return old;
}


/***************************************
 * ClientMessage is intercepted if it is delete window
 ***************************************/

static void
handle_client_message( FL_FORM * form,
					   void *    xev )
{
    XClientMessageEvent *xcm = xev;
    static Atom atom_protocol;
    static Atom atom_del_win;


    if ( ! atom_del_win )
    {
		atom_protocol = XInternAtom( xcm->display, "WM_PROTOCOLS", 0 );
		atom_del_win = XInternAtom( xcm->display, "WM_DELETE_WINDOW", 0 );
    }

    /* if delete top-level window, quit unless handlers are installed */

    if (    xcm->message_type == atom_protocol
		 && ( Atom ) xcm->data.l[0] == atom_del_win )
    {
		if ( form->close_callback )
		{
			if (    form->close_callback( form, form->close_data ) != FL_IGNORE
				 && form->visible == FL_VISIBLE )
				fl_hide_form( form );

			if ( form->sort_of_modal )
				fl_activate_all_forms( );
		}
		else if ( fl_context->atclose )
		{
			if ( fl_context->atclose( form, fl_context->close_data )
				 	                                              != FL_IGNORE )
				exit( 1 );
		}
		else
			exit( 1 );
    }
    else
    {
		/* pump it thru current form */

		fl_handle_form( form, FL_OTHER, 0, xev );
    }
}


static int pre_emptive_consumed( FL_FORM *,
								 int,
								 XEvent * );

/***************************************
 ***************************************/

FL_FORM *
fl_win_to_form( Window win )
{
	size_t i;

    for ( i = 0; i < formnumb; i++ )
		if ( forms[ i ]->window == win )
			return forms[ i ];

    return NULL;
}


void ( * fl_handle_signal )( void );
int ( * fl_handle_clipboard )( void * );


/***************************************
 ***************************************/

void
fl_handle_automatic( XEvent * xev,
					 int      idle_cb )
{
    FL_IDLE_REC *idle_rec;
	size_t i;
    static int nc;

    if ( fl_handle_signal )
		fl_handle_signal( );

	if ( auto_count )
		for ( i = 0; i < formnumb; i++ )
			if ( forms[ i ]->has_auto )
				fl_handle_form( forms[ i ], FL_STEP, 0, xev );

    if ( idle_cb )
    {
		if ( ++nc & 0x40 )
		{
			fl_free_freelist( );
			nc = 0;
		}

		if ( ( idle_rec = fl_context->idle_rec ) && idle_rec->callback )
			idle_rec->callback( xev, idle_rec->data );

		fl_handle_timeouts( 200 );	/* force a re-sync */
    }
}


/* how frequent to generate FL_STEP EVENT, in milli-seconds. These
 * are modified if idle callback exists
 */

static int delta_msec = TIMER_RES;

static int form_event_queued( XEvent *, int );


static XEvent st_xev;


/***************************************
 ***************************************/

const XEvent *
fl_last_event( void )
{
    return &st_xev;
}


static int ignored_fake_configure;


/***************************************
 ***************************************/

static int
button_is_really_down( void )
{
    FL_Coord x,
		     y;
    unsigned int km;

    fl_get_mouse( &x, &y, &km );
    return button_down( km );
}


/***************************************
 * should pass the mask instead of button numbers into the
 * event handler. basically throwing away info ..
 ***************************************/

static int
xmask2key( unsigned int mask )
{
    /* once the FL_XXX_MOUSE is changed to mask, just loose the else */

    if ( mask & Button1Mask )
		return FL_LEFT_MOUSE;

    if ( mask & Button2Mask )
		return FL_MIDDLE_MOUSE;

    if ( mask & Button3Mask )
		return FL_RIGHT_MOUSE;

    if ( mask & Button4Mask )
		return FL_SCROLLUP_MOUSE;

    if ( mask & Button5Mask )
		return FL_SCROLLDOWN_MOUSE;

    return 0;
}


static void handle_idling( int            wait_io,
						   int *          lasttimer,
						   unsigned int * query_cnt );
static void handle_EnterNotify_event( FL_FORM * evform );
static void handle_LeaveNotify_event( void );
static void handle_MotionNotify_event( FL_FORM * evform );
static void handle_Expose_event( FL_FORM *  evform,
								 FL_FORM ** redraw_form );
static void handle_ConfigureNotify_event( FL_FORM *  evform,
										  FL_FORM ** redraw_form );
static void handle_DestroyNotify_event( FL_FORM * evform );


/***************************************
 ***************************************/

static void
do_interaction_step( int wait_io )
{
    FL_FORM *evform = NULL;
	static FL_FORM *redraw_form = NULL;
    static unsigned int query_cnt = 0;
    static int lasttimer = 0;

    if ( ! get_next_event( wait_io, &evform, &st_xev ) )
    {
		handle_idling( wait_io, &lasttimer, &query_cnt );
		return;
	}

	/* got an event for forms */

#if FL_DEBUG >= ML_WARN
	if ( st_xev.type != MotionNotify || fl_cntl.debug > 2 )
		fl_xevent_name( "MainLoop", &st_xev );
#endif

	if ( ! evform )
	{
		M_err( "do_interaction_step", "Something is wrong" );
		return;
	}

	fl_compress_event( &st_xev, evform->compress_mask );

	lasttimer = 0;
	query_cnt = 0;

	if ( pre_emptive_consumed( evform, st_xev.type, &st_xev ) )
		return;

    switch ( st_xev.type )
    {
		case MappingNotify:
			XRefreshKeyboardMapping( ( XMappingEvent * ) &st_xev );
			break;

		case FocusIn:
			if ( ! fl_context->xic )
				break;

			M_info( "do_interaction_step", "Setting focus window for IC" );
			XSetICValues( fl_context->xic,
						  XNFocusWindow, st_xev.xfocus.window,
						  XNClientWindow, st_xev.xfocus.window,
						  ( char * ) NULL );
			break;

		case KeyPress:
			fl_mousex = st_xev.xkey.x;
			fl_mousey = st_xev.xkey.y;
			fl_keymask = st_xev.xkey.state;
			do_keyboard( &st_xev, FL_KEYPRESS );
			break;

		case KeyRelease:
			fl_mousex = st_xev.xkey.x;
			fl_mousey = st_xev.xkey.y;
			fl_keymask = st_xev.xkey.state;
			do_keyboard( &st_xev, FL_KEYRELEASE );
			break;

		case EnterNotify:
			handle_EnterNotify_event( evform );
			break;

		case LeaveNotify:
			handle_LeaveNotify_event( );
			break;

		case MotionNotify:
			handle_MotionNotify_event( evform );
			break;

		case ButtonPress:
			fl_keymask = st_xev.xbutton.state;
			fl_mousex = st_xev.xbutton.x;
			fl_mousey = st_xev.xbutton.y;
			fl_context->mouse_button = st_xev.xbutton.button;
			if ( metakey_down( fl_keymask ) && st_xev.xbutton.button == 2 )
				fl_print_version( 1 );
			else
				fl_handle_form( mouseform, FL_PUSH,
								st_xev.xbutton.button, &st_xev );
			break;

		case ButtonRelease:
			fl_keymask = st_xev.xbutton.state;
			fl_mousex = st_xev.xbutton.x;
			fl_mousey = st_xev.xbutton.y;
			fl_context->mouse_button = st_xev.xbutton.button;

#if FL_DEBUG >= ML_DEBUG
			if ( ! mouseform )
				M_warn( "do_interaction_step", "mouse form == 0" );
#endif
			if ( mouseform )
				fl_handle_form( mouseform, FL_RELEASE,
								st_xev.xbutton.button, &st_xev );

			mouseform = evform;
			break;

		case Expose:
			handle_Expose_event( evform, &redraw_form );
			break;

		case ConfigureNotify:
			handle_ConfigureNotify_event( evform, &redraw_form );
			break;

		case ClientMessage:
			handle_client_message( evform, &st_xev );
			break;

		case DestroyNotify:	/* only sub-form gets this due to parent destroy */
			handle_DestroyNotify_event( evform );
			break;

		case SelectionClear:
		case SelectionRequest:
		case SelectionNotify:
			if ( ! fl_handle_clipboard || fl_handle_clipboard( &st_xev ) < 0 )
				fl_handle_form( evform, FL_OTHER, 0, &st_xev );
			break;

		default:
			fl_handle_form( evform, FL_OTHER, 0, &st_xev );
			break;
    }
}


/***************************************
 ***************************************/

static void
handle_idling( int            wait_io,
	           int *          lasttimer,
			   unsigned int * query_cnt )
{
#if FL_DEBUG >= ML_TRACE
	M_info( "handle_idling", "Artificial timer event" );
#endif

	/* certain events like Selection/GraphicsExpose do not have a window
	   member itself, but xany.window happens to be the one we want */

	if ( ( *query_cnt )++ % 100 == 0 )
	{
		*query_cnt = 1;
		fl_get_form_mouse( mouseform, &fl_mousex, &fl_mousey, &fl_keymask );
		st_xev.xany.window = mouseform ? mouseform->window : 0;
		st_xev.xany.send_event = 1;		/* indicating synthetic event */
		st_xev.xmotion.state = fl_keymask;
		st_xev.xmotion.x = fl_mousex;
		st_xev.xmotion.y = fl_mousey;
		st_xev.xmotion.is_hint = 0;
	}
	else
		st_xev.xmotion.time += wait_io ? delta_msec : SHORT_PAUSE;

	/* need to do FL_MOUSE for touch buttons */

	st_xev.type = MotionNotify;

	/* unless ButtonRelease does both Release and Leave, have to generate at
	   least one lasttimer due to the possibility of a Release/Leave/Motion
	   eaten by popups. True fix would be to have xpopup send an motion
	   event */

	if ( button_down( fl_keymask ) || fl_pushobj || ! *lasttimer )
	{
		fl_handle_form( mouseform, FL_MOUSE,
						xmask2key( fl_keymask ), &st_xev );
		*lasttimer = 1;
	}

	/* handle both automatic and idle callback */

	fl_handle_automatic( &st_xev, 1 );
}


/***************************************
 * Handling of EnterNotiy events
 ***************************************/

static void
handle_EnterNotify_event( FL_FORM * evform )
{
    Window win = st_xev.xany.window;

	fl_keymask = st_xev.xcrossing.state;

	if (    button_down( fl_keymask )
		 && st_xev.xcrossing.mode != NotifyUngrab )
		return;

	fl_mousex = st_xev.xcrossing.x;
	fl_mousey = st_xev.xcrossing.y;

	if ( mouseform )
		fl_handle_form( mouseform, FL_LEAVE,
						xmask2key( fl_keymask ), &st_xev );
	mouseform = NULL;

	if ( evform )
	{
		mouseform = evform;

		/* this is necessary because win might be un-managed. To be
		   friendly to other applications, grab focus only if abslutely
		   necessary */

		if (    mouseform->deactivated == 0
			 && ! st_xev.xcrossing.focus && unmanaged_count > 0 )
		{
			fl_check_key_focus( "EnterNotify", win );
			fl_winfocus( win );
		}

		fl_handle_form( mouseform, FL_ENTER,
						xmask2key( fl_keymask ), &st_xev );
	}
#if FL_DEBUG >= ML_DEBUG
	else
		M_err( "handle_EnterNotify_event", "Null form!" );
#endif
}


/***************************************
 * Handling of LeaveNotiy events
 ***************************************/

static void
handle_LeaveNotify_event( void )
{
	fl_keymask = st_xev.xcrossing.state;

	if (    button_down( fl_keymask )
		 && st_xev.xcrossing.mode == NotifyNormal )
		return;

	/* olvwm sends leavenotify with NotifyGrab whenever button is clicked.
	   Ignore it. Due to Xpoup grab, (maybe Wm bug ?), end grab can also
	   generate this event. we can tell these two situations by doing a real
	   button_down test (as opposed to relying on the keymask in event) */

	if (    st_xev.xcrossing.mode == NotifyGrab
		 && button_is_really_down( ) )
		return;

	fl_mousex = st_xev.xcrossing.x;
	fl_mousey = st_xev.xcrossing.y;

	if ( ! mouseform )
		return;

	/* due to grab in pop-up , FL_RELEASE is necessary */

	fl_handle_form( mouseform, FL_RELEASE, 0, &st_xev );
	fl_handle_form( mouseform, FL_LEAVE,
					xmask2key( fl_keymask ), &st_xev );
	mouseform = NULL;
}


/***************************************
 * Handling of MotionNotify events
 ***************************************/

static void
handle_MotionNotify_event( FL_FORM * evform )
{
    static unsigned int auto_cnt = 0;
    Window win = st_xev.xany.window;

	fl_keymask = st_xev.xmotion.state;
	fl_mousex = st_xev.xmotion.x;
	fl_mousey = st_xev.xmotion.y;

	if ( ! mouseform )
	{
		M_warn( "handle_MotionNotify_event", "evwin=0x%lx", win );
		return;
	}

	if ( mouseform->window != win )
	{
		M_warn( "handle_MotionNotify_event", "mousewin=0x%ld evwin=0x%ld",
				mouseform->window, win );
		fl_mousex += evform->x - mouseform->x;
		fl_mousey += evform->y - mouseform->y;
	}

	fl_handle_form( mouseform, FL_MOUSE,
					xmask2key( fl_keymask ), &st_xev );

	/* Handle FL_STEP. Every 10 events. This will reduce cpu usage */

	if ( ++auto_cnt % 10 == 0 )
	{
#if FL_DEBUG >= ML_TRACE
		M_info( "handle_MotionNotify_event", "Auto FL_STEP" );
#endif
		fl_handle_automatic( &st_xev, 0 );
		auto_cnt = 0;
	}
}


/***************************************
 * Handling of Expose events
 ***************************************/

static void
handle_Expose_event( FL_FORM *  evform,
					 FL_FORM ** redraw_form )
{
	if ( ! evform )
		return;

	/* If 'redraw_form' is set we got a ConfigureNotify and the data from the
	   Exposure event aren't correct - set clipping to the complete area of
	   the form.                                                         JTT */

	if ( *redraw_form == evform )
	{
		st_xev.xexpose.x = 0;
		st_xev.xexpose.y = 0;
		st_xev.xexpose.width  = evform->w;
		st_xev.xexpose.height = evform->h;
		*redraw_form = NULL;
	}
	else
	{
		if ( st_xev.xexpose.x + st_xev.xexpose.width > evform->w )
			st_xev.xexpose.width = evform->w - st_xev.xexpose.x;
		if ( st_xev.xexpose.y + st_xev.xexpose.height > evform->h )
			st_xev.xexpose.height = evform->h - st_xev.xexpose.y;
	}

	fl_set_perm_clipping( st_xev.xexpose.x, st_xev.xexpose.y,
						  st_xev.xexpose.width, st_xev.xexpose.height );
	fl_set_clipping( st_xev.xexpose.x, st_xev.xexpose.y,
					 st_xev.xexpose.width, st_xev.xexpose.height );

	/* run into trouble by ignoring configure notify */

	if ( ignored_fake_configure )
	{
		FL_Coord neww,
			     newh;

		M_warn( "handle_Expose_event", "Run into trouble - correcting it" );
		fl_get_winsize( evform->window, &neww, &newh );
		scale_form( evform, ( double ) neww / evform->w,
					( double ) newh / evform->h );
		ignored_fake_configure = 0;
	}

	fl_handle_form( evform, FL_DRAW, 0, &st_xev );

	fl_unset_perm_clipping( );
	fl_unset_clipping( );
	fl_unset_text_clipping( );
}


/***************************************
 * Handling of ConfigureNotify events
 ***************************************/

static void
handle_ConfigureNotify_event( FL_FORM *  evform,
							  FL_FORM ** redraw_form )
{
	Window win = st_xev.xany.window;
	int old_w = evform->w;
	int old_h = evform->h;

	if ( ! evform )
		return;

	if ( ! st_xev.xconfigure.send_event )
		fl_get_winorigin( win, &evform->x, &evform->y );
	else
	{
		evform->x = st_xev.xconfigure.x;
		evform->y = st_xev.xconfigure.y;
		M_warn( "handle_ConfigureNotify_event", "WMConfigure:x=%d y=%d",
				evform->x, evform->y );
	}

	/* mwm sends bogus ConfigureNotify randomly without following up with a
	   redraw event, but it does set send_event. The check is somewhat
	   dangerous, use 'ignored_fake_configure' to make sure when we got expose
	   we can respond correctly. The correct fix is always to get window
	   geometry in Expose handler, but that has a two-way traffic overhead */

	ignored_fake_configure =    st_xev.xconfigure.send_event
				             && (    st_xev.xconfigure.width  != evform->w
					              || st_xev.xconfigure.height != evform->h );

	/* Dragging the form across the screen changes its absolute x, y coords.
	   Objects that themselves contain forms should ensure that they are up to
	   date. */

	fl_handle_form( evform, FL_MOVEORIGIN, 0, &st_xev );

	if ( st_xev.xconfigure.send_event )
		return;

	/* can't just set form->{w,h}. Need to take care of obj gravity */

	scale_form( evform, ( double ) st_xev.xconfigure.width  / evform->w,
				        ( double ) st_xev.xconfigure.height / evform->h );

	/* If both the width and the height got smaller (or one got smaller and
	   the other one is unchanged) we're not going to get an Expose event at
	   all, so we need to redraw the form. Even if only one of the lengths
	   gets smaller or remains unchanged while the other got larger, the next
	   (compressed) Expose event will only cover the added part, so in this
	   case store the forms address, so on the next Expose event we receive
	   for it it's full area can be redrawn.                              JTT */

	if ( evform->w <= old_w && evform->h <= old_h )
		fl_redraw_form( evform );
	else if ( ! ( evform->w > old_w && evform->h > old_h ) ) 
		*redraw_form = evform;
}


/***************************************
 * Handling of DestroyNotify events - only
 * sub-form gets this due to parent destroy
 ***************************************/

static void
handle_DestroyNotify_event( FL_FORM * evform )
{
	size_t i;

	evform->visible = 0;
	evform->window = 0;
	for ( i = 0; i < formnumb; i++ )
		if ( evform == forms[ i ] )
		{
			forms[ i ] = forms[ --formnumb ];
			forms = fl_realloc( forms, formnumb * sizeof *forms );
			return;
		}
}


/***************************************
 * Handle all events in the queue and flush output buffer
 ***************************************/

void
fl_treat_interaction_events( int wait )
{
    XEvent xev;

    /* if no event, output buffer will be flushed. If event exists,
       XNextEvent in do_interaction will flush the output buffer */

    do
		do_interaction_step( wait );
    while ( form_event_queued( &xev, QueuedAfterFlush ) );
}


/***************************************
 * Checks all forms. Does not wait.
 ***************************************/

FL_OBJECT *
fl_check_forms( void )
{
    FL_OBJECT *obj;

    if ( ( obj = fl_object_qread( ) ) == NULL )
    {
		fl_treat_interaction_events( 0 );
		fl_treat_user_events( );
		obj = fl_object_qread( );
    }

    return obj;
}


/***************************************
 * Checks all forms. Waits if nothing happens.
 ***************************************/

FL_OBJECT *
fl_do_forms( void )
{
    FL_OBJECT *obj;

    while ( ( obj = fl_object_qread( ) ) == NULL )
    {
		fl_treat_interaction_events( 1 );
		fl_treat_user_events( );
    }

	return obj;
}


/***************************************
 * Same as fl_check_forms but never returns FL_EVENT.
 ***************************************/

FL_OBJECT *
fl_check_only_forms( void )
{
    FL_OBJECT *obj;


    if ( ( obj = fl_object_qread( ) ) == NULL )
    {
		fl_treat_interaction_events( 0 );
		obj = fl_object_qread( );
    }

    return obj;
}


/***************************************
 * Same as fl_do_forms but never returns FL_EVENT.
 ***************************************/

FL_OBJECT *
fl_do_only_forms( void )
{
    FL_OBJECT *obj;

    while ( ( obj = fl_object_qread( ) ) == NULL )
		fl_treat_interaction_events( 1 );

	if ( obj == FL_EVENT )
		M_warn( "fl_do_only_forms", "Shouldn't happen" );

	return obj;
}


/***************************************
 * Check if an event exists that is meant for FORMS
 ***************************************/

static FL_FORM *
select_form_event( Display *  d  FL_UNUSED_ARG,
				   XEvent  *  xev )
{
    size_t i;
    Window win = ( ( XAnyEvent * ) xev )->window;

    for ( i = 0; i < formnumb; i++ )
		if ( win == forms[ i ]->window )
			return forms[ i ];

    return NULL;
}


/***************************************
 ***************************************/

static int
form_event_queued( XEvent * xev,
				   int      mode )
{
    if ( XEventsQueued( flx->display, mode ) )
    {
		XPeekEvent( flx->display, xev );
		return select_form_event( flx->display, xev ) != NULL;
    }

    return 0;
}


/***************************************
 * remove RCS keywords
 ***************************************/

const char *
fl_rm_rcs_kw( const char * s )
{
    static unsigned char buf[ 5 ][ 255 ];
    static int nbuf;
    unsigned char *q = buf[ ( nbuf = ( nbuf + 1 ) % 5 ) ];
    int left = 0,
		lastc = -1;


    while ( *s && ( q - buf[ nbuf ] ) < ( int ) sizeof buf[ nbuf ] - 2 )
    {
		switch ( *s )
		{
			case '$':
				if ( ( left = ! left ) )
					while ( *s && *s != ':' )
						s++;
				break;

			default:
				/* copy the char and remove extra space */
				if ( ! ( lastc == ' ' && *s == ' ' ) )
					*q++ = lastc = *s;
				break;
		}
		s++;
    }

    *q = '\0';

    return ( const char * ) buf[ nbuf ];
}


/***************************************
 ***************************************/

void
fl_set_initial_placement( FL_FORM * form,
						  FL_Coord  x,
						  FL_Coord  y,
						  FL_Coord  w,
						  FL_Coord  h )
{
    fl_set_form_position( form, x, y );
    fl_set_form_size( form, w, h );

    /* this alters the windowing defaults */

    fl_initial_wingeometry( form->x, form->y, form->w, form->h );
    has_initial = 1;
}


/***************************************
 * register pre-emptive event handlers
 ***************************************/

FL_RAW_CALLBACK
fl_register_raw_callback( FL_FORM *       form,
						  unsigned long   mask,
						  FL_RAW_CALLBACK rcb )
{
    FL_RAW_CALLBACK old_rcb = NULL;
    int valid = 0;

    if ( ! form )
    {
		Bark( "fl_register_raw_callback", "Null form" );
		return NULL;
    }

    if ( ( mask & FL_ALL_EVENT ) == FL_ALL_EVENT )
    {
		old_rcb = form->all_callback;
		form->evmask = mask;
		form->all_callback = rcb;
		return old_rcb;
    }

    if ( mask & ( KeyPressMask | KeyReleaseMask ) )
    {
		form->evmask |= mask & ( KeyPressMask | KeyReleaseMask );
		old_rcb = form->key_callback;
		form->key_callback = rcb;
		valid = 1;
    }

    if ( mask & ( ButtonPressMask | ButtonReleaseMask ) )
    {
		form->evmask |= mask & ( ButtonPressMask | ButtonReleaseMask );
		old_rcb = form->push_callback;
		form->push_callback = rcb;
		valid = 1;
    }

    if ( mask & ( EnterWindowMask | LeaveWindowMask ) )
    {
		form->evmask |= mask & ( EnterWindowMask | LeaveWindowMask );
		old_rcb = form->crossing_callback;
		form->crossing_callback = rcb;
		valid = 1;
    }

    if ( mask & ( ButtonMotionMask | PointerMotionMask ) )
    {
		form->evmask |= mask & ( ButtonMotionMask | PointerMotionMask );
		old_rcb = form->motion_callback;
		form->motion_callback = rcb;
		valid = 1;
    }

    if ( ! valid )			/* unsupported mask */
		Bark( "fl_register_raw_callback", "Unsupported mask 0x%x", mask );

    return old_rcb;
}


/***************************************
 ***************************************/

static int
pre_emptive_consumed( FL_FORM * form,
					  int       type,
					  XEvent *  xev )
{
    if ( ! form || ! form->evmask || form->deactivated )
		return 0;

    if (    ( form->evmask & FL_ALL_EVENT ) == FL_ALL_EVENT
		 && form->all_callback )
		return form->all_callback( form, xev );

    switch ( type )
    {
		case ButtonPress:
			if (    form->evmask & ButtonPressMask
				 && form->push_callback )
				return form->push_callback( form, xev );
			break;

		case ButtonRelease:
			if (    form->evmask & ButtonReleaseMask
				 && form->push_callback )
				return form->push_callback( form, xev );
			break;

		case KeyPress:
			if (    form->evmask & KeyPressMask
				 && form->key_callback )
				return form->key_callback( form, xev );
			break;

		case KeyRelease:
			if (    form->evmask & KeyRelease
				 && form->key_callback )
				return form->key_callback( form, xev );
			break;

		case EnterNotify:
			if (    form->evmask & EnterWindowMask
				 && form->crossing_callback )
				return form->crossing_callback( form, xev );
			break;

		case LeaveNotify:
			if (    form->evmask & LeaveWindowMask
				 && form->crossing_callback )
				return form->crossing_callback( form, xev );
			break;

		case MotionNotify:
			if (    form->evmask & ( ButtonMotionMask | PointerMotionMask )
				 && form->motion_callback )
				return form->motion_callback( form, xev );
    }

    return 0;
}


/***************************************
 ***************************************/

void
fl_set_form_event_cmask( FL_FORM *     form,
						 unsigned long cmask )
{
    if ( form )
		form->compress_mask = cmask;
}


/***************************************
 ***************************************/

unsigned long
fl_get_form_event_cmask( FL_FORM * form )
{
    return form ? form->compress_mask : 0UL;
}


/***************************************
 * Cleanup everything. At the moment, only need to restore the keyboard
 * settings and dealloctae memory used for the object and event queue.
 * This routine is registered as an atexit in fl_initialize in flresource.c
 ***************************************/

void
fl_finish( void )
{
    /* make sure the connection is alive */

    if ( flx->display )
    {
		XChangeKeyboardControl( flx->display, fl_keybdmask, &fl_keybdcontrol );

		fl_obj_queue_delete( );
		fl_event_queue_delete( );

		XCloseDisplay( flx->display );
		flx->display = fl_display = 0;
    }
}


/***************************************
 * Sets the call_back routine for the form
 ***************************************/

void
fl_set_form_callback( FL_FORM *          form,
					  FL_FORMCALLBACKPTR callback,
					  void *             d )
{
    if ( form == NULL )
    {
		fl_error( "fl_set_form_callback", "Setting callback of NULL form." );
		return;
    }

    form->form_callback = callback;
    form->form_cb_data = d;
}


/***************************************
 * currently only a single idle callback is support
 ***************************************/

static void
add_idle_callback( FL_APPEVENT_CB cb,
				   void *         data )
{
    if ( ! fl_context->idle_rec )
    {
		fl_context->idle_rec = fl_malloc( sizeof *fl_context->io_rec );
		fl_context->idle_rec->next = NULL;
    }

    fl_context->idle_rec->callback = cb;
    fl_context->idle_rec->data = data;
}


/***************************************
 * idle callback
 ***************************************/

FL_APPEVENT_CB
fl_set_idle_callback( FL_APPEVENT_CB callback,
					  void *         user_data )
{
    FL_APPEVENT_CB old =
		           fl_context->idle_rec ? fl_context->idle_rec->callback : NULL;

    add_idle_callback( callback, user_data );

    /* if we have idle callbacks, decrease the wait time */

    delta_msec = ( int ) ( TIMER_RES * ( callback ? 0.8 : 1.0 ) );

    fl_context->idle_delta = delta_msec;

    return old;
}


/***************************************
 ***************************************/

void
fl_set_idle_delta( long delta )
{
    if ( delta < 0 )
		delta = TIMER_RES;
    else if ( delta == 0 )
		delta = TIMER_RES / 10;

    delta_msec = delta;
    fl_context->idle_delta = delta;
}


/* xevent has about 10:1 priority as compared to user sockets */

static void handle_global_event( XEvent * );

#define IsGlobalEvent( xev )  ( xev->type == MappingNotify )


/***************************************
 ***************************************/

static int
get_next_event( int        wait_io,
				FL_FORM ** form,
				XEvent *   xev )
{
    int msec,
		has_event = 0;
    static int dox;

    if ( ++dox % 11 && XEventsQueued( flx->display, QueuedAfterFlush ) )
    {
		XNextEvent( flx->display, xev );

		if ( IsGlobalEvent( xev ) )
		{
			handle_global_event( xev );
			return has_event;
		}

		*form = select_form_event( flx->display, xev );
		has_event = *form != NULL;

		if ( ! has_event )
		{
			fl_compress_event( xev,   ExposureMask
							        | PointerMotionMask
							        | ButtonMotionMask );

			/* process user events */

#if FL_DEBUG >= ML_WARN
			if ( xev->type != MotionNotify || fl_cntl.debug > 2 )
				fl_xevent_name( "MainLoopUser", xev );
#endif

			fl_XPutBackEvent( xev );
		}
    }

    if ( has_event || do_x_only )
		return has_event;

    /* If incoming XEvent has already being pumped from the socket,
       watch_io() will time out, causing a bad delay in handling xevent.
       Make sure there is no event in the X event queue before we go into
       watch_io() */

    if ( dox % 11 && XEventsQueued( flx->display, QueuedAfterFlush ) )
		return has_event;

    /* compute how much time to wait */

    if ( ! wait_io )
		msec = SHORT_PAUSE;
    else if (    auto_count
			  || fl_pushobj
			  || fl_context->idle_rec
			  || fl_context->timeout_rec )
		msec = delta_msec;
	else
		msec = FL_min( delta_msec * 3, 300 );

    fl_watch_io( fl_context->io_rec, msec );

    if ( fl_context->timeout_rec )
		fl_handle_timeouts( msec );

    return has_event;
}


/***************************************
 ***************************************/

void
fl_set_form_icon( FL_FORM * form,
				  Pixmap    p,
				  Pixmap    m )
{
    if ( form )
    {
		form->icon_pixmap = p;
		form->icon_mask = m;
		if ( form->window )
			fl_winicon( form->window, p, m );
    }
}


/***************************************
 ***************************************/

void
fl_set_app_mainform( FL_FORM * form )
{
    fl_mainform = form;
    fl_set_form_property( form, FL_COMMAND_PROP );
}


/***************************************
 ***************************************/

FL_FORM *
fl_get_app_mainform( void )
{
    return nomainform ? NULL : fl_mainform;
}


/***************************************
 ***************************************/

void
fl_set_app_nomainform( int flag )
{
    nomainform = flag;
}


/***************************************
 ***************************************/

static void
handle_global_event( XEvent * xev )
{
    if ( xev->type == MappingNotify )
		XRefreshKeyboardMapping( ( XMappingEvent * ) &st_xev );
}


/***************************************
 ***************************************/

static void
simple_form_rescale( FL_FORM * form,
					 double    scale )
{
	form->w_hr *= scale;
	form->h_hr *= scale;

	form->w = FL_crnd( form->w_hr );
	form->h = FL_crnd( form->h_hr );
}


/***************************************
 * never shrinks a form. margin is the minimum margin to leave
 ***************************************/

void
fl_fit_object_label( FL_OBJECT * obj,
					 FL_Coord    xmargin,
					 FL_Coord    ymargin )
{
    FL_OBJECT *ob;
    int sw,
		sh,
		osize;
    double factor,
		   xfactor,
		   yfactor;

    if ( fl_no_connection )
		return;

    fl_get_string_dimension( obj->lstyle, obj->lsize, obj->label,
							 strlen( obj->label ), &sw, &sh );

    if (    sw <= obj->w - 2 * ( FL_abs( obj->bw ) + xmargin )
		 && sh <= obj->h - 2 * ( FL_abs( obj->bw ) + ymargin ) )
		return;

    if ( ( osize = obj->w - 2 * ( FL_abs( obj->bw ) + xmargin ) ) <= 0 )
		osize = 1;
    xfactor = ( double ) sw / osize;

    if ( ( osize = obj->h - 2 * ( FL_abs( obj->bw ) + ymargin ) ) <= 0 )
		osize = 1;
    yfactor = ( double ) sh / osize;

    factor = FL_max( xfactor, yfactor );

    if ( factor > 1.5 )
		factor = 1.5;

	simple_form_rescale( obj->form, factor );

    /* must suspend gravity and rescale */

    for ( ob = obj->form->first; ob; ob = ob->next )
		if (    ob->objclass != FL_BEGIN_GROUP
			 && ob->objclass != FL_END_GROUP )
			fl_scale_object( ob, factor, factor );

    fl_redraw_form( obj->form );
}


/***************************************
 ***************************************/

int
fl_is_good_form( FL_FORM * form )
{
	size_t i;

    for ( i = 0; i < formnumb; i++ )
		if ( forms[ i ] == form )
			return 1;

    if ( form )
		M_warn( "fl_is_good_form", "skipped invisible form" );

    return 0;
}


/***************************************
 ***************************************/

void
fl_recount_auto_object( void )
{
    size_t i;

    for ( auto_count = i = 0; i < formnumb; i++ )
		if ( forms[ i ]->has_auto )
			auto_count++;
}


/***************************************
 ***************************************/

void
fl_addto_group( FL_OBJECT * group )
{
    if ( group == 0 )
    {
		fl_error( "addto_group", "trying add to null group" );
		return;
    }

    if ( group->objclass != FL_BEGIN_GROUP )
    {
		fl_error( "addto_group", "parameter is not a group object" );
		return;
    }

    if ( fl_current_form && fl_current_form != group->form )
    {
		fl_error( "addto_group", "can't switch to a group on different from" );
		return;
    }

    if ( fl_current_group && fl_current_group != group )
    {
		fl_error( "addto_group", "you forgot to call fl_end_group" );
		fl_end_group( );
    }

    reopened_group = 1;
    reopened_group += fl_current_form == 0 ? 2 : 0;
    fl_current_form = group->form;
    fl_current_group = group;
}


/***************************************
 ***************************************/

int
fl_form_is_visible( FL_FORM * form )
{
    return ( form && form->window ) ? form->visible : FL_INVISIBLE;
}


/***************************************
 * similar to fit_object_label, but will do it for all objects and has
 * a smaller threshold. Mainly intended for compensation for font size
 * variations
 ***************************************/

double
fl_adjust_form_size( FL_FORM * form )
{
    FL_OBJECT *ob;
    double xfactor,
		   yfactor,
		   max_factor,
		   factor;
    int sw,
		sh,
		osize;
    double xm = 0.5,
		   ym = 0.5;
    int bw;

    if ( fl_no_connection )
		return 1.0;

    max_factor = factor = 1.0;
    for ( ob = form->first; ob; ob = ob->next )
    {
		if (    (    ob->align == FL_ALIGN_CENTER
				  || ob->align & FL_ALIGN_INSIDE
				  || ob->objclass == FL_INPUT )
			 && ! ob->is_child
			 && ob->label[ 0 ] != '\0'
			 && ob->label[ 0 ] != '@'
			 && ob->boxtype != FL_NO_BOX
			 && ( ob->boxtype != FL_FLAT_BOX || ob->objclass == FL_MENU ) )
		{
			fl_get_string_dimension( ob->lstyle, ob->lsize, ob->label,
									 strlen( ob->label ), &sw, &sh );

			bw = ( ob->boxtype == FL_UP_BOX || ob->boxtype == FL_DOWN_BOX ) ?
				 FL_abs( ob->bw ) : 1;

			if (    ob->objclass == FL_BUTTON
				 && (    ob->type == FL_RETURN_BUTTON
					  || ob->type == FL_MENU_BUTTON ) )
				sw += FL_min( 0.6 * ob->h, 0.6 * ob->w ) - 1;

			if ( ob->objclass == FL_BUTTON && ob->type == FL_LIGHTBUTTON )
				sw += FL_LIGHTBUTTON_MINSIZE + 1;

			if (    sw <= ob->w - 2 * ( bw + xm )
				 && sh <= ob->h - 2 * ( bw + ym ) )
				continue;

			if ( ( osize = ob->w - 2 * ( bw + xm ) ) <= 0 )
				osize = 1;
			xfactor = ( double ) sw / osize;

			if ( ( osize = ob->h - 2 * ( bw + ym ) ) <= 0 )
				osize = 1;
			yfactor = ( double ) sh / osize;

			if ( ob->objclass == FL_INPUT )
			{
				xfactor = 1.0;
				yfactor = ( sh + 1.6 ) / osize;
			}

			if ( ( factor = FL_max( xfactor, yfactor ) ) > max_factor )
				max_factor = factor;
		}
    }

    if ( max_factor <= 1.0 )
		return 1.0;

    max_factor = 0.01 * ( int ) ( max_factor * 100.0 );

    if ( max_factor > 1.25 )
		max_factor = 1.25;

    /* scale all objects. No gravity */

	simple_form_rescale( form, max_factor );

    for ( ob = form->first; ob; ob = ob->next )
		if ( ob->objclass != FL_BEGIN_GROUP && ob->objclass != FL_END_GROUP )
			fl_scale_object( ob, max_factor, max_factor );

    fl_redraw_form( form );

    return max_factor;
}


/***************************************
 ***************************************/

long
fl_mouse_button( void )
{
    return fl_context->mouse_button;
}


/***************************************
 ***************************************/

FL_TARGET *
fl_internal_init( void )
{
    static FL_TARGET *default_flx;

    if ( ! default_flx )
		default_flx = fl_calloc( 1, sizeof *default_flx );

    return flx = default_flx;
}


/***************************************
 * fl_display is exposed to the outside world. Bad
 ***************************************/

void
fl_switch_target( FL_TARGET * newtarget )
{
    flx = newtarget;
    fl_display = flx->display;
}


/***************************************
 ***************************************/

void
fl_restore_target( void )
{
    fl_internal_init( );
    fl_display = flx->display;
}


/***************************************
 ***************************************/

static int
fl_XLookupString( XKeyEvent * xkey,
				  char *      buf,
				  int         buflen,
				  KeySym *    ks )
{
    int len = INT_MIN;

    if ( ! fl_context->xic )
		len = XLookupString( xkey, buf, buflen, ks, 0 );
    else
    {
		Status status;

		if ( XFilterEvent( ( XEvent * ) xkey, None ) )
		{
			*ks = NoSymbol;
			return 0;
		}

		len =
			XmbLookupString( fl_context->xic, xkey, buf, buflen, ks, &status );

		if ( status == XBufferOverflow )
			len = -len;
	}

    return len;
}
