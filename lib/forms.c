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
 * \file forms.c
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 *  Main event dispatcher.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include "include/forms.h"
#include "flinternal.h"


static void force_visible( FL_FORM * );
static void set_form_property( FL_FORM *,
							   unsigned int );


static FL_FORM *fli_mainform;
static int nomainform;
static int reopened_group;

int fli_fast_free_object = 0;    /* exported to objects.c */

static int has_initial;


/***************************************
 * Returns the index of a form in the list of visible forms
 * or -1 if the form isn't in this list
 ***************************************/

int
fli_get_visible_forms_index( FL_FORM * form )
{
	int i;

    for ( i = 0; i < fli_int.formnumb; i++ )
		if ( fli_int.forms[ i ] == form )
			return i;

    return -1;
}


/***************************************
 * Returns the index of a form in the list of hidden forms
 * or -1 if the form isn't in this list
 ***************************************/

static int
get_hidden_forms_index( FL_FORM * form )
{
	int i;

    for ( i = fli_int.formnumb;
		  i < fli_int.formnumb + fli_int.hidden_formnumb; i++ )
		if ( fli_int.forms[ i ] == form )
			return i;

    return -1;
}


/***************************************
 * Adds a new form to the list of hidden forms, extending
 * the list in the process.
 ***************************************/

static void
add_form_to_hidden_list( FL_FORM * form )
{
	/* Extend the list of visible and hidden forms by one element
	   and put the new forms address into the new element */

	fli_int.forms = realloc( fli_int.forms,
							 ( fli_int.formnumb + fli_int.hidden_formnumb + 1 )
							 * sizeof *fli_int.forms );
	fli_int.forms[ fli_int.formnumb + fli_int.hidden_formnumb++ ] = form;
}


/***************************************
 * Moves a form from the list of hidden forms
 * to the list of visible forms
 ***************************************/

static int
move_form_to_visible_list( FL_FORM * form )
{
	int i;

	/* Find the index of the hidden form */

	if (    fli_int.hidden_formnumb == 0
		 || ( i = get_hidden_forms_index( form ) ) < 0 )
	{
		M_err( "move_form_to_visble_list", "Form not in hidden list" );
		return -1;
	}
		
	/* If it's not at the very start of the hidden list exchange it
	   with the one at the start */

	if ( i != fli_int.formnumb )
	{
		fli_int.forms[ i ] = fli_int.forms[ fli_int.formnumb ];
		fli_int.forms[ fli_int.formnumb ] = form;
	}

	fli_int.hidden_formnumb--;

    if ( form->has_auto_objects )
		fli_int.auto_count++;

	return ++fli_int.formnumb;
}


/***************************************
 * Moves a form from the list of visible forms
 * to the list of hidden forms
 ***************************************/

static int
move_form_to_hidden_list( FL_FORM * form )
{
	int i;

	/* Find the index of the form to be moved to the hidden list */

	if ( fli_int.formnumb == 0 || ( i = fli_get_visible_forms_index( form ) ) < 0 )
	{
		M_err( "move_form_to_hidden_list", "Form not in visible list" );
		return -1;
	}

	/* Unless the form is the very last in the visible list exchange it
	   with the form at the end of the visible list */

	if ( i != --fli_int.formnumb )
	{
		fli_int.forms[ i ] = fli_int.forms[ fli_int.formnumb ];
		fli_int.forms[ fli_int.formnumb ] = form;
	}

	fli_int.hidden_formnumb++;

    if ( form->has_auto_objects )
    {
		if ( fli_int.auto_count == 0 )
			M_err( "move_form_to_hidden_list", "Bad auto count" );
		else
			fli_int.auto_count--;
    }

	return fli_int.formnumb;
}


/***************************************
 * Removes a form from the list of hidden forms,
 * shortening the list in the process
 ***************************************/

int
remove_form_from_hidden_list( FL_FORM * form )
{
	int i;

	/* Find the index of the form to be removed completely from the
	   hidden list */

	if (    fli_int.hidden_formnumb == 0
		 || ( i = get_hidden_forms_index( form ) ) < 0 )
	{
		M_err( "remove_form_from_hidden_list", "Form not in hidden list" );
		return -1;
	}

	/* If it's not the form at the very end of the hidden list exchange
	   it with the one at the end */

	if ( i != fli_int.formnumb + --fli_int.hidden_formnumb )
		fli_int.forms[ i ] =
			       fli_int.forms[ fli_int.formnumb + fli_int.hidden_formnumb ];

	/* Shorten the list of visible and hidden forms by one element */

	fli_int.forms = fl_realloc( fli_int.forms,
								( fli_int.formnumb + fli_int.hidden_formnumb )
								* sizeof *fli_int.forms );

	return fli_int.formnumb;
}


/***************************************
 ***************************************/

FL_FORM *
fl_win_to_form( Window win )
{
	int i;

	if ( win == None )
		return NULL;

    for ( i = 0; i < fli_int.formnumb; i++ )
		if ( fli_int.forms[ i ]->window == win )
			return fli_int.forms[ i ];

    return NULL;
}


/***************************************
 * Starts a form definition
 ***************************************/

