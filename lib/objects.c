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
 * \file objects.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_obj = "$Id: objects.c,v 1.49 2009/01/02 17:58:25 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include <string.h>


extern FL_OBJECT * fli_mouseobj,          /* defined in forms.c */
                 * fli_pushobj;


#define PointToPixel( a )     FL_crnd( ( a ) * fli_dpi / 72.0   )
#define MMToPixel( a )        FL_crnd( ( a ) * fli_dpi / 25.4   )
#define CMMToPixel( a )       FL_crnd( ( a ) * fli_dpi / 2540.0 )
#define CPointToPixel( a )    FL_crnd( ( a ) * fli_dpi / 7200.0 )

#define TRANY( obj, form )    ( form->h - obj->h - obj->y )

#define LInside( a )    \
               ( ( ( a ) == FL_ALIGN_CENTER ) || ( ( a ) & FL_ALIGN_INSIDE ) )


extern int fli_fast_free_object;     /* defined in forms.c */

static void lose_focus( FL_OBJECT * );
static void get_object_bbox_rect( FL_OBJECT *,
								  XRectangle * );
static int objects_intersect( FL_OBJECT *,
							  FL_OBJECT * );
static int object_is_under( FL_OBJECT * );
static void checked_hide_tooltip( FL_OBJECT *,
								  XEvent    * );

static FL_OBJECT *refocus;


/***************************************
 * Creates an empty form
 ***************************************/

FL_FORM *
fli_make_form( FL_Coord w,
			   FL_Coord h )
{
    FL_FORM *form;

    form = fl_calloc( 1, sizeof *form );

    /* convert non-pixel unit into pixles */

	switch ( fli_cntl.coordUnit )
	{
		case FL_COORD_PIXEL :
			break;

		case FL_COORD_MM :
			w = MMToPixel( w );
			h = MMToPixel( h );
			break;

		case FL_COORD_POINT :
			w = PointToPixel( w );
			h = PointToPixel( h );
			break;

		case FL_COORD_centiPOINT :
			w = CPointToPixel( w );
			h = CPointToPixel( h );
			break;

		case FL_COORD_centiMM :
			w = CMMToPixel( w );
			h = CMMToPixel( h );
			break;

		default :
			M_err( "fli_make_form", "Unknown unit: %d. Reset to pixel",
				   fli_cntl.coordUnit );
			fli_cntl.coordUnit = FL_COORD_PIXEL;
    }

    /* initialize pointers and non-zero defaults */

    form->w_hr = form->w = w;
    form->h_hr = form->h = h;

	form->handle_dec_x = 0;
	form->handle_dec_y = 0;

    form->deactivated    = 1;
    form->form_callback  = NULL;
    form->compress_mask  = ExposureMask | ButtonMotionMask | PointerMotionMask;
    form->key_callback   =
	form->push_callback  = form->crossing_callback = NULL;
    form->focusobj       = NULL;
    form->first          = form->last           = NULL;
    form->hotx           = form->hoty = -1;
    form->use_pixmap     = fli_cntl.doubleBuffer;
    form->label          = NULL;
    form->u_vdata        = NULL;
    form->close_callback = NULL;
    form->close_data     = NULL;
    form->icon_pixmap    = form->icon_mask = None;
    form->no_tooltip     = 0;

    return form;
}


#define IS_BUTTON_CLASS( i )   (    i == FL_BUTTON           \
							     || i == FL_ROUNDBUTTON      \
							     || i == FL_ROUND3DBUTTON    \
							     || i == FL_LIGHTBUTTON      \
							     || i == FL_CHECKBUTTON      \
							     || i == FL_BITMAPBUTTON     \
							     || i == FL_PIXMAPBUTTON )


/***************************************
 * Creates an object, NOT FOR USER.
 ***************************************/

FL_OBJECT *
fl_make_object( int            objclass,
				int            type,
				FL_Coord       x,
				FL_Coord       y,
				FL_Coord       w,
				FL_Coord       h,
				const char   * label,
				FL_HANDLEPTR   handle )
{
    FL_OBJECT *obj;
#ifdef FL_WIN32
    int def = -2;
#else
    int def = FL_BOUND_WIDTH;
#endif

    obj = fl_calloc( 1, sizeof *obj );

    obj->objclass  = objclass;
    obj->type      = type;
    obj->resize    = FL_RESIZE_ALL;
    obj->nwgravity = obj->segravity = FL_NoGravity;
    obj->boxtype   = FL_NO_BOX;
    obj->bw        = (    fli_cntl.borderWidth
					   && FL_abs( fli_cntl.borderWidth ) <= FL_MAX_BW ) ?
		             fli_cntl.borderWidth : def;

    obj->x         = x;
    obj->y         = y;
    obj->w         = w;
    obj->h         = h;

	switch ( fli_cntl.coordUnit )
	{
		case FL_COORD_PIXEL :
			break;

		case FL_COORD_MM :
			fl_scale_object( obj, fli_dpi / 25.4, fli_dpi / 25.4 );
			break;

		case FL_COORD_POINT :
			fl_scale_object( obj, fli_dpi / 72.0, fli_dpi / 72.0 );
			break;

		case FL_COORD_centiPOINT :
			fl_scale_object( obj, fli_dpi / 7200.0, fli_dpi / 7200.0 );
			break;

		case FL_COORD_centiMM :
			fl_scale_object( obj, fli_dpi / 2540.0, fli_dpi / 2540.0 );
			break;

		default:
			M_err( "fl_make_object", "Unknown unit: %d. Reset",
				   fli_cntl.coordUnit );
			fli_cntl.coordUnit = FL_COORD_PIXEL;
    }

    obj->wantkey  = FL_KEY_NORMAL;

    obj->flpixmap = NULL;

    obj->label    = fl_strdup( label ? label : "" );
    obj->handle   = handle;
    obj->align    = FL_ALIGN_CENTER;
    obj->lcol     = FL_BLACK;
    obj->col1     = FL_COL1;
    obj->col2     = FL_MCOL;

    if ( IS_BUTTON_CLASS( objclass ) && fli_cntl.buttonFontSize )
		obj->lsize = fli_cntl.buttonFontSize;
    else if ( objclass == FL_MENU && fli_cntl.menuFontSize )
		obj->lsize = fli_cntl.menuFontSize;
    else if (    ( objclass == FL_CHOICE || objclass == FL_SELECT )
			  && fli_cntl.choiceFontSize )
		obj->lsize = fli_cntl.choiceFontSize;
    else if ( objclass == FL_INPUT && fli_cntl.inputFontSize )
		obj->lsize = fli_cntl.inputFontSize;
    else if ( objclass == FL_SLIDER && fli_cntl.sliderFontSize )
		obj->lsize = fli_cntl.sliderFontSize;
#if 0
    else if ( objclass == FL_BROWSER && fli_cntl.browserFontSize )
		obj->lsize = fli_cntl.browserFontSize;
#endif
    else if ( fli_cntl.labelFontSize )
		obj->lsize = fli_cntl.labelFontSize;
    else
		obj->lsize = FL_DEFAULT_SIZE;

    obj->lstyle             = FL_NORMAL_STYLE;
    obj->shortcut           = fl_calloc( 1, sizeof( long ) );
    obj->shortcut[ 0 ]      = 0;
    obj->active             = 1;
    obj->visible            = FL_VISIBLE;
    obj->object_callback    = NULL;
    obj->spec               = NULL;
    obj->next = obj->prev   = NULL;
    obj->form               = NULL;
    obj->dbl_background     = FL_COL1;
	obj->parent             = NULL;
	obj->child              = NULL;
	obj->nc                 = NULL;
	obj->is_child           = 0;
	obj->group_id           = 0;

    return obj;
}


/***************************************
 * Adds an object to the form.
***************************************/

void
fl_add_object( FL_FORM   * form,
			   FL_OBJECT * obj )
{
	FL_OBJECT *o;

    /* Checking for correct behaviour. */

    if ( ! obj )
    {
		M_err( "fl_add_object", "NULL object." );
		return;
    }

    if ( ! form )
    {
		M_err( "fl_add_object", "NULL form for %s",
			   fli_object_class_name( obj ) );
		return;
    }

	if ( obj->form )
	{
		M_err( "fl_add_object", "Object already belongs to a form" );
		return;
	}

	if ( obj->objclass == FL_BEGIN_GROUP || obj->objclass == FL_END_GROUP )
	{
		M_err( "fl_add_object", "Can't add an pseudo-object that marks the "
			   "start or end of a group" );
		return;
	}

    if ( obj->automatic )
	{
		form->has_auto_objects++;
		fli_recount_auto_objects( );
	}

    obj->prev = obj->next = NULL;
    obj->form = form;

	obj->fl1 = obj->x;
	obj->fr1 = form->w_hr - obj->fl1;
	obj->ft1 = obj->y;
	obj->fb1 = form->h_hr - obj->ft1;

	obj->fl2 = obj->x + obj->w;
	obj->fr2 = form->w - obj->fl2;
	obj->ft2 = obj->y + obj->h;
	obj->fb2 = form->h - obj->ft2;

    if ( fli_inverted_y )
		obj->y = TRANY( obj, form );

    /* If adding to a group, set objects group ID, then find the end of the
	   group or the end of the object list on this form */

	if ( fli_current_group )
	{
		FL_OBJECT *end = fli_current_group;

		obj->group_id = fli_current_group->group_id;

		while ( end && end->objclass != FL_END_GROUP )
			end = end->next;

		/* If 'end' exists must've opened the group with fl_addto_group */

		if ( end )
		{
			end->prev->next = obj;
			obj->prev = end->prev;
			obj->next = end;
			end->prev = obj;
			fl_redraw_object( obj );
			return;
		}
	}

	if ( ! form->first )
		form->first = form->last = obj;
	else
	{
		obj->prev = form->last;
		form->last->next = obj;
		form->last = obj;
	}

    if ( obj->input && obj->active && ! form->focusobj )
		fl_set_focus_object( form, obj );

	/* If the object has child objects also add them to the form,
	   otherwise check if the object partialy or completely hiddes
	   other objects. */

    if ( obj->child )
		fli_add_composite( obj );
	else
		for ( o = form->first; o != obj; o = o->next )
		{
			if (    o->is_under
				 || o->is_child
				 || o->objclass == FL_BEGIN_GROUP
				 || o->objclass == FL_END_GROUP )
				continue;

			if ( objects_intersect( o, obj ) )
				o->is_under = 1;
		}

    fl_redraw_object( obj );
}


/***************************************
 * Inserts object 'obj' in front of the object 'before'.
 ***************************************/

void
fli_insert_object( FL_OBJECT * obj,
				   FL_OBJECT * before )
{
    FL_FORM *form;

    /* Checking for correct behaviour. */

    if ( ! obj || ! before )
    {
		M_err( "fli_insert_object", "NULL object." );
		return;
    }

    if ( ! before->form  )
    {
		M_err( "fli_insert_object", "Trying to insert object into NULL "
				  "form." );
		return;
    }

    form = before->form;
    obj->next = before;
    obj->group_id = before->group_id;

    if ( before == form->first )
    {
		form->first = obj;
		obj->prev = NULL;
    }
    else
    {
		obj->prev = before->prev;
		obj->prev->next = obj;
    }

    before->prev = obj;
    obj->form = form;

    if ( obj->input && obj->active && ! form->focusobj )
		fl_set_focus_object( form, obj );

	/* If the object has child objects also add them to the form */

    if ( obj->child )
		fli_add_composite( obj );

    fl_redraw_form( form );
}


/***************************************
 * Unlinks an object from its form
 ***************************************/

void
fl_delete_object( FL_OBJECT * obj )
{
    FL_FORM *form;

    if ( ! obj )
    {
		M_err( "fl_delete_object", "NULL object." );
		return;
    }

    if ( ! obj->form )
    {
		M_err( "fl_delete_object", "Delete %s from NULL form.",
			   ( obj->label && *obj->label ) ? obj->label : "object" );
		return;
    }

	checked_hide_tooltip( obj, NULL );

	/* If object is the pseudo-object starting a group delete the
	   complete group */

	if ( obj->objclass == FL_BEGIN_GROUP )
	{
		FL_OBJECT *o;

		for ( o = obj->next; o != NULL; o = o->next )
		{
			fl_delete_object( o );
			if ( o->objclass == FL_END_GROUP )
				break;
		}
	}

	/* Avoid deleting an object that represents the end of a group if
	   the group isn't empty */

	if ( obj->objclass == FL_END_GROUP )
	{
		FL_OBJECT *o;

		for ( o = obj->form->first; o != NULL && o != obj; o = o->next )
			if ( o->group_id == obj->group_id && o->objclass != FL_BEGIN_GROUP )
				break;

		if ( o != obj )
		{
			M_err( "fl_delete_object", "Can't delete end of group object "
				   "while the group still has members" );
			return;
		}
	}

    /* If this object has childs also unlink them */

    if ( obj->child )
		fli_delete_composite( obj );

    form = obj->form;

    if ( obj->automatic )
	{
		form->has_auto_objects--;
		fli_recount_auto_objects( );
	}

	lose_focus( obj ); 
    if ( obj == fli_pushobj )
		fli_pushobj = NULL;
    if ( obj == fli_mouseobj )
		fli_mouseobj = NULL;

#ifdef DELAYED_ACTION
    fli_object_qflush_object( obj );
#endif

	/* Object also loses its group membership */

    if ( obj->objclass != FL_BEGIN_GROUP && obj->objclass != FL_END_GROUP )
		obj->group_id = 0;

    obj->form = NULL;

    if ( obj->prev )
		obj->prev->next = obj->next;
    else
		form->first = obj->next;

    if ( obj->next )
		obj->next->prev = obj->prev;
    else
		form->last = obj->prev;

	/* Unless the whole form is getting free'd recalculate the
	   intersections of the other objects */

	if ( ! fli_fast_free_object )
		fli_recalc_intersections( form );

    if (    obj->visible
		 && ( ! obj->is_child || obj->parent->visible )
		 && form && form->visible == FL_VISIBLE )
		fl_redraw_form( form );
}


/***************************************
 * Frees the memory used by an object
 ***************************************/

void
fl_free_object( FL_OBJECT * obj )
{
    /* Check whether it's ok to free it */

    if ( ! obj )
    {
		M_err( "fl_free_object", "NULL object." );
		return;
    }

	/* If the object is the pseudo-object starting a group free the
	   complete group */

	if ( obj->objclass == FL_BEGIN_GROUP )
	{
		FL_OBJECT *o,
			      *on;

		for ( o = obj->next; o && o->objclass != FL_END_GROUP; o = on )
		{
			on = o->next;
			fl_free_object( o );
		}

		if ( o )
			fl_free_object( o );
	}

	/* Avoid deleting an object that represents the end of a group if
	   the group isn't empty */

	if ( obj->objclass == FL_END_GROUP )
	{
		FL_OBJECT *o;

		for ( o = obj->form->first; o != NULL && o != obj; o = o->next )
			if ( o->group_id == obj->group_id && o->objclass != FL_BEGIN_GROUP )
				break;

		if ( o != obj )
		{
			M_err( "fl_free_object", "Can't free end of group object "
				   "while the group still has members" );
			return;
		}
	}

	/* If the object hasn't yet been unlinked from its form do it know */

    if ( obj->form )
		fl_delete_object( obj );

	/* Make the object release memory it may have allocated */

    fli_handle_object( obj, FL_FREEMEM, 0, 0, 0, NULL );

	/* If this is a parent object free the children first */

	if ( obj->child )
		fli_free_composite( obj );

	/* If it's a child object remove it from the linked list of childs
	   of the parent object */

	if ( obj->is_child )
	{
		FL_OBJECT *o = obj->parent->child;

		if ( o == obj )
			obj->parent->child = obj->nc;
		else
		{
			while ( o->nc != obj )
				o = o->nc;
			o->nc = obj->nc;
		}
	}

    /* Finally free all memory allocated for the object */

	fl_safe_free( obj->label );
	fl_safe_free( obj->tooltip );
	fl_safe_free( obj->shortcut );

    if ( obj->flpixmap )
    {
		fli_free_flpixmap( obj->flpixmap ) ;
		fl_safe_free( obj->flpixmap );
    }

    fl_free( obj );
}


/*-----------------------------------------------------------------------
   Setting Attributes.
-----------------------------------------------------------------------*/

/***************************************
 * Sets the boxtype of the object
 ***************************************/

void
fl_set_object_boxtype( FL_OBJECT * obj,
					   int         boxtype )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_boxtype", "NULL object." );
		return;
    }

    if ( obj->boxtype != boxtype )
    {
		obj->boxtype = boxtype;
        if ( obj->child )
            fli_handle_object( obj, FL_ATTRIB, 0, 0, 0, NULL );
		fl_redraw_object( obj );
    }
}


/***************************************
 * Sets the resize property of an object
 ***************************************/

void
fl_set_object_resize( FL_OBJECT    * obj,
					  unsigned int   what )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_resize", "NULL object." );
		return;
    }

    obj->resize = what;

	/* Check if object has childs, if so also change all of them */

    if ( obj->child )
		fli_set_composite_resize( obj, what );

    /* Check if object is a group, if so also change all members */

    if ( obj->objclass == FL_BEGIN_GROUP )
		for ( ; obj && obj->objclass != FL_END_GROUP; obj = obj->next )
		{
			obj->resize = what;
			fli_set_composite_resize( obj, what );
		}
}


/***************************************
 * Returns the resize setting of an object
 ***************************************/

void
fl_get_object_resize( FL_OBJECT *    obj,
					  unsigned int * resize )
{
    if ( ! obj )
    {
		M_err( "fl_get_object_resize", "NULL object." );
		return;
    }

	if ( resize )
		*resize = obj->resize;
}


/***************************************
 * Sets the gravity properties of an object
 ***************************************/

void
fl_set_object_gravity( FL_OBJECT    * obj,
					   unsigned int   nw,
					   unsigned int   se )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_gravity", "NULL object." );
		return;
    }

    obj->nwgravity = nw;
    obj->segravity = se;

	/* Check if object has childs, if so also change all of them */

    if ( obj->child )
		fli_set_composite_gravity( obj, nw, se );

    /* Check if object is a group, if so change also all members */

    if ( obj->objclass == FL_BEGIN_GROUP )
		for ( ; obj && obj->objclass != FL_END_GROUP; obj = obj->next )
		{
			obj->nwgravity = nw;
			obj->segravity = se;
			fli_set_composite_gravity( obj, nw, se );
		}
}


/***************************************
 * Returns the gravity settings for an object
 ***************************************/
void
fl_get_object_gravity( FL_OBJECT *    obj,
					   unsigned int * nw,
					   unsigned int * se )
{
    if ( ! obj )
    {
		M_err( "fl_get_object_gravity", "NULL object." );
		return;
    }

	if ( nw )
		*nw = obj->nwgravity;
	if ( se )
		*se = obj->segravity;
}