FL_FORM *
fl_bgn_form( int      type,
			 FL_Coord w,
			 FL_Coord h )
{
    if ( ! fli_no_connection && ! flx->display )
    {
		M_err( "fl_bgn_form", "Missing or failed call of fl_initialize()" );
		exit( 1 );
    }

	/* Check that we're not already in a form definition - an error actually
	   is serious and can't be fixed easily as it might be due to a bad
	   recursion */

    if ( fl_current_form )
    {
		M_err( "fl_bgn_form", "You forgot to call fl_end_form" );
		exit( 1 );
    }

	/* Create a new form */

    fl_current_form = fli_make_form( w, h );

	/* Add it to the list of still hidden forms */

	add_form_to_hidden_list( fl_current_form );

	/* Each form has a empty box, covering the whole form as its first
	   object */

    fl_add_box( type, 0, 0, w, h, "" );

    return fl_current_form;
}


/***************************************
 * Ends a form definition
 ***************************************/

void
fl_end_form( void )
{
    if ( ! fl_current_form )
		M_err( "fl_end_form", "No current form" );

    if ( fli_current_group )
    {
		M_err( "fl_end_form", "You forgot to call fl_end_group." );
		fl_end_group( );
    }

    fl_current_form = NULL;
}


/***************************************
 * Reopens a form for input
 ***************************************/

FL_FORM *
fl_addto_form( FL_FORM * form )
{
    if ( ! form )
    {
		M_err( "fl_addto_form", "NULL form" );
		return NULL;
    }

	/* Can't open a form for adding objects when another form is already
	   opened for the same purpose */

    if ( fl_current_form && fl_current_form != form )
	{
		M_err( "fl_addto_form", "You forgot to call fl_end_form" );
		return NULL;
	}

	if ( fl_current_form )
		M_warn( "fl_addto_form", "Form was never closed." );

    return fl_current_form = form;
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
		M_err( "fl_bgn_group", "NULL form" );
		return NULL;
    }

    if ( fli_current_group )
    {
		M_err( "fl_bgn_group", "You forgot to call fl_end_group." );
		fl_end_group( );
    }

    fli_current_group = fl_make_object( FL_BEGIN_GROUP, 0, 0, 10, 10, 0,
										"", NULL );
    fli_current_group->group_id = id++;

	/* Temporarily set the object class to something invalid since
	   fl_add_object() will not add objects of class FL_BEGIN_GROUP */

	fli_current_group->objclass = FL_INVALID_CLASS;
    fl_add_object( fl_current_form, fli_current_group );
	fli_current_group->objclass = FL_BEGIN_GROUP;

    return fli_current_group;
}


/***************************************
 * Ends a group definition
 ***************************************/

FL_OBJECT *
fli_end_group( void )
{
    FL_OBJECT *obj;
    int id;

    if ( ! fl_current_form )
    {
		M_err( "fl_end_group", "NULL form" );
		return NULL;
    }

    if ( ! fli_current_group )
    {
		M_err( "fl_end_group", "NULL group." );
		return NULL;
    }

	obj = fli_current_group;
    id = obj->group_id;
    fli_current_group = NULL;

    if ( ! reopened_group )
    {
		obj = fl_make_object( FL_END_GROUP, 0, 0, 0, 0, 0, "", NULL );
		obj->group_id = id;

		/* Temporarily set the object class to something invalid since
		   fl_add_object() will not add objects of class FL_END_GROUP */

		obj->objclass = FL_INVALID_CLASS;
		fl_add_object( fl_current_form, obj );
		obj->objclass = FL_END_GROUP;
    }

    if ( reopened_group == 3 )
		fl_end_form( );

    reopened_group = 0;

	return obj;
}


/***************************************
 ***************************************/

void
fl_end_group( void )
{
	fli_end_group( );
}


/***************************************
 * Function for "freezing" all (shown) forms
 ***************************************/

void
fl_freeze_all_forms( void )
{
    int i;

    for ( i = 0; i < fli_int.formnumb; i++ )
		fl_freeze_form( fli_int.forms[ i ] );
}


/***************************************
 * Function for "unfreezing" all (shown) forms
 ***************************************/

void
fl_unfreeze_all_forms( void )
{
	int i;

    for ( i = 0; i < fli_int.formnumb; i++ )
		fl_unfreeze_form( fli_int.forms[ i ] );
}


/***************************************
 * Corrects the shape of the form based on the shape of its window
 ***************************************/

static void
reshape_form( FL_FORM * form )
{
    FL_Coord w,
		     h,
		     dummy;
	int top,
		right,
		bottom,
		left;

	if (    ( ! form->handle_dec_x && ! form->handle_dec_y )
		 || form->wm_border == FL_NOBORDER )
	{
		fl_get_wingeometry( form->window, &form->x, &form->y, &w, &h );
		fl_set_form_size( form, w, h );
		return;
	}

	fl_get_decoration_sizes( form, &top, &right, &bottom, &left );

	if ( form->handle_dec_x && ! form->handle_dec_y )
	{
		fl_get_wingeometry( form->window, &dummy, &form->y, &w, &h );
		form->x -= left;
	}
	else if ( ! form->handle_dec_x && form->handle_dec_y )
	{
		fl_get_wingeometry( form->window, &form->x, &dummy, &w, &h );
		form->y -= bottom;
	}
	else
	{
		fl_get_wingeometry( form->window, &dummy, &dummy, &w, &h );
		form->x -= left;
		form->y -= bottom;
	}

	XMoveWindow( flx->display, form->window, form->x, form->y );
	fl_set_form_size( form, w, h );
}