/***************************************
 * Sets the color of the object
 ***************************************/

void
fl_set_object_color( FL_OBJECT * obj,
					 FL_COLOR    col1,
					 FL_COLOR    col2 )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_color", "NULL object." );
		return;
    }

	if ( col1 >= FL_MAX_COLORS || col2 >= FL_MAX_COLORS )
	{
		M_err( "fl_set_object_color", "Invalid color" );
		return;
	}

    if ( obj->col1 != col1 || obj->col2 != col2 )
    {
		obj->col1 = col1;
		obj->col2 = col2;
		fl_redraw_object( obj );
    }
}


/***************************************
 ***************************************/

void
fl_set_object_dblbuffer( FL_OBJECT * obj,
						 int         y )
{
    FL_COLOR bkcol;

    if ( ! obj )
    {
		M_err( "fl_set_object_dblbuffer", "NULL object." );
		return;
    }

    /* never bother with composite object */

    if ( obj->child || obj->is_child )
		return;

    if ( obj->use_pixmap == y )
		return;

    if ( ( obj->use_pixmap = y ) && ! obj->flpixmap )
		obj->flpixmap = fl_calloc( 1, sizeof( FL_pixmap ) );

    /* figure out the double buffer background */

    if ( obj->form && obj->form->first )
    {
		bkcol = obj->form->first->col1;
		if ( obj->form->first->boxtype == FL_NO_BOX && obj->form->first->next )
			bkcol = obj->form->first->next->col1;
		obj->dbl_background = bkcol;
    }
}


/* Test if an object is really visible */

#define ObjIsVisible( obj )  (    ( obj )->visible                        \
                               && ( obj )->form                           \
							   && ( obj )->form->visible == FL_VISIBLE )


/***************************************
 * Returns a pointer to the label of an object
 ***************************************/

const char *
fl_get_object_label( FL_OBJECT * obj )
{
    if ( ! obj )
    {
		M_err( "fl_get_object_label", "NULL object." );
		return NULL;
    }

    return obj->label;
}


/***************************************
 * Sets the label of an object
 ***************************************/

void
fl_set_object_label( FL_OBJECT  * obj,
					 const char * label )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_label", "NULL object." );
		return;
    }

    if ( ! label )
		label = "";

    if ( ! strcmp( obj->label, label )  )
		return;

    if ( LInside( obj->align ) )
    {
		obj->label = fl_realloc( obj->label, strlen( label ) + 1 );
		strcpy( obj->label, label );
		fl_redraw_object( obj );
    }
    else
    {
		int visible = ObjIsVisible( obj );

		if ( visible )
			fl_hide_object( obj );

		obj->label = fl_realloc( obj->label, strlen( label ) + 1 );
		strcpy( obj->label, label );

		if ( visible )
			fl_show_object( obj );
    }
}


/***************************************
 * Sets the label color of an object
 ***************************************/

void
fl_set_object_lcol( FL_OBJECT * obj,
					FL_COLOR    lcol )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_lcol", "NULL object." );
		return;
    }

    if ( obj->objclass == FL_BEGIN_GROUP )
    {
		fl_freeze_form( obj->form );

		for ( ; obj && obj->objclass != FL_END_GROUP; obj = obj->next )
		{
			if ( obj->lcol != lcol )
			{
				obj->lcol = lcol;
				if ( obj->objclass != FL_BEGIN_GROUP )
					fl_redraw_object( obj );
			}
		}

		fl_unfreeze_form( obj->form );
    }
    else if ( obj->lcol != lcol )
    {
		obj->lcol = lcol;
		fl_redraw_object( obj );
    }
}


/***************************************
 * Sets the label size of an object
 ***************************************/

void
fl_set_object_lsize( FL_OBJECT * obj,
					 int         lsize )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_lsize", "NULL object." );
		return;
    }

    /* no nested groups */

    if ( obj->objclass == FL_BEGIN_GROUP )
    {
		obj->lsize = lsize;
		for ( obj = obj->next; obj && obj->objclass != FL_END_GROUP;
			  obj = obj->next )
			fl_set_object_lsize( obj, lsize );
    }
    else if ( obj->lsize != lsize )
    {
		if ( LInside( obj->align ) )
		{
			obj->lsize = lsize;
			fl_redraw_object( obj );
		}
		else
		{
			int visible = ObjIsVisible( obj );

			if ( visible )
				fl_hide_object( obj );

			obj->lsize = lsize;

			if ( visible )
				fl_show_object( obj );
		}
    }
}


/***************************************
 * Sets the label style of an object
 ***************************************/

void
fl_set_object_lstyle( FL_OBJECT * obj,
					  int         lstyle )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_lstyle", "NULL object." );
		return;
    }

    if ( obj->objclass == FL_BEGIN_GROUP )
    {
		obj->lstyle = lstyle;
		for ( obj = obj->next; obj && obj->objclass != FL_END_GROUP;
			  obj = obj->next )
			fl_set_object_lstyle( obj, lstyle );
    }
    else if ( obj->lstyle != lstyle )
    {
		if ( LInside( obj->align ) )
		{
			obj->lstyle = lstyle;
			fl_redraw_object( obj );
		}
		else
		{
			int visible = ObjIsVisible( obj );

			if ( visible )
				fl_hide_object( obj );

			obj->lstyle = lstyle;

			if ( visible )
				fl_show_object( obj );
		}
    }
}


/***************************************
 * Sets the label alignment of an object
 ***************************************/

void
fl_set_object_lalign( FL_OBJECT * obj,
					  int         align )
{
    int visible;
	int need_overlap_check = ( obj->align ^ align ) & FL_ALIGN_INSIDE;

    if ( ! obj )
    {
		M_err( "fl_set_object_lalign", "NULL object." );
		return;
    }

    if ( obj->align == align )
		return;

    visible = ObjIsVisible( obj );

    if ( LInside( obj->align ) && LInside( align ) )
    {
		obj->align = align;
		fl_redraw_object( obj );
    }
    else
    {
		if ( visible )
			fl_hide_object( obj );

		obj->align = align;

		if ( need_overlap_check )
			fli_recalc_intersections( obj->form );

		if ( visible )
			fl_show_object( obj );
    }
}


/***************************************
 * Makes an object active
 ***************************************/

static void
activate_object( FL_OBJECT * obj )
{
	if ( obj->active )
		return;

	obj->active = 1;
	if ( obj->input && obj->active && ! obj->form->focusobj )
		fl_set_focus_object( obj->form, obj );
	if ( obj->child )
		fli_activate_composite( obj );
}


/***************************************
 * Public function for making an object active
 ***************************************/

void
fl_activate_object( FL_OBJECT * obj )
{
    if ( ! obj )
    {
		M_err( "fl_activate_object", "NULL object." );
		return;
    }

    if ( obj->objclass == FL_BEGIN_GROUP )
		for ( ; obj && obj->objclass != FL_END_GROUP; obj = obj->next )
			activate_object( obj );
    else
		activate_object( obj );
}


/***************************************
 * Deactivates an object
 ***************************************/

static void
deactivate_object( FL_OBJECT * obj )
{
	if ( ! obj->active )
		return;

	obj->active = 0;
	lose_focus( obj );

	if ( obj->child )
		fli_deactivate_composite( obj );
}


/***************************************
 * Public function for deactivating an object
 ***************************************/

void
fl_deactivate_object( FL_OBJECT * obj )
{
    if ( ! obj )
    {
		M_err( "fl_deactive_object", "NULL object." );
		return;
    }

    if ( obj->objclass == FL_BEGIN_GROUP )
		for ( ; obj && obj->objclass != FL_END_GROUP; obj = obj->next )
			deactivate_object( obj );
    else
		deactivate_object( obj );
}


/***************************************
 * Makes an object visible and sets the visible flag to 1
 ***************************************/

void
fli_show_object( FL_OBJECT * obj )
{
	if ( obj->visible )
		return;

	obj->visible = 1;

	if ( obj->child )
		fli_show_composite( obj );

	if ( obj->input && obj->active && ! obj->form->focusobj )
		fl_set_focus_object( obj->form, obj );
}


/***************************************
 * Public function for making an object visible
 ***************************************/

void
fl_show_object( FL_OBJECT * obj )
{
    FL_OBJECT *o = obj;

    if ( ! obj )
    {
		M_err( "fl_show_object", "NULL object." );
		return;
    }

    if ( obj->objclass == FL_BEGIN_GROUP )
		for ( ; obj && obj->objclass != FL_END_GROUP; obj = obj->next )
			fli_show_object( obj );
    else
		fli_show_object( obj );

    fl_redraw_object( o );
}


/***************************************
 * Sets an object up for being hidden and
 * adds the area it covers to a region
 ***************************************/

void
fli_hide_and_get_region( FL_OBJECT * obj,
						 Region    * reg )
{
    FL_RECT xrect;	
	int extra;

#ifdef DELAYED_ACTION
	/* Remove all entries for the object from the object queue */

	fli_object_qflush_object( obj );
#endif

	/* The object can't be the object anymore that has the focus or be
	   the pushed object or the object the mouse is on */

	lose_focus( obj );
	if ( obj == fli_pushobj )
		fli_pushobj = NULL;
	if ( obj == fli_mouseobj )
		fli_mouseobj = NULL;

	/* Get the area the object covers and add that to the region passed 
	   to the function */

	if ( obj->objclass == FL_CANVAS || obj->objclass == FL_GLCANVAS )
	{
		fl_hide_canvas( obj );
		extra = 3;
		xrect.x = obj->x - extra;
		xrect.y = obj->y - extra;
		xrect.width = obj->w + 2 * extra + 1;
		xrect.height = obj->h + 2 * extra + 1;
	}
	else
	{
		get_object_bbox_rect( obj, &xrect );
	
		if ( obj->objclass == FL_FRAME )
		{
			extra = FL_abs( obj->bw );
			xrect.x -= extra;
			xrect.y -= extra;
			xrect.width  += 2 * extra + 1;
			xrect.height += 2 * extra + 1;
		}
	}

	XUnionRectWithRegion( &xrect, *reg, *reg );

	/* Mark it as invisible (must be last, fl_hide_canvas() tests for
	   visibility and doesn't do anything if already marked as invisible) */

	obj->visible = 0;
}


/***************************************
 * Makes an object (and all its children) invisible
 ***************************************/

void fl_hide_object( FL_OBJECT * obj )
{
	FL_OBJECT *tmp;
    FL_RECT xrect;
    Region reg;

    if ( ! obj )
    {
		M_err( "fl_hide_object", "NULL object." );
		return;
    }

    if ( ! obj->visible )
    {
		M_warn( "fl_hide_object", "%s already invisible",
				obj->label ? obj->label : "Object" );
		return;
    }

	reg = XCreateRegion( );

	/* If this is an object that marks the start of a group hide all
	   objects that belong to the group */

    if ( obj->objclass == FL_BEGIN_GROUP )
		for ( tmp = obj->next; tmp && tmp->objclass != FL_END_GROUP;
			  tmp = tmp->next )
		{
			if ( tmp->child )
				fli_hide_composite( tmp, &reg );

			fli_hide_and_get_region( tmp, &reg );
		}
	else
	{
		if ( obj->child )
			fli_hide_composite( obj, &reg );

		fli_hide_and_get_region( obj, &reg );
	}

	/* Determine the rectangle that covers the area of the object */

	XClipBox( reg, &xrect );
	XDestroyRegion( reg );

	/* No redraw needed if the form isn't shown */

	if ( obj->form->visible != FL_VISIBLE )
		return;

	/* Redraw only the area covered by the object (take care with the
	   order the different clipping types are set and unset, changing
	   them has strange effects) */

	fli_set_perm_clipping( xrect.x, xrect.y, xrect.width, xrect.height );
	fl_set_clipping( xrect.x, xrect.y, xrect.width, xrect.height );
	fl_set_text_clipping( xrect.x, xrect.y, xrect.width, xrect.height );

    fl_redraw_form( obj->form );

	fli_unset_perm_clipping( );
	fl_unset_clipping( );
	fl_unset_text_clipping( );
}


/***************************************
 * Sets the list of shortcuts for the object. Shortcuts are specified
 * with a string with the following special sequences:
 * '^x'  stands for  Ctrl-x (for a-z case doesn't matter)
 * '#x'  stands for  Alt-x (case matters!)
 * '&n'  with n = 1,...,34 stands for function key n
 * '&A', '&B', '&C' '&D'  stand for up down, right and left cursor keys
 * '^[' stand for escape key
 * '^^  stand for '^'
 * '^#' stand for '#'
 * '^&' stand for '&'
 * Note: '&' followed by anything else than the above will be skipped,
 * e.g. '&E' or '&0'. If '&' is followed by a number larger than 34
 * only the first digit of the number is used.
 * Not escapable are Crtl-^, Crtl-# and Ctrl-&.
 ***************************************/

#include <ctype.h>

int
fli_convert_shortcut( const char * str,
					  long         sc[ ] )
{
    int i = 0;
	long offset = 0;
	const char *c;

	for ( c = str; *c && i < MAX_SHORTCUTS; c++ )
	{
		switch ( *c )
		{
			case '^' :
				if ( offset & FL_CONTROL_MASK && c[ -1 ] == '^' )
				{
					sc[ i++ ] = '^' + offset - FL_CONTROL_MASK;
					offset = 0;
				}
				else
				{
					if ( c[ 1 ] == '[' )
					{
						sc[ i++ ] = 0x1b + offset - FL_CONTROL_MASK;
						c++;
						offset = 0;
					}
					else
						offset += FL_CONTROL_MASK;
				}
				break;

			case '#' :
				if ( offset & FL_CONTROL_MASK && c[ -1 ] == '^' )
				{
					sc[ i++ ] = '#' + offset - FL_CONTROL_MASK;
					offset = 0;
				}
				else
					offset += FL_ALT_MASK;
				break;

			case '&' :
				if ( offset & FL_CONTROL_MASK && c[ -1 ] == '^' )
				{
					sc[ i++ ] = '&' + offset - FL_CONTROL_MASK;
					offset = 0;
					break;
				}
				else if ( c[ 1 ] == 'A' )
					sc[ i++ ] = XK_Up + offset;
				else if ( c[ 1 ] == 'B' )
					sc[ i++ ] = XK_Down + offset;
				else if ( c[ 1 ] == 'C' )
					sc[ i++ ] = XK_Right + offset;
				else if ( c[ 1 ] == 'D' )
					sc[ i++ ] = XK_Left + offset;
				else if ( isdigit( ( int ) c[ 1 ] ) && c[ 1 ] > '0' )
				{
					long j = c[ 1 ]  - '0';

					if (    isdigit( ( int ) c[ 2 ] )
						 && 10 * j + c[ 2 ] - '0' <= 35 )
					{
						 j = 10 * c[ 2 ] - '0';
						 c++;
					}
					sc[ i++ ] = offset + XK_F1 + j - 1;
				}
				offset = 0;
				c++;
				break;

			default :
				if (    offset & ( FL_CONTROL_MASK | FL_ALT_MASK )
					 && *c >= 'a' && *c <= 'z' )
					sc[ i++ ] = toupper( ( int ) *c ) + offset;
				else
					sc[ i++ ] = *c + offset;
				offset = 0;
				break;
		}
	}

	sc[ i ] = 0;

	if ( *c != '\0' )
    {
		M_err( "fli_convert_shortcut", "Too many shortcuts (>%d)",
			   MAX_SHORTCUTS );
    }

    return i;
}


/***************************************
 ***************************************/

int
fli_get_underline_pos( const char * label,
					   const char * sc )
{
    int c;
    const char *p;

    /* Find the first non-special char in the shortcut string */

    for ( c = '\0', p = sc; ! c && *p; p++ )
    {
		if ( isalnum( ( int ) *p ) )
		{
			if ( p == sc )
				c = *p;
			else if ( * ( p - 1 ) != '&' && ! isdigit( ( int ) * ( p - 1 ) ) )
				c = *p;
		}
    }

    if ( ! c )
		return -1;

    /* Find where the match occurs */

    if ( c == sc[ 0 ] )
		p = strchr( label, c );
    else if ( ! ( p = strchr( label, c ) ) )
		p = strchr( label, islower( c ) ? toupper( c ) : tolower( c ) );

    if ( ! p )
		return -1;

    return p - label + 1;
}


/***************************************
 ***************************************/

void
fl_set_object_shortcut( FL_OBJECT  * obj,
						const char * sstr,
						int          showit )
{
    int scsize,
		n;
    long sc[ MAX_SHORTCUTS + 1 ];	   /* converted shortcuts - we need one
										  more than max for trailing 0 */

    if ( ! obj )
    {
		M_err( "fl_set_object_shortcut", "NULL object." );
		return;
    }

    if ( ! sstr || ! *sstr )
    {
		obj->shortcut[ 0 ] = 0;
		return;
    }

    n = fli_convert_shortcut( sstr, sc );
    scsize = ( n + 1 ) * sizeof *obj->shortcut;
    obj->shortcut = fl_realloc( obj->shortcut, scsize );
    memcpy( obj->shortcut, sc, scsize );

    if (    ! showit
		 || ! obj->label
		 || ! obj->label[ 0 ]
		 || obj->label[ 0 ] == '@' )
		return;

    /* Find out where to underline */

    if (    ( n = fli_get_underline_pos( obj->label, sstr ) ) > 0
		 && ! strchr( obj->label, *fl_ul_magic_char ) )
    {
		size_t len = strlen( obj->label ) + 1;

		obj->label = fl_realloc( obj->label, len + 1 );
		memmove( obj->label + n + 1, obj->label + n, len - n );
		obj->label[ n ] = *fl_ul_magic_char;
    }
}


/***************************************
 * Set a shortcut with keysyms directly
 ***************************************/

void
fl_set_object_shortcutkey( FL_OBJECT    * obj,
						   unsigned int   keysym )
{
    size_t n;

    for ( n = 0; obj->shortcut[ n ]; n++ )
		/* empty */;

    /* always have a terminator, thus n + 2 */

    obj->shortcut = fl_realloc( obj->shortcut,
								( n + 2 ) * sizeof *obj->shortcut );
    obj->shortcut[ n ] = keysym;
    obj->shortcut[ n + 1 ] = 0;
}


/***************************************
 * Sets the object in the form that gets keyboard input.
 ***************************************/

void
fl_set_focus_object( FL_FORM   * form,
					 FL_OBJECT * obj )
{
    if ( ! form )
    {
		M_err( "fl_set_focus_object", "NULL form." );
		return;
    }

    if ( obj == form->focusobj )
		return;

	if ( form->focusobj )
		fli_handle_object_direct( form->focusobj, FL_UNFOCUS, 0, 0, 0, NULL );
    fli_handle_object_direct( obj, FL_FOCUS, 0, 0, 0, NULL );
}