/***************************************
 * Scale a form with the given scaling factors and take care of object
 * gravity. This one differs from fl_scale_form() in the fact that we
 * don't reshape the window in any way. Most useful as a follow up to
 * ConfigureNotify event
 ***************************************/

void
fli_scale_form( FL_FORM * form,
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

    /* Need to handle different resizing request */

    for ( obj = form->first; obj; obj = obj->next )
    {
		double oldw = obj->fl2 - obj->fl1;
		double oldh = obj->ft2 - obj->ft1;

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

		if ( fli_inverted_y )
			obj->y = form->h - obj->h - obj->y;

		fli_handle_object( obj, FL_RESIZED, 0, 0, 0, NULL, 0 );
    }

	fli_recalc_intersections( form );
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
		M_err( "fl_scale_form", "NULL form" );
		return;
    }

    if (    FL_crnd( form->w_hr * xsc ) == form->w
		 && FL_crnd( form->h_hr * ysc ) == form->h )
		return;

	fli_scale_form( form, xsc, ysc );

    /* Resize the window */

    if ( form->visible == FL_VISIBLE )
		fl_winresize( form->window, form->w, form->h );
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
		M_err( "fl_set_form_minsize", "Null form" );
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
		M_err( "fl_set_form_maxsize", "NULL form" );
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
    if ( ! form )
    {
		M_err( "fl_set_form_dblbuffer", "NULL form" );
		return;
    }

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
    if ( ! form )
    {
		M_err( "fl_set_form_size", "NULL form" );
		return;
    }

    if ( w != form->w || h != form->h )
		fl_scale_form( form, w / form->w_hr, h / form->h_hr );
}


/***************************************
 * Sets the position of the form on the screen.
 ***************************************/

void
fl_set_form_position( FL_FORM * form,
					  FL_Coord  x,
					  FL_Coord  y )
{
    FL_Coord oldx,
		     oldy;

    if ( ! form )
    {
		M_err( "fl_set_form_position", "NULL form" );
		return;
    }

    oldx = form->x;
    oldy = form->y;

	/* Negative values for x or y are interpreted as meaning that the
	   position is that of the right or bottom side of the form relative
	   to the right or bottom side to the screen. May have to be corrected
	   for the right or bottom border decoration widths. */

	if ( x >= 0 )
	{
		form->x = x;
		form->handle_dec_x = 0;
	}
	else
	{
		form->x = fl_scrw - form->w + x;
		form->handle_dec_x = 1;
	}

	if ( y >= 0 )
	{
		form->y = y;
		form->handle_dec_y = 0;
	}
	else
	{
		form->y = fl_scrh - form->h + y;
		form->handle_dec_y = 1;
	}

	/* If the form is already shown move it */

    if ( form->visible == FL_VISIBLE )
	{
		int bottom = 0,
			left = 0,
			dummy;

		if (    ( form->handle_dec_x || form->handle_dec_y )
			 && form->wm_border != FL_NOBORDER )
		{
			fl_get_decoration_sizes( form, &dummy, &dummy, &bottom, &left );

			if ( form->handle_dec_x )
				form->x -= left;

			if ( form->handle_dec_y )
				form->y -= bottom;
		}

		form->handle_dec_x = form->handle_dec_y = 0;

		if ( oldx != form->x || oldy != form->y )
			XMoveWindow( flx->display, form->window, form->x, form->y );
	}
}


/***************************************
 * Sets the position of the form on the screen.
 ***************************************/

void
fl_set_form_hotspot( FL_FORM * form,
					 FL_Coord  x,
					 FL_Coord  y )
{
    if ( ! form )
    {
		M_err( "fl_set_form_hotspot", "NULL form" );
		return;
    }

    form->hotx = x;
    form->hoty = y;
}


/***************************************
 ***************************************/

void
fl_set_form_hotobject( FL_FORM   * form,
					   FL_OBJECT * obj )
{
    if ( ! form  )
    {
		M_err( "fl_set_form_hotobject", "NULL form" );
		return;
    }

    if ( ! obj )
    {
		M_err( "fl_set_form_hotobject", "NULL object." );
		return;
    }


	fl_set_form_hotspot( form, obj->x + obj->w / 2, obj->y + obj->h / 2 );
}


/***************************************
 * Try to make sure a form is completely visible
 ***************************************/

static void
force_visible( FL_FORM * form )
{
    if ( form->x > fl_scrw - form->w )
		form->x = fl_scrw - form->w;

    if ( form->x < 0 )
		form->x = 0;

    if ( form->y > fl_scrh - form->h )
		form->y = fl_scrh - form->h;

    if ( form->y < 0 )
		form->y = 0;
}


/***************************************
 ***************************************/