/***************************************
 * Returns the object that has the focus
 ***************************************/

FL_OBJECT *
fl_get_focus_object( FL_FORM * form )
{
    if ( form && form->focusobj )
    {
		if ( form->focusobj->type == FL_MULTILINE_INPUT )
			return form->focusobj->parent;
		else
			return form->focusobj;
    }

    return NULL;
}


/*-----------------------------------------------------------------------
   Searching in forms
-----------------------------------------------------------------------*/

/***************************************
 * Returns an object of type 'find' in a form , starting at 'obj'.
 * If find_object() does not return an object the event that
 * triggered the call will be eaten. This is how the deactived
 * and inactive objects reject events.
 * Modify with care!
 ***************************************/

FL_OBJECT *
fli_find_object( FL_OBJECT * obj,
				 int         find,
				 FL_Coord    mx,
				 FL_Coord    my )
{
    while ( obj )
	{
		if (    obj->objclass != FL_BEGIN_GROUP
			 && obj->objclass != FL_END_GROUP
			 && obj->visible
			 && ( ! obj->is_child || obj->parent->visible )
			 && (     obj->active
				  || ( obj->posthandle && ! obj->active )
				  || ( obj->tooltip && *obj->tooltip && ! obj->active ) ) )
		{
			if ( find == FL_FIND_INPUT && obj->input && obj->active )
				return obj;

			if ( find == FL_FIND_AUTOMATIC && obj->automatic )
				return obj;

			if (    find == FL_FIND_MOUSE
				 && mx >= obj->x
				 && mx <= obj->x + obj->w
				 && my >= obj->y
				 && my <= obj->y + obj->h )
				return obj;

			if ( find == FL_FIND_KEYSPECIAL && obj->wantkey & FL_KEY_SPECIAL )
				return obj;
		}

		obj = obj->next;
	}

    return NULL;
}


/***************************************
 * Same as above but going backwards through the linked list of objects.
 ***************************************/

FL_OBJECT *
fli_find_object_backwards( FL_OBJECT * obj,
						   int         find,
						   FL_Coord    mx,
						   FL_Coord    my )
{
    for ( ; obj; obj = obj->prev )
		if (    obj->objclass != FL_BEGIN_GROUP
			 && obj->objclass != FL_END_GROUP
			 && obj->visible
			 && ( ! obj->is_child || obj->parent->visible )
			 && (     obj->active
				  || ( obj->posthandle && ! obj->active )
				  || ( obj->tooltip && *obj->tooltip && ! obj->active ) ) )
		{
			if ( find == FL_FIND_INPUT && obj->input && obj->active )
				return obj;

			if ( find == FL_FIND_AUTOMATIC && obj->automatic )
				return obj;

			if (    find == FL_FIND_MOUSE
				 && mx >= obj->x
				 && mx <= obj->x + obj->w
				 && my >= obj->y
				 && my <= obj->y + obj->h )
				return obj;

			if ( find == FL_FIND_KEYSPECIAL && obj->wantkey & FL_KEY_SPECIAL )
				return obj;
		}

    return NULL;
}


/***************************************
 * Returns the first object of type find
 ***************************************/

FL_OBJECT *
fli_find_first( FL_FORM  * form,
				int        find,
				FL_Coord   mx,
				FL_Coord   my )
{
    return fli_find_object( form->first, find, mx, my );
}


/***************************************
 * Returns the last object of the type find
 ***************************************/

FL_OBJECT *
fli_find_last( FL_FORM * form,
			   int       find,
			   FL_Coord  mx,
			   FL_Coord  my )
{
    FL_OBJECT *last,
		      *obj;

    last = obj = fli_find_first( form, find, mx, my );

    while ( obj )
    {
		last = obj;
		obj = fli_find_object( obj->next, find, mx, my );
    }

    return last;
}


/*-----------------------------------------------------------------------
   Drawing Routines.
-----------------------------------------------------------------------*/


/***************************************
 ***************************************/

static int
object_is_clipped( FL_OBJECT * obj )
{
    FL_RECT xr,
		    *xc;
    int extra = 1;

    get_object_bbox_rect( obj, &xr );

    xr.x      -= extra;
    xr.y      -= extra;
    xr.width  += 2 * extra;
    xr.height += 2 * extra;

	xc = fli_union_rect( &xr, &fli_perm_xcr );

	if ( ! xc )
		return 1;

	fl_free( xc );
	return 0;
}


/***************************************
 * Redraws all marked objects and reduces the mark. It is important
 * NOT to set any clip masks inside this function (except for free objects)
 * in that it will prevent the drawing function from drawing labels. That
 * would be wrong since fl_redraw_object() calls redraw_marked
 * directly. All clip must done prior to calling this routines
 ***************************************/

static void
redraw_marked( FL_FORM * form,
			   int       key,
			   XEvent  * xev )
{
    FL_OBJECT *obj,
		      *o;

    if ( form->visible != FL_VISIBLE || form->frozen > 0 )
		return;

    fli_set_form_window( form );
    fli_create_form_pixmap( form );

	/* Check if there are any objects that partially or fully hide one of
	   the objects to be redrawn and mark those also for redrawing (and, of
	   course, also those that are "above" this newly added object etc.) */

	for ( obj = form->first; obj; obj = obj->next )
		if (    obj->visible
			 && obj->redraw
             && obj->is_under
			 && obj->objclass != FL_BEGIN_GROUP
			 && obj->objclass != FL_END_GROUP
			 && !obj->is_child )
			break;

	if ( obj && obj->next )
	{
		for ( ; obj && obj->next; obj = obj->next )
		{
			if (    ! obj->visible
				 || ! obj->redraw
				 || ! obj->is_under
				 || obj->objclass == FL_BEGIN_GROUP
				 || obj->objclass == FL_END_GROUP
				 || obj->is_child )
				continue;

			for ( o = obj->next; o; o = o->next )
			{
				if (    ! o->visible
					 || o->redraw
					 || o->objclass == FL_BEGIN_GROUP
					 || o->objclass == FL_END_GROUP
					 || o->is_child )
					continue;
				 
				if ( objects_intersect( obj, o ) )
				{
					o->redraw = 1;
					if ( o->child )
						fli_mark_composite_for_redraw( o );
				}
			}
		}
	}

	/* Now redraw all marked objects */

    for ( obj = form->first; obj; obj = obj->next )
    {
		if (    obj->visible
			 && obj->redraw
			 && ( ! obj->is_child || obj->parent->visible ) )
		{
			obj->redraw = 0;

			/* no point redrawing unexposed object */

			if ( fli_perm_clip && object_is_clipped( obj ) )
			{
#if FL_DEBUG >= ML_WARN
				M_warn( "redraw_marked", "%s is clipped", obj->label );
#endif
				continue;
			}

			fli_create_object_pixmap( obj );

			/* Will not allow free object draw outside of its box. Check
			   perm_clip so we don't have draw regions we don't have to
			   (Expose etc.) */

			if ( ( obj->objclass == FL_FREE || obj->clip ) && ! fli_perm_clip )
			{
				fl_set_clipping( obj->x, obj->y, obj->w, obj->h );
				fl_set_text_clipping( obj->x, obj->y, obj->w, obj->h );
			}

			fli_handle_object( obj, FL_DRAW, 0, 0, key, xev );

			if ( ( obj->objclass == FL_FREE || obj->clip ) && ! fli_perm_clip )
			{
				fl_unset_clipping( );
				fl_unset_text_clipping( );
			}

			fli_show_object_pixmap( obj );
		}
    }

    fli_show_form_pixmap( form );
}


/***************************************
 * The actual drawing routine seen by the user
 ***************************************/

void
fl_redraw_object( FL_OBJECT * obj )
{
    if ( ! obj )
    {
		M_err( "fl_redraw_object", "NULL object." );
		return;
    }

    if ( ! obj->form )
		return;

    if ( obj->objclass == FL_BEGIN_GROUP )
    {
		FL_OBJECT *ob = obj->next;

		for ( ; ob && ob->objclass != FL_END_GROUP; ob = ob->next )
			ob->redraw = 1;
    }
    else
		obj->redraw = 1;

    /* if composite object, flag all children */

    if (    obj->child
		 && ( ! obj->is_child || obj->parent->visible )
		 && obj->visible )
		fli_mark_composite_for_redraw( obj );

    /* If obj is a child object and the parent is not visible, do nothing */

    if ( obj->visible && ( ! obj->is_child || obj->parent->visible ) )
		redraw_marked( obj->form, 0, NULL );
}


/***************************************
 * Function to test if the areas of 'obj1' and 'obj2' intersect
 ***************************************/

static int
objects_intersect( FL_OBJECT * obj1,
				   FL_OBJECT * obj2 )
{
	FL_OBJECT *ob[ ] = { obj1, obj2 };
	int i;
    FL_RECT xrect;
    Region reg[ 2 ];
	int extra;

	for ( i = 0; i < 2; i++ )
	{
		reg[ i ] = XCreateRegion( );

		if (    ob[ i ]->objclass == FL_CANVAS
			 || ob[ i ]->objclass == FL_GLCANVAS )
		{
			extra = 3;
			xrect.x      = ob[ i ]->x - extra;
			xrect.y      = ob[ i ]->y - extra;
			xrect.width  = ob[ i ]->w + 2 * extra + 1;
			xrect.height = ob[ i ]->h + 2 * extra + 1;
		}
		else
		{
			get_object_bbox_rect( ob[ i ], &xrect );
	
			if ( ob[ i ]->objclass == FL_FRAME )
			{
				extra = FL_abs( ob[ i ]->bw );
				xrect.x      -= extra;
				xrect.y      -= extra;
				xrect.width  += 2 * extra + 1;
				xrect.height += 2 * extra + 1;
			}
		}

		XUnionRectWithRegion( &xrect, reg[ i ], reg[ i ] );
	}

	XIntersectRegion( reg[ 0 ], reg[ 1 ], reg[ 0 ] );
	XClipBox( reg[ 0 ], &xrect );
	XDestroyRegion( reg[ 1 ] );
	XDestroyRegion( reg[ 0 ] );

	return xrect.width > 0 && xrect.height > 0;
}