void
fl_set_form_title( FL_FORM *    form,
				   const char * name )
{
    if ( ! form )
    {
		M_err( "fl_set_form_title", "NULL form" );
		return;
    }

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

Window
fl_prepare_form_window( FL_FORM    * form,
						int          place,
						int          border,
						const char * name )
{
    long screenw,
		 screenh,
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
		fl_current_form = NULL;
    }

    if ( form == NULL )
    {
		M_err( "fl_prepare_form", "NULL form" );
		return None;
    }

    if ( form->visible != FL_INVISIBLE )
		return form->window;

	/* Try to move the form from the part of the list for hidden forms to
	   tha at the start for visible forms */

	move_form_to_visible_list( form );

    if ( form->label != name )
    {
		if ( form->label )
			fl_free( form->label );
		form->label = fl_strdup( name ? name : "" );
    }

    if ( border == FL_NOBORDER )
		fli_int.unmanaged_count++;

    form->wm_border = border;
    form->deactivated = 0;
    screenw = fl_scrw;
    screenh = fl_scrh;

    fl_get_mouse( &mx, &my, &fli_int.keymask );

    if ( ( dont_fix_size = place & FL_FREE_SIZE ) )
		place &= ~ FL_FREE_SIZE;

    if ( place == FL_PLACE_SIZE )
		fl_pref_winsize( form->w, form->h );
    else if ( place == FL_PLACE_ASPECT )
		fl_winaspect( 0, form->w, form->h );
    else if ( place == FL_PLACE_POSITION )
    {
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
				if ( form->hotx < 0 || form->hoty < 0 )    /* not set */
				{
					form->hotx = form->w / 2;
					form->hoty = form->h / 2;
				}

				nmx = mx;
				nmy = my;
				form->x = mx - form->hotx;
				form->y = my - form->hoty;
				force_visible( form );
				nmx = form->x + form->hotx;
				nmy = form->y + form->hoty;
				if ( nmx != mx || nmy != my )
					fl_set_mouse( nmx, nmy );
				break;

			case FL_PLACE_GEOMETRY :
				if ( form->x < 0 )
				{
					form->x = screenw - form->w + form->x;
					form->handle_dec_x = 1;
				}
				if ( form->y < 0 )
				{
					form->y = screenh - form->h + form->y;
					form->handle_dec_y = 1;
				}
				break;
		}

		/* Final check. Make sure form is visible */

		if ( place != FL_PLACE_GEOMETRY )
			force_visible( form );

		if ( dont_fix_size && place != FL_PLACE_GEOMETRY )
			fl_initial_wingeometry( form->x, form->y, form->w, form->h );
		else
			fl_pref_wingeometry( form->x, form->y, form->w, form->h );
    }
    else if ( place == FL_PLACE_FREE )
    {
		fl_initial_winsize( form->w, form->h );
		if ( has_initial )
			fl_initial_wingeometry( form->x, form->y, form->w, form->h );
    }
    else
    {
		M_err( "fl_prepare_form_window", "Unknown requests: %d", place );
		fl_initial_wingeometry( form->x, form->y, form->w, form->h );
    }

    /* Window managers typically do not allow dragging transient windows */

    if ( border != FL_FULLBORDER )
    {
		if ( place == FL_PLACE_ASPECT || place == FL_PLACE_FREE )
		{
			form->x = mx - form->w / 2;
			form->y = my - form->h / 2;
			force_visible( form );
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
    fli_init_colormap( fl_vmode );

    form->window = fli_create_window( fl_root, fli_colormap( fl_vmode ), name );
    fl_winicontitle( form->window, name );

    if ( border == FL_FULLBORDER || form->prop & FLI_COMMAND_PROP )
		set_form_property( form, FLI_COMMAND_PROP );

    return form->window;
}


/***************************************
 ***************************************/

Window
fl_show_form_window( FL_FORM * form )
{
	FL_OBJECT *obj;

    if ( ! form  )
    {
		M_err( "fl_show_form_window", "NULL form" );
		return None;
    }

    if ( form->window == None || form->visible != FL_INVISIBLE )
		return form->window;

    fl_winshow( form->window );
    form->visible = FL_VISIBLE;
    reshape_form( form );
	fl_redraw_form( form );

	/* TODO: somehow formbrowser objects get drawn incorrectly the first
	   time round so, for the time being, we redraw it once again... */

	for ( obj = form->first; obj; obj = obj->next )
		if ( obj->objclass == FL_FORMBROWSER )
			fl_redraw_object( obj );

	if ( ! form->focusobj )
		for ( obj = form->first; obj; obj = obj->next )
			if ( obj->input && obj->active )
			{
				fl_set_focus_object( form, obj );
				break;
			}

    return form->window;
}


/***************************************
 ***************************************/

Window
fl_show_form( FL_FORM *    form,
			  int          place,
			  int          border,
			  const char * name )
{
    if ( ! form  )
    {
		M_err( "fl_show_form", "NULL form" );
		return None;
    }

    fl_prepare_form_window( form, place, border, name );
    return fl_show_form_window( form );
}


/***************************************
 * Hides a particular form
 ***************************************/

static void
close_form_window( Window win )
{
    XEvent xev;

    XUnmapWindow( flx->display, win );
    XDestroyWindow( flx->display, win );

    XSync( flx->display, 0 );
    while ( XCheckWindowEvent( flx->display, win, AllEventsMask, &xev ) )
		fli_xevent_name( "Eaten", &xev );

    /* Give subwindows a chance to handle destroy event promptly, take care
	   the window of the form doesn't exist anymore! */

    while ( XCheckTypedEvent( flx->display, DestroyNotify, &xev ) )
    {
		FL_FORM *form;

		if ( ( form = fli_find_event_form( &xev ) ) )
		{
			form->window = None;
			fl_hide_form( form );
		}
		else
			fl_XPutBackEvent( &xev );
    }
}


/***************************************
 ***************************************/

static FL_FORM *
property_set( unsigned int prop )
{
    int i;

    for ( i = 0; i < fli_int.formnumb; i++ )
		if (    fli_int.forms[ i ]->prop & prop
			 && fli_int.forms[ i ]->prop & FLI_PROP_SET )
			return fli_int.forms[ i ];

    return NULL;
}


/***************************************
 ***************************************/

static void
set_form_property( FL_FORM *    form,
				   unsigned int prop )
{
    if ( ! form  )
    {
		M_err( "set_form_property", "NULL form" );
		return;
    }

    if ( property_set( prop ) )
		return;

    if ( ! ( prop & FLI_COMMAND_PROP ) )
	{
		M_err( "set_form_property", "Unknown form property request %u",
			   prop );
		return;
	}

	if ( form->window )
	{
		fli_set_winproperty( form->window, FLI_COMMAND_PROP );
		form->prop |= FLI_PROP_SET;
	}

	form->prop |= FLI_COMMAND_PROP;
	fli_mainform = form;
}


/***************************************
 ***************************************/

void
fl_hide_form( FL_FORM * form )
{
    Window owin;
	FL_OBJECT *o;

    if ( ! form )
    {
		M_err( "fl_hide_form", "NULL form" );
		return;
    }

    if ( fli_get_visible_forms_index( form ) < 0 )
    {
		M_err( "fl_hide_form", "Hiding unknown form" );
		return;
    }

    if ( form->visible == FL_BEING_HIDDEN )
    {
		M_err( "fl_hide_form", "Recursive call?" );
		return;
    }

    form->visible = FL_BEING_HIDDEN;
    fli_set_form_window( form );

    /* Checking mouseobj->form is necessary as it might be deleted from a
       form */

    if ( fli_int.mouseobj != NULL && fli_int.mouseobj->form == form )
    {
		fli_handle_object( fli_int.mouseobj, FL_LEAVE, 0, 0, 0, NULL, 1 );
		fli_int.mouseobj = NULL;
    }

    if ( fli_int.pushobj != NULL && fli_int.pushobj->form == form )
    {
		fli_handle_object( fli_int.pushobj, FL_RELEASE, 0, 0, 0, NULL, 1 );
		fli_int.pushobj = NULL;
    }

    if ( form->focusobj )
	{
		fli_handle_object( form->focusobj, FL_UNFOCUS, 0, 0, 0, NULL, 0 );
		form->focusobj = NULL;
	}

	/* Get canvas objects to unmap their windows (but only for those that
	   aren't childs, those will be dealt with by their parents) */

	for ( o = form->first; o; o = o->next )
		if (    ( o->objclass == FL_CANVAS || o->objclass == FL_GLCANVAS )
			 && ! o->parent )
			fli_unmap_canvas_window( o );

#ifdef DELAYED_ACTION
    fli_object_qflush( form );
#endif

    /* Free backing store pixmap but keep the pointer */

    fli_free_flpixmap( form->flpixmap );

    if ( fli_int.mouseform && fli_int.mouseform->window == form->window )
		fli_int.mouseform = NULL;

    form->deactivated = 1;
    form->visible = FL_INVISIBLE;
    owin = form->window;
    form->window = None;

    fli_hide_tooltip( );

	/* If the forms window is None it already has been closed */

	if ( owin )
		close_form_window( owin );

	if ( flx->win == owin )
		flx->win = None;

	/* Move the form from the part of the list for visible forms to the
	   part of hidden forms at the end of the array */

	move_form_to_hidden_list( form );

    if ( form->wm_border == FL_NOBORDER )
    {
		fli_int.unmanaged_count--;
		if ( fli_int.unmanaged_count < 0 )
		{
			M_err( "fl_hide_form", "Bad unmanaged count" );
			fli_int.unmanaged_count = 0;
		}
    }

    /* Need to re-establish command property */

    if ( fli_int.formnumb && form->prop & FLI_COMMAND_PROP )
		set_form_property( fli_int.forms[ 0 ], FLI_COMMAND_PROP );

    if ( form == fli_int.keyform )
		fli_int.keyform = NULL;
}


/***************************************
 * Frees the memory used by a form, together with all its objects.
 ***************************************/

void
fl_free_form( FL_FORM * form )
{
    /* Check whether ok to free */

    if ( ! form )
    {
		M_err( "fl_free_form", "NULL form" );
		return;
    }

    if ( form->visible == FL_VISIBLE )
    {
		M_warn( "fl_free_form", "Freeing visible form" );
		fl_hide_form( form );
    }

    if ( get_hidden_forms_index( form ) < 0 )
    {
		M_err( "fl_free_form", "Freeing unknown form" );
		return;
    }

    /* Free all objects of the form */

	fli_fast_free_object = 1;

	while ( form->first )
		fl_free_object( form->first );

	fli_fast_free_object = 0;

    if ( form->flpixmap )
    {
		fli_free_flpixmap( form->flpixmap );
		fl_free( form->flpixmap );
    }

    if ( form->label )
    {
		fl_free( form->label );
		form->label = NULL;
    }

    if ( form == fli_mainform )
		fli_mainform = NULL;

    /* Free the form and remove it from the list of existing forms */

	fl_free( form );

	remove_form_from_hidden_list( form );
}


/***************************************
 * activates a form
 ***************************************/

void
fl_activate_form( FL_FORM * form )
{
    if ( ! form )
    {
		M_err( "fl_activate_form", "NULL form" );
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
 * Deactivates a form
 ***************************************/

void
fl_deactivate_form( FL_FORM * form )
{
    if ( ! form )
    {
		M_err( "fl_deactivate_form", "NULL form" );
		return;
    }

    if (    ! form->deactivated
		 && fli_int.mouseobj != NULL
		 && fli_int.mouseobj->form == form )
		fli_handle_object( fli_int.mouseobj, FL_LEAVE, 0, 0, 0, NULL, 1 );

    if ( ! form->deactivated && form->deactivate_callback )
		form->deactivate_callback( form, form->deactivate_data );

    form->deactivated++;

    if ( form->child )
		fl_deactivate_form( form->child );
}


/***************************************
 ***************************************/

FL_FORM_ATACTIVATE
fl_set_form_atactivate( FL_FORM            * form,
						FL_FORM_ATACTIVATE   cb,
						void *               data )
{
    FL_FORM_ATACTIVATE old = NULL;

    if ( ! form  )
    {
		M_err( "fl_set_form_atactivate", "NULL form" );
		return NULL;
    }

	old = form->activate_callback;
	form->activate_callback = cb;
	form->activate_data = data;

    return old;
}


/***************************************
 ***************************************/

FL_FORM_ATDEACTIVATE
fl_set_form_atdeactivate( FL_FORM              * form,
						  FL_FORM_ATDEACTIVATE   cb,
						  void                 * data )
{
    FL_FORM_ATDEACTIVATE old = NULL;

    if ( ! form  )
    {
		M_err( "fl_set_form_atdeactivate", "NULL form" );
		return NULL;
    }

	old = form->deactivate_callback;
	form->deactivate_callback = cb;
	form->deactivate_data = data;

    return old;
}


/***************************************
 * activates all forms
 ***************************************/

void
fl_activate_all_forms( void )
{
    int i;

    for ( i = 0; i < fli_int.formnumb; i++ )
		fl_activate_form( fli_int.forms[ i ] );
}


/***************************************
 * Deactivates all forms
 ***************************************/

void
fl_deactivate_all_forms( void )
{
    int i;

    for ( i = 0; i < fli_int.formnumb; i++ )
		fl_deactivate_form( fli_int.forms[ i ] );
}


/***************************************
 ***************************************/

FL_FORM_ATCLOSE
fl_set_form_atclose( FL_FORM         * form,
					 FL_FORM_ATCLOSE   fmclose,
					 void            * data )
{
    FL_FORM_ATCLOSE old;

    if ( ! form  )
    {
		M_err( "fl_set_form_atclose", "NULL form" );
		return NULL;
    }

	old = form->close_callback;
    form->close_callback = fmclose;
    form->close_data = data;

    return old;
}


/***************************************
 ***************************************/

FL_FORM_ATCLOSE
fl_set_atclose( FL_FORM_ATCLOSE   fmclose,
				void            * data )
{
    FL_FORM_ATCLOSE old = fli_context->atclose;

    fli_context->atclose = fmclose;
    fli_context->close_data = data;

    return old;
}


/***************************************
 ***************************************/

void
fl_set_initial_placement( FL_FORM  * form,
						  FL_Coord   x,
						  FL_Coord   y,
						  FL_Coord   w,
						  FL_Coord   h )
{
    fl_set_form_position( form, x, y );
    fl_set_form_size( form, w, h );

    /* This alters the windowing defaults */

    fl_initial_wingeometry( form->x, form->y, form->w, form->h );
    has_initial = 1;
}


/***************************************
 * Register pre-emptive event handlers
 ***************************************/

FL_RAW_CALLBACK
fl_register_raw_callback( FL_FORM         * form,
						  unsigned long     mask,
						  FL_RAW_CALLBACK   rcb )
{
    FL_RAW_CALLBACK old_rcb = NULL;
    int valid = 0;

    if ( ! form )
    {
		M_err( "fl_register_raw_callback", "Null form" );
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
		M_err( "fl_register_raw_callback", "Unsupported mask 0x%x", mask );

    return old_rcb;
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
 * Sets the call_back routine for the form
 ***************************************/

void
fl_set_form_callback( FL_FORM            * form,
					  FL_FORMCALLBACKPTR   callback,
					  void *               d )
{
    if ( form == NULL )
    {
		M_err( "fl_set_form_callback", "NULL form" );
		return;
    }

    form->form_callback = callback;
    form->form_cb_data = d;
}


/***************************************
 ***************************************/

void
fl_set_form_icon( FL_FORM * form,
				  Pixmap    p,
				  Pixmap    m )
{
    if ( ! form )
		return;

	form->icon_pixmap = p;
	form->icon_mask = m;
	if ( form->window )
		fl_winicon( form->window, p, m );
}


/***************************************
 ***************************************/

void
fl_set_app_mainform( FL_FORM * form )
{
    fli_mainform = form;
    set_form_property( form, FLI_COMMAND_PROP );
}


/***************************************
 ***************************************/

FL_FORM *
fl_get_app_mainform( void )
{
    return nomainform ? NULL : fli_mainform;
}


/***************************************
 ***************************************/

void
fl_set_app_nomainform( int flag )
{
    nomainform = flag;
}


/***************************************
 * Does a rescale of a form without taking into
 * account object gravity or resize settings
 ***************************************/

static void
simple_form_rescale( FL_FORM * form,
					 double    scale )
{
    FL_OBJECT *obj;

	form->w_hr *= scale;
	form->h_hr *= scale;

	form->w = FL_crnd( form->w_hr );
	form->h = FL_crnd( form->h_hr );

    for ( obj = form->first; obj; obj = obj->next )
		if ( obj->objclass != FL_BEGIN_GROUP && obj->objclass != FL_END_GROUP )
			fl_scale_object( obj, scale, scale );

    fl_redraw_form( form );
}


/***************************************
 * Never shrinks a form, margin is the minimum margin to leave
 ***************************************/

void
fl_fit_object_label( FL_OBJECT * obj,
					 FL_Coord    xmargin,
					 FL_Coord    ymargin )
{
    int sw,
		sh,
		osize;
    double factor,
		   xfactor,
		   yfactor;

    if ( fli_no_connection )
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

    /* Scale all objects without taking care of gravity etc. */

	simple_form_rescale( obj->form, factor );
}


/***************************************
 ***************************************/

void
fli_recount_auto_objects( void )
{
    int i;

    for ( fli_int.auto_count = i = 0; i < fli_int.formnumb; i++ )
		if ( fli_int.forms[ i ]->has_auto_objects )
			fli_int.auto_count++;
}


/***************************************
 * Function for adding an object to the (currently open) group
 ***************************************/

FL_OBJECT *
fl_addto_group( FL_OBJECT * group )
{
    if ( group == NULL )
    {
		M_err( "fl_addto_group", "NULL group." );
		return NULL;
    }

    if ( group->objclass != FL_BEGIN_GROUP )
    {
		M_err( "fl_addto_group", "Parameter is not a group object." );
		return NULL;
    }

    if ( fl_current_form && fl_current_form != group->form )
    {
		M_err( "fl_addto_group",
			   "Can't switch to a group on different form" );
		return NULL;
    }

    if ( fli_current_group && fli_current_group != group )
    {
		M_err( "fl_addto_group", "You forgot to call fl_end_group" );
		return NULL;
    }

	if ( fli_current_group )
		M_warn( "fl_addto_group", "Group was never closed" );

    reopened_group = 1;
    reopened_group += fl_current_form ? 0 : 2;
    fl_current_form = group->form;
    return fli_current_group = group;
}


/***************************************
 ***************************************/

int
fl_form_is_visible( FL_FORM * form )
{
	if ( ! form )
	{
		M_warn( "fl_form_is_visible", "NULL form" );
		return FL_INVISIBLE;
	}

    return form->window ? form->visible : FL_INVISIBLE;
}


/***************************************
 * Similar to fit_object_label, but will do it for all objects and has
 * a smaller threshold. Mainly intended for compensation for font size
 * variations
 ***************************************/

double
fl_adjust_form_size( FL_FORM * form )
{
    FL_OBJECT *obj;
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

    if ( fli_no_connection )
		return 1.0;

    max_factor = factor = 1.0;
    for ( obj = form->first; obj; obj = obj->next )
    {
		if (    (    obj->align == FL_ALIGN_CENTER
				  || obj->align & FL_ALIGN_INSIDE
				  || obj->objclass == FL_INPUT )
			 && ! obj->parent
			 && obj->label[ 0 ] != '\0'
			 && obj->label[ 0 ] != '@'
			 && obj->boxtype != FL_NO_BOX
			 && ( obj->boxtype != FL_FLAT_BOX || obj->objclass == FL_MENU ) )
		{
			fl_get_string_dimension( obj->lstyle, obj->lsize, obj->label,
									 strlen( obj->label ), &sw, &sh );

			bw = ( obj->boxtype == FL_UP_BOX || obj->boxtype == FL_DOWN_BOX ) ?
				 FL_abs( obj->bw ) : 1;

			if (    obj->objclass == FL_BUTTON
				 && (    obj->type == FL_RETURN_BUTTON
					  || obj->type == FL_MENU_BUTTON ) )
				sw += FL_min( 0.6 * obj->h, 0.6 * obj->w ) - 1;

			if ( obj->objclass == FL_BUTTON && obj->type == FL_LIGHTBUTTON )
				sw += FL_LIGHTBUTTON_MINSIZE + 1;

			if (    sw <= obj->w - 2 * ( bw + xm )
				 && sh <= obj->h - 2 * ( bw + ym ) )
				continue;

			if ( ( osize = obj->w - 2 * ( bw + xm ) ) <= 0 )
				osize = 1;
			xfactor = ( double ) sw / osize;

			if ( ( osize = obj->h - 2 * ( bw + ym ) ) <= 0 )
				osize = 1;
			yfactor = ( double ) sh / osize;

			if ( obj->objclass == FL_INPUT )
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

    /* Scale all objects without taking care of gravity etc. */

	simple_form_rescale( form, max_factor );

    return max_factor;
}


/***************************************
 * Returns the sizes of the "descorations" the window manager puts around
 * a forms window. Returns 0 on success and 1 if the form isn't visisble
 * or is a form embedded into another form.
 * This first tries to use the "_NET_FRAME_EXTENTS" atom which window
 * manager in principle should set for windows that have decorations.
 * For those window managers that don't have that atom we try it with the
 * old trick of searching up for the parent window that's either Null or
 * is a direct child of the root window and using this window's geometry
 * (but note: this doesn't work with window managers that don't reparent
 * the windows they manage, but we can't recognize that).
 ***************************************/

int
fl_get_decoration_sizes( FL_FORM * form,
						 int     * top,
						 int     * right,
						 int     * bottom,
						 int     * left )
{
	Atom a;

	if (    ! form
		 || ! form->window
		 || form->visible != FL_VISIBLE
		 || form->parent )
		return 1;

	*top = *right = *bottom = *left = 0;

	/* If the window manager knows about the '_NET_FRAME_EXTENTS' ask for
	   the settings for the form's window (if there are none the window
	   probably has no decorations) */

    if ( ( a = XInternAtom( fl_get_display( ), "_NET_FRAME_EXTENTS", True ) )
                                                                       != None )
	{
		Atom actual_type;
		int actual_format;
		unsigned long nitems;
		unsigned long bytes_after;
		static unsigned char *prop;

        XGetWindowProperty( fl_get_display( ), form->window, a, 0,
                            4, False, XA_CARDINAL,
                            &actual_type, &actual_format, &nitems,
                            &bytes_after, &prop );

		if (    actual_type == XA_CARDINAL
			 && actual_format == 32
			 && nitems == 4 )
		{
			*top    = ( ( long * ) prop )[ 2 ];
			*right  = ( ( long * ) prop )[ 1 ];
			*bottom = ( ( long * ) prop )[ 3 ];
			*left   = ( ( long * ) prop )[ 0 ];
		}
	}
	else
	{
		/* The window manager doesn't have the _NET_FRAME_EXTENDS atom so we
		   have to try with the traditional method (which assumes that the
           window manager reparents the windows it manages) */

		Window cur_win = form->window;
		Window root;
		Window parent;
		Window *childs = NULL;
		XWindowAttributes win_attr;
		XWindowAttributes frame_attr;
		Window wdummy;
		unsigned int udummy;


        /* Get the coordinates and size of the form's window */

		XGetWindowAttributes( fl_get_display( ), cur_win, &win_attr );

        /* Check try to get its parent window */

		XQueryTree( fl_get_display( ), cur_win, &root, &parent, &childs,
                    &udummy );
		if ( childs )
		{
			XFree( childs );
			childs = NULL;
		}

		/* If there's no parent or the parent window is the root window
		   itself we've got to assume that there are no decorations */

		if ( ! parent || parent == root )
			return 0;

        /* Now translate the form window's coordiates (that are relative to
		   its parent) to that relative to the root window and then find the
		   top-most parent that isn't the root window itself */

		XTranslateCoordinates( fl_get_display( ), parent, root,
							   win_attr.x, win_attr.y,
							   &win_attr.x, &win_attr.y, &wdummy );

		while ( parent && parent != root )
		{
			cur_win = parent;
			XQueryTree( fl_get_display( ), cur_win, &root, &parent, &childs,
						&udummy );
			if ( childs )
			{
				XFree( childs );
				childs = NULL;
			}
		}

        /* Get the cordinates and sizes of that top-most window... */

		XGetWindowAttributes( fl_get_display( ), cur_win, &frame_attr );

        /* ...and finally calculate the decoration sizes */

		*top    = win_attr.y - frame_attr.y;
		*left   = win_attr.x - frame_attr.x;
		*bottom = frame_attr.height - win_attr.height - *top;
		*right  = frame_attr.width - win_attr.width - *left;
	}

	return 0;
}


/***************************************
 * Function returns if a form's window is in iconified state
 ***************************************/

int
fl_form_is_iconified( FL_FORM * form )
{
	XWindowAttributes xwa;

	if ( ! form )
	{
		M_err( "fl_form_is_iconified", "NULL form" );
		return 0;
	}

	if ( ! form->window || form->visible == FL_INVISIBLE )
		return 0;

	XGetWindowAttributes( fl_get_display( ), form->window, &xwa );

	return xwa.map_state != IsViewable;
}