/***************************************
 * Function to test if the areas of 'obj1' and 'obj2' intersect
 ***************************************/

static int
object_is_under( FL_OBJECT * obj )
{
	FL_OBJECT *o;

	if (    obj->is_child
		 || obj->objclass == FL_BEGIN_GROUP 
		 || obj->objclass == FL_END_GROUP )
		return 0;

	for ( o = obj->next; o; o = o->next )
	{
		if (    o->is_child
			 || o->objclass == FL_BEGIN_GROUP 
			 || o->objclass == FL_END_GROUP )
			continue;

		if ( objects_intersect( obj, o ) )
			return 1;
	}

	return 0;
}


/***************************************
 * Marks all objects for redraw
 ***************************************/

static void
mark_for_redraw( FL_FORM * form )
{
    FL_OBJECT *obj;

    if ( ! form )
    {
		M_err( "mark_for_redraw", "Drawing NULL form." );
		return;
    }

    for ( obj = form->first; obj; obj = obj->next )
	{
		if ( ! obj->visible || obj->is_child )
			continue;

		if ( obj->objclass != FL_BEGIN_GROUP && obj->objclass != FL_END_GROUP )
		{
			obj->redraw = 1;
			if ( obj->child )
				fli_mark_composite_for_redraw( obj );
		}
	}
}


/***************************************
 * Draws a form
 ***************************************/

void
fl_redraw_form( FL_FORM * form )
{
    mark_for_redraw( form );
    redraw_marked( form, 0, NULL );
}


/***************************************
 * Draws a form
 ***************************************/

void
fli_redraw_form_using_xevent( FL_FORM * form,
							  int       key,
							  XEvent  * xev )
{
    mark_for_redraw( form );
    redraw_marked( form, key, xev );
}


/***************************************
 * Disables drawing of form
 ***************************************/

void
fl_freeze_form( FL_FORM * form )
{
    if ( ! form )
    {
		M_err( "fl_freeze_form", "NULL form." );
		return;
    }

    form->frozen++;
}


/***************************************
 * Enables drawing of form
 ***************************************/

void
fl_unfreeze_form( FL_FORM * form )
{
    if ( ! form )
    {
		M_err( "fl_unfreeze_form", "NULL form." );
		return;
    }

    if ( form->frozen == 0 )
    {
		M_err( "fl_unfreeze_form", "Unfreezing non-frozen form." );
		return;
    }

    form->frozen--;

    if ( form->frozen == 0 && form->visible == FL_VISIBLE )
		redraw_marked( form, 0, NULL );
}

/*-----------------------------------------------------------------------
   Handling Routines.
-----------------------------------------------------------------------*/

/***************************************
 * should only be used as a response to FL_UNFOCOS
 ***************************************/

void
fl_reset_focus_object( FL_OBJECT * obj )
{
    refocus = obj;
}

/*** handle tooltips ***/


/***************************************
 ***************************************/

static
FL_OBJECT * get_parent( FL_OBJECT * obj )
{
    if ( obj )
		while ( obj->parent && obj->parent != obj )
			obj = obj->parent;

    return obj;
}


/***************************************
 ***************************************/

static
void tooltip_handler( int    ID  FL_UNUSED_ARG,
					  void * data )
{
    FL_OBJECT * const obj = get_parent( data );
    char const * const tooltip = obj->tooltip;

    if ( tooltip && *tooltip )
		fli_show_tooltip( tooltip, obj->form->x + obj->x,
						  obj->form->y + obj->y + obj->h + 1 );
    obj->tipID = 0;
}


/***************************************
 ***************************************/

static
void checked_hide_tooltip( FL_OBJECT * obj,
						   XEvent    * xev )
{
    FL_OBJECT * const parent = get_parent( obj );
    char const * const tooltip = parent->tooltip;

    if ( ! tooltip || ! *tooltip )
		return;

	/* If obj is part of a composite widget, it may well be that we're
	   leaving a child widget but are still within the parent.
	   If that is the case, we don't want to hide the tooltip at all. */

	if (    parent != obj
		 && xev
		 && xev->xmotion.x >= parent->x
		 && xev->xmotion.x <= parent->x + parent->w
		 && xev->xmotion.y >= parent->y
		 && xev->xmotion.y <= parent->y + parent->h )
		return;

	fli_hide_tooltip( );

	if ( parent->tipID )
	{
		fl_remove_timeout( parent->tipID );
		parent->tipID = 0;
	}
}


/***************************************
 ***************************************/

static
void unconditional_hide_tooltip( FL_OBJECT * obj )
{
    FL_OBJECT * const parent = get_parent( obj );

    fli_hide_tooltip( );
    if ( parent->tipID )
	{
		fl_remove_timeout( parent->tipID );
		parent->tipID = 0;
    }
}


/***************************************
 * Handles an event for an object
 ***************************************/

static int
fl_handle_it( FL_OBJECT * obj,
			  int         event,
			  FL_Coord    mx,
			  FL_Coord    my,
			  int         key,
			  XEvent    * xev )
{
    static unsigned long last_clicktime = 0;
    static int last_dblclick = 0,
		       last_key = 0;
    static FL_Coord last_mx,
		            last_my;
    int status = 0;
    int cur_event;

    if ( ! obj )
		return 0;

#if FL_DEBUG >= ML_WARN
    if ( ! obj->form && event != FL_FREEMEM )
    {
		M_err( "fl_handle_it", "Bad object %s. Event=%s",
			   obj->label ? obj->label : "", fli_event_name( event ) );
		return 0;
    }
#endif

    if ( obj->objclass == FL_BEGIN_GROUP || obj->objclass == FL_END_GROUP )
		return 0;

    if ( ! obj->handle )
		return 0;

    switch ( event )
    {
		case FL_ENTER:
		{
			/* We assign the timer to the parent widget in the case of a
			   composite object as that's the thing that's actually got the
			   tip. */

			FL_OBJECT * const parent = get_parent( obj );

			if ( ! parent->tipID )
			{
				char const * const tooltip = parent->tooltip;

				if ( tooltip && *tooltip && ! parent->form->no_tooltip )
					parent->tipID = fl_add_timeout( fli_context->tooltip_time,
													tooltip_handler, parent );
			}
			obj->belowmouse = 1;
			break;
		}

		case FL_LEAVE:
			checked_hide_tooltip( obj, xev );
			obj->belowmouse = 0;
			break;

		case FL_PUSH:
			unconditional_hide_tooltip( obj );
			obj->pushed = 1;
			break;

		case FL_KEYPRESS:
			unconditional_hide_tooltip( obj );
			break;

		case FL_RELEASE:
			if ( ! obj->radio )
				obj->pushed = 0;

			/* Changed: before double and triple clicks weren't accepted for
			   the middle mouse button (which didn't make too much sense IMHO),
			   now they don't get flagged for the mouse wheel "buttons". JTT */

			if (    key == last_key
				 && ! ( key == FL_MBUTTON4 || key == FL_MBUTTON5 )
				 && ! (    FL_abs( last_mx - mx ) > 4
						|| FL_abs( last_my - my ) > 4 )
				 && xev
				 && xev->xbutton.time - last_clicktime < obj->click_timeout )
				event = last_dblclick ? FL_TRPLCLICK : FL_DBLCLICK;

			last_dblclick = event == FL_DBLCLICK;
			last_clicktime = xev ? xev->xbutton.time : 0;
			last_key = key;
			last_mx = mx;
			last_my = my;
			break;

		case FL_FOCUS:
			/* 'refocus' is set if on the last FL_UNFOCUS it was found
			   that the text in the input field didn't validate. In that
			   case the focus has to go back to that field instead to a
			   different one */

			if ( refocus && refocus->form )
			{
				obj = refocus;
				refocus = NULL;
			}

			if ( obj->form )
			{
				obj->form->focusobj = obj;
				obj->focus = 1;
			}
			break;

		case FL_UNFOCUS:
			obj->form->focusobj = NULL;
			obj->focus = 0;
			break;

		case FL_DRAW:
			if ( obj->objclass == FL_FREE )
			{
				fl_set_clipping( obj->x, obj->y, obj->w, obj->h );
				fl_set_text_clipping( obj->x, obj->y, obj->w, obj->h );
			}
			break;
    }

    cur_event = event;
    if ( event == FL_DBLCLICK || event == FL_TRPLCLICK )
		event = FL_RELEASE;

 recover:
    if (    obj->prehandle
		 && event != FL_FREEMEM
		 && obj->prehandle( obj, event, mx, my, key, xev ) == FL_PREEMPT )
		return 0;

    status = obj->handle( obj, event, mx, my, key, xev );

    if ( obj->posthandle && event != FL_FREEMEM )
		obj->posthandle( obj, event, mx, my, key, xev );

    if ( cur_event == FL_DBLCLICK || cur_event == FL_TRPLCLICK )
    {
		event = cur_event;
		cur_event = 0;
		if ( status )
			fli_object_qenter( obj );
		goto recover;
    }

    if ( obj->objclass == FL_FREE && event == FL_DRAW )
    {
		fl_unset_clipping( );
		fl_unset_text_clipping( );
    }

    return ( event == FL_DBLCLICK || event == FL_TRPLCLICK ) ? 0 : status;
}


/***************************************
 * Handle and store object in object queue if handler returns non-zero
 ***************************************/

void
fli_handle_object( FL_OBJECT * obj,
				   int         event,
				   FL_Coord    mx,
				   FL_Coord    my,
				   int         key,
				   XEvent *    xev )
{
    if ( ! obj )
		return;

    if ( fl_handle_it( obj, event, mx, my, key, xev ) )
		fli_object_qenter( obj );
}


/***************************************
 * Handle but returns whether successful
 ***************************************/

int
fli_handle_object_direct( FL_OBJECT * obj,
						  int         event,
						  FL_Coord    mx,
						  FL_Coord    my,
						  int         key,
						  XEvent    * xev )
{
    return fl_handle_it( obj, event, mx, my, key, xev );
}


/***************************************
 * Sets the call_back routine for the object
 ***************************************/

FL_CALLBACKPTR
fl_set_object_callback( FL_OBJECT      * obj,
						FL_CALLBACKPTR   callback,
						long             argument )
{
    FL_CALLBACKPTR old;

    if ( ! obj )
    {
		M_err( "fl_set_object_callback", "NULL object." );
		return NULL;
    }

    old = obj->object_callback;
    obj->object_callback = callback;
    obj->argument = argument;

    return old;
}


/***************************************
 * Sets the borderwidth of an object
 ***************************************/

void
fl_set_object_bw( FL_OBJECT * obj,
				  int         bw )
{
	/* Clamp border width to a reasonable range */

    if ( FL_abs( bw ) > FL_MAX_BW )
		bw = bw > 0 ? FL_MAX_BW : - FL_MAX_BW;

    if ( bw == 0 )
		bw = -1;

    if ( ! obj )
    {
		M_err( "fl_set_object_bw", "NULL object." );
		return;
    }

    /* check if this object is a group, if so, change all members */

    if ( obj->objclass == FL_BEGIN_GROUP )
    {
		fl_freeze_form( obj->form );
		for ( ; obj && obj->objclass != FL_END_GROUP; obj = obj->next )
		{
			if ( obj->bw != bw )
			{
				obj->bw = bw;
				if ( obj->objclass != FL_BEGIN_GROUP )
					fl_redraw_object( obj );
			}
		}

		fl_unfreeze_form( obj->form );
    }
    else if ( obj->bw != bw )
    {
		obj->bw = bw;
		fl_redraw_object( obj );
    }
}


/***************************************
 * Returns the borderwidth of an object
 ***************************************/

void
fl_get_object_bw( FL_OBJECT * obj,
				  int *       bw )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_bw", "NULL object." );
		return;
    }

	*bw = obj->bw;
}


/***************************************
 ***************************************/

Window
fl_get_real_object_window( FL_OBJECT * obj )
{
    FL_pixmap *objp = obj->flpixmap;
    FL_pixmap *formp = obj->form->flpixmap;

    if ( objp && objp->win ) 
		return objp->win;
    else if (    (    obj->objclass == FL_CANVAS
			       || obj->objclass == FL_GLCANVAS )
			  && fl_get_canvas_id( obj ) )
		return fl_get_canvas_id( obj );
    else if ( formp && formp->win )
		return formp->win;

	return obj->form->window;
}


/***************************************
 ***************************************/

FL_RECT *
fli_union_rect( const FL_RECT * r1,
				const FL_RECT * r2 )
{
    FL_RECT *p = fl_malloc( sizeof *p );
    int xi,
		yi,
		xf,
		yf;

    xi = p->x = FL_max( r1->x, r2->x );
    yi = p->y = FL_max( r1->y, r2->y );
    xf = FL_min( r1->x + r1->width,  r2->x + r2->width )  - 1;
    yf = FL_min( r1->y + r1->height, r2->y + r2->height ) - 1;

    p->width  = xf - xi + 1;
    p->height = yf - yi + 1;

	if ( p->width <= 0 || p->height <= 0 )
	{
		fl_free( p );
		return NULL;
	}

    return p;
}


/***************************************
 ***************************************/

static const FL_RECT *
get_bounding_rect( const FL_RECT * r1,
				   const FL_RECT * r2 )
{
    static FL_RECT rect;
    int xi,
		yi,
		xf,
		yf;

    xi = rect.x = FL_min( r1->x, r2->x );
    yi = rect.y = FL_min( r1->y, r2->y );
    xf = FL_max( r1->x + r1->width, r2->x + r2->width );
    yf = FL_max( r1->y + r1->height, r2->y + r2->height );

    rect.width  = xf - xi;
    rect.height = yf - yi;

    return &rect;
}


/***************************************
 ***************************************/

void
fli_scale_length( FL_Coord * x,
				  FL_Coord * w,
				  double     s )
{
    FL_Coord xi,
		     xf;

    xi = FL_crnd( s * *x );
    xf = FL_crnd( s * ( *x + *w ) );
    *x = xi;
    *w = xf - xi;
}


/***************************************
 * Scale an object. For internal use. No gravity and resize settings
 * for the object are taken into account. The calculation takes care
 * of round-off errors and has the property that if two objects were
 * "glued" before scaling, they will remain so.
 ***************************************/

void
fl_scale_object( FL_OBJECT * obj,
				 double      xs,
				 double      ys )
{
    if ( xs == 1.0 && ys == 1.0 )
		return;

	if ( ! obj->form )
	{
		obj->x = FL_crnd( xs * obj->x );
		obj->y = FL_crnd( ys * obj->y );
		obj->w = FL_crnd( xs * obj->w );
		obj->h = FL_crnd( ys * obj->h );
	}
	else
	{
		double new_w = xs * ( obj->fl2 - obj->fl1 ),
			   new_h = ys * ( obj->ft2 - obj->ft1 );

		obj->fl1 *= xs;
		obj->fr1  = obj->form->w_hr - obj->fl1;
		obj->ft1 *= ys;
		obj->fb1  = obj->form->h_hr - obj->ft1;

		obj->fl2  = obj->fl1 + new_w;
		obj->fr2  = obj->form->w_hr - obj->fl2;
		obj->ft2  = obj->ft1 + new_h;;
		obj->fb2  = obj->form->h_hr - obj->ft2;;

		obj->x    = FL_crnd( obj->fl1 );
		obj->y    = FL_crnd( obj->ft1 );
		obj->w    = FL_crnd( new_w );
		obj->h    = FL_crnd( new_h );

		if ( fli_inverted_y )
			obj->y = TRANY( obj, obj->form );

		fli_recalc_intersections( obj->form );
	}
}


/***************************************
 * Register a preemptive object handler
 ***************************************/

FL_HANDLEPTR
fl_set_object_prehandler( FL_OBJECT *  obj,
						  FL_HANDLEPTR phandler )
{
    FL_HANDLEPTR oldh = obj->prehandle;

    obj->prehandle = phandler;
    return oldh;
}


/***************************************
 ***************************************/

FL_HANDLEPTR
fl_set_object_posthandler( FL_OBJECT    * obj,
						   FL_HANDLEPTR   post )
{
    FL_HANDLEPTR oldh = obj->posthandle;
    obj->posthandle = post;
    return oldh;
}


/***************************************
 ***************************************/

void
fl_trigger_object( FL_OBJECT * obj )
{
    if (    obj
		 && obj != FL_EVENT
		 && obj->form
		 && obj->visible
		 && obj->active )
		fli_object_qenter( obj );
}


/***************************************
 ***************************************/

void
fl_draw_object_label( FL_OBJECT * obj )
{
    int align = obj->align % FL_ALIGN_INSIDE;

	if ( align != obj->align )
		fl_drw_text( align, obj->x, obj->y, obj->w, obj->h,
					 obj->lcol, obj->lstyle, obj->lsize, obj->label );
	else
		fl_drw_text_beside( align, obj->x, obj->y, obj->w, obj->h,
							obj->lcol, obj->lstyle, obj->lsize, obj->label );
}


/***************************************
 ***************************************/

void
fl_draw_object_label_outside( FL_OBJECT * obj )
{
    fl_drw_text_beside( obj->align & ~FL_ALIGN_INSIDE, obj->x, obj->y, obj->w,
						obj->h, obj->lcol, obj->lstyle, obj->lsize,
						obj->label );
}


/***************************************
 ***************************************/

void
fl_call_object_callback( FL_OBJECT * obj )
{
    if ( ! obj )
    {
		M_err( "fl_call_object_callback", "NULL object." );
		return;
    }

    if ( obj->object_callback )
		obj->object_callback( obj, obj->argument );
}


/***************************************
 * Rechecks for all objects of a form if they are
 * partially or fully hidden by another object
 ***************************************/

void
fli_recalc_intersections( FL_FORM * form )
{
	FL_OBJECT *obj;

	for ( obj = form->first; obj && obj->next; obj = obj->next )
		obj->is_under = object_is_under( obj );
}


/***************************************
 ***************************************/

void
fl_move_object( FL_OBJECT * obj,
				FL_Coord    dx,
				FL_Coord    dy )
{
     FL_Coord x,
		      y;

    if ( obj->objclass == FL_BEGIN_GROUP )
    {
		FL_OBJECT *o;

        fl_freeze_form( obj->form );

        for ( o = obj->next;  o->objclass != FL_END_GROUP; o = o->next )
        {
			fl_get_object_position( o, &x, &y );
			fl_set_object_position( o, x + dx, y + dy );
        }

        fl_unfreeze_form( obj->form );
    }
    else
    {
        fl_get_object_position( obj, &x, &y );
        fl_set_object_position( obj, x + dx, y + dy );
    }
}


/***************************************
 * Returns the position of an object
 ***************************************/

void
fl_get_object_position( FL_OBJECT * obj,
						FL_Coord  * x,
						FL_Coord  * y )
{
    *x = obj->x;
    *y = fli_inverted_y ? TRANY( obj, obj->form ) : obj->y;
}


/***************************************
 * Sets the position of an object
 ***************************************/

void
fl_set_object_position( FL_OBJECT * obj,
						FL_Coord    x,
						FL_Coord    y )
{
	int visible = obj->visible;
	double diff;

    if ( obj->x == x && obj->y == y )
		return;

    if ( fli_inverted_y )
		y = obj->form->h - obj->h - y;

    if ( obj->x == x && obj->y == y )
		return;

	if ( visible )
		fl_hide_object( obj );

	if ( x != obj->x )
	{
		diff = x - obj->fl1;
		obj->fl1 += diff;
		obj->fl2 += diff;
		obj->fr1 -= diff;
		obj->fr2 -= diff;
		obj->x = x;
	}

	if ( y != obj->y )
	{
		diff = y - obj->ft1;
		obj->ft1 += diff;
		obj->ft2 += diff;
		obj->fb1 -= diff;
		obj->fb2 -= diff;
		obj->y = y;
	}

	if ( fli_inverted_y )
		obj->y = TRANY( obj, obj->form );

	if ( ! obj->is_child )
		fli_recalc_intersections( obj->form );

    if ( visible )
		fl_show_object( obj );
}


/***************************************
 * Returns the size of an object
 ***************************************/

void
fl_get_object_size( FL_OBJECT * obj,
					FL_Coord  * w,
					FL_Coord  * h )
{
    *w = obj->w;
    *h = obj->h;
}


/***************************************
 * Sets the size of an object
 ***************************************/

void
fl_set_object_size( FL_OBJECT * obj,
					FL_Coord    w,
					FL_Coord    h )
{
	int visible = obj->visible;
	double diff;

    if ( obj->w == w && obj->h == h )
		return;

	if ( visible )
		fl_hide_object( obj );

	if ( w != obj->w )
	{
		diff = w - ( obj->fl2 - obj->fl1 );

		if ( HAS_FIXED_HORI_ULC_POS( obj ) )
		{
			obj->fl2 += diff;
			obj->fr2 -= diff;
		}
		if ( HAS_FIXED_HORI_LRC_POS( obj ) )
		{
			obj->fl1 -= diff;
			obj->fr1 += diff;
		}
		else    /* keep center of gravity */
		{
			diff *= 0.5;
			obj->fl1 -= diff;
			obj->fr1 += diff;
			obj->fl2 += diff;
			obj->fr2 -= diff;
		}

		obj->x = FL_crnd( obj->fl1 );
		obj->w = FL_crnd( obj->fl2 - obj->fl1 );
	}

	if ( h != obj->h )
	{
		diff = h - ( obj->ft2 - obj->ft1 );

		if ( HAS_FIXED_VERT_ULC_POS( obj ) )
		{
			obj->ft2 += diff;
			obj->fb2 -= diff;
		}
		else if ( HAS_FIXED_VERT_LRC_POS( obj ) )
		{
			obj->ft1 -= diff;
			obj->fb1 += diff;
		}
		else    /* keep center of gravity */
		{
			diff *= 0.5;
			obj->ft1 -= diff;
			obj->fb1 += diff;
			obj->ft2 += diff;
			obj->fb2 -= diff;
		}

		obj->y = FL_crnd( obj->ft1 );
		obj->h = FL_crnd( obj->ft2 - obj->ft1 );
	}

	if ( fli_inverted_y )
		obj->y = TRANY( obj, obj->form );

	if ( ! obj->is_child )
		fli_recalc_intersections( obj->form );

	fli_handle_object_direct( obj, FL_RESIZED, 0, 0, 0, NULL );

    if ( visible )
		fl_show_object( obj );
}


/***************************************
 * Returns the position and size of an object
 ***************************************/

void
fl_get_object_geometry( FL_OBJECT * obj,
						FL_Coord *  x,
						FL_Coord *  y,
						FL_Coord *  w,
						FL_Coord *  h )
{
    fl_get_object_position( obj, x, y );
    fl_get_object_size( obj, w, h );
}


/***************************************
 * Sets the position and size of an object
 ***************************************/

void
fl_set_object_geometry( FL_OBJECT * obj,
						FL_Coord    x,
						FL_Coord    y,
						FL_Coord    w,
						FL_Coord    h )
{
	fl_set_object_size( obj, w, h );
	fl_set_object_position( obj, x, y );
}


/***************************************
 * Computes object geometry also taking the label into account
 ***************************************/

void
fl_get_object_bbox( FL_OBJECT * obj,
					FL_Coord  * x,
					FL_Coord  * y,
					FL_Coord  * w,
					FL_Coord  * h )
{
    FL_OBJECT *tmp;
    int extra = 0;
    XRectangle lrect,
		       orect;
    const XRectangle *xr;

    if ( obj->objclass == FL_FRAME || obj->objclass == FL_LABELFRAME )
		extra += FL_abs( obj->bw );

    if (    obj->objclass >= FL_USER_CLASS_START
		 && obj->objclass <= FL_USER_CLASS_END )
		extra = FL_abs( obj->bw ) + obj->lsize;

    lrect.x      = orect.x      = obj->x - extra;
    lrect.y      = orect.y      = obj->y - extra;
    lrect.width  = orect.width  = obj->w + 2 * extra;
    lrect.height = orect.height = obj->h + 2 * extra;

    /* label position */

    if ( obj->label && *obj->label )
    {
		int len = strlen( obj->label );
		int sw,
			sh;
		int xx,
			yy,
			a,
			d;

		fl_get_string_dimension( obj->lstyle, obj->lsize, obj->label, len,
								 &sw, &sh );
		fl_get_char_height( obj->lstyle, obj->lsize, &a, &d );
		fl_get_align_xy( obj->align, obj->x, obj->y, obj->w, obj->h,
						 sw, sh + d, 3, 3, &xx, &yy );
		lrect.x      = xx - 1;
		lrect.y      = yy;
		lrect.width  = sw + 1;
		lrect.height = sh + d;
    }

    xr = get_bounding_rect( &lrect, &orect );

    for ( tmp = obj->child; tmp; tmp = tmp->nc )
    {
		lrect        = *xr;
		orect.x      = tmp->x;
		orect.y      = tmp->y;
		orect.width  = tmp->w;
		orect.height = tmp->h;

		xr = get_bounding_rect( &lrect, &orect );
    }

    *x = xr->x;
    *y = xr->y;
    *w = xr->width;
    *h = xr->height;
}


/***************************************
 ***************************************/

static void
get_object_bbox_rect( FL_OBJECT * obj,
					  FL_RECT   * xr )
{
	FL_Coord  x,
		      y,
		      w,
		      h;

    fl_get_object_bbox( obj, &x, &y, &w, &h );

	xr->x = x;
	xr->y = y;
	xr->width = w;
	xr->height = h;
}


/***************************************
 ***************************************/

void
fl_set_object_automatic( FL_OBJECT * obj,
						 int         flag )
{
    if ( obj->automatic != ( flag ? 1 : 0 ) )
    {
		obj->automatic = flag ? 1 : 0;

		if ( obj->form )
		{
			if ( flag )
				obj->form->has_auto_objects++;
			else
				obj->form->has_auto_objects--;
		}

		fli_recount_auto_objects( );
    }
}


/***************************************
 ***************************************/

static void
lose_focus( FL_OBJECT * obj )
{
	FL_FORM *form = obj->form;

	if ( ! form || ! obj->focus || obj != form->focusobj )
		return;

	if ( obj == form->focusobj )
		fli_handle_object( form->focusobj, FL_UNFOCUS, 0, 0, 0, NULL );

	obj->focus = 0;

	/* Try to find some input object to give it the focus */

	obj->input = 0;
	form->focusobj = fli_find_first( obj->form, FL_FIND_INPUT, 0, 0 );
	obj->input = 1;

	if ( obj == refocus )
		refocus = form->focusobj ? form->focusobj : NULL;

	if ( form->focusobj )
		fli_handle_object_direct( form->focusobj, FL_FOCUS, 0, 0, 0, NULL );
}


/***************************************
 * Part of the public interface, not used within the library
 ***************************************/

void
fl_for_all_objects( FL_FORM * form,
					int       ( * cb )( FL_OBJECT *, void * ),
					void    * v )
{
    FL_OBJECT *obj;

    if ( ! form )
    {
		M_err( "fl_for_all_objects", "NULL form." );
		return;
    }

    if ( ! cb )
    {
		M_err( "fl_for_all_objects", "NULL callback function." );
		return;
    }

    for ( obj = form->first; obj && ! cb( obj, v ); obj = obj->next )
		/* empty */ ;
}


/***************************************
 ***************************************/

void
fl_set_object_helper( FL_OBJECT  * obj,
					  const char * tip )
{
    if ( ! obj )
    {
		M_err( "fl_set_object_helper", "NULL object." );
		return;
    }

	fl_safe_free( obj->tooltip );
    obj->tooltip = tip ? fl_strdup( tip ) : NULL;
}
