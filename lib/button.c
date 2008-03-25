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
 * \file button.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *  All Buttons. Additional button class can be added via
 *  fl_add_button_class and fl_create_generic_button
 */

#if defined(F_ID) || defined(DEBUG)
char *fl_id_but = "$Id: button.c,v 1.9 2008/03/25 12:41:27 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include <sys/types.h>

typedef FL_BUTTON_STRUCT SPEC;

extern FL_FORM *fl_popup_form;

/***************************************
 ***************************************/

static void
free_pixmap( SPEC * sp )
{
    if ( sp->pixmap != None )
	{
		XFreePixmap( flx->display, sp->pixmap );
		sp->pixmap = None;
	}
}


#define ISTABBOX( t )   (    t == FL_TOPTAB_UPBOX			       \
						  || t == FL_SELECTED_TOPTAB_UPBOX	       \
						  || t == FL_BOTTOMTAB_UPBOX			   \
						  || t == FL_SELECTED_BOTTOMTAB_UPBOX    )


/********** DRAWING *************/

/***************************************
 * Draws the object
 ***************************************/

static void
draw_button( FL_OBJECT * ob )
{
    FL_COLOR col = ( ( SPEC * ) ( ob->spec ) )->val ? ob->col2 : ob->col1;
    FL_Coord dh,
		     dw,
		     ww,
		     absbw = FL_abs( ob->bw );
    int off2 = 0;

    if ( ob->belowmouse && col == FL_BUTTON_COL1 )
		col = FL_BUTTON_MCOL1;
    if ( ob->belowmouse && col == FL_BUTTON_COL2 )
		col = FL_BUTTON_MCOL2;

    if ( FL_IS_UPBOX( ob->boxtype ) && ( ( SPEC * ) ( ob->spec ) )->val )
		fl_drw_box( FL_TO_DOWNBOX( ob->boxtype ), ob->x, ob->y, ob->w,
					ob->h, col, ob->bw );
    else
		fl_drw_box( ob->boxtype, ob->x, ob->y, ob->w, ob->h, col, ob->bw );

    dh = FL_crnd( 0.6 * ob->h );
    dw = FL_crnd( FL_min( 0.6 * ob->w, dh ) );

    ww = FL_crnd( 0.75 * ob->h );
    if ( ww < ( dw + absbw + 1 + ( ob->bw > 0 ) ) )
		ww = dw + absbw + 1 + ( ob->bw > 0 );

    if ( ob->type == FL_RETURN_BUTTON )
    {
		fl_drw_text(0, ob->x + ob->w - ww, FL_crnd( ob->y + 0.2 * ob->h ),
					dw, dh, ob->lcol, 0, 0, "@returnarrow" );
		off2 = dw - 2;
    }

    if ( ob->type == FL_MENU_BUTTON && ob->boxtype == FL_UP_BOX )
    {
		int dbh = FL_max( absbw - 1, 1 );

		dw = FL_max( 0.11 * ob->w, 13 );
		dh = FL_max( 6 + (ob->bw > 0 ), ob->h * 0.1 );

		fl_drw_box( FL_UP_BOX, ob->x + ob->w - dw - absbw - 2,
					ob->y + ( ob->h - dh ) / 2, dw, dh, ob->col1, -dbh );
		off2 = dw - 1;
    }

    if ( ob->type == FL_MENU_BUTTON || ob->type == FL_RETURN_BUTTON )
    {
		ob->w -= off2;
		fl_draw_object_label( ob );
		ob->w += off2;
    }
    else if ( ob->boxtype & FL_BROKEN_BOX || ISTABBOX( ob->boxtype ) )
    {
		fl_set_text_clipping( ob->x + 3, ob->y, ob->w - 6, ob->h );
		fl_draw_object_label( ob );
		fl_unset_text_clipping( );
    }
    else
		fl_draw_object_label( ob );
}


/***************************************
 * All button classes differ only in the way they are drawn, so we
 * separate this info out from generic button handlers
 ***************************************/

#define MAX_BUTCLASS 12

typedef struct
{
    FL_DrawButton    drawbutton;
    FL_CleanupButton cleanup;
    int              bclass;			/* button class */
} ButtonRec;

static ButtonRec how_draw[ MAX_BUTCLASS ];


/***************************************
 * lookup a drawing function given a button class ID
 ***************************************/

static FL_DrawButton
lookup_drawfunc( int bclass )
{
    ButtonRec *db = how_draw, *dbs;

    for ( dbs = db + MAX_BUTCLASS; db < dbs; db++ )
		if ( db->bclass == bclass )
			return db->drawbutton;

    return 0;
}


/***************************************
 ***************************************/

static FL_CleanupButton
lookup_cleanupfunc( int bclass )
{
    ButtonRec *db = how_draw, *dbs;

    for ( dbs = db + MAX_BUTCLASS; db < dbs; db++ )
		if ( db->bclass == bclass )
			return db->cleanup;

    return 0;
}


/***************************************
 ***************************************/

/* associate a button class with the drawing function */
void
fl_add_button_class( int              bclass,
					 FL_DrawButton    drawit,
					 FL_CleanupButton cleanup )
{
    static int initialized;
    ButtonRec *db = how_draw,
		      *dbs,
		      *first_avail;

    if ( ! initialized )
    {
		for ( dbs = db + MAX_BUTCLASS; db < dbs; db++ )
			db->bclass = -1;

		initialized = 1;
    }

    for ( first_avail = 0, db = how_draw, dbs = db + MAX_BUTCLASS; db < dbs;
		  db++ )
    {
		if ( db->bclass == bclass )
		{
			db->drawbutton = drawit;
			db->cleanup = cleanup;
			return;
		}
		else if ( db->bclass < 0 && ! first_avail )
			first_avail = db;
    }

    /* if we got here, the class is not defined yet */

    if ( first_avail )
    {
		first_avail->bclass = bclass;
		first_avail->drawbutton = drawit;
		first_avail->cleanup = cleanup;
    }
    else
    {
		M_err( "AddButtonClass", "Exceeding limit: %d", MAX_BUTCLASS );
    }
}


/***************************************
 ***************************************/

static void
wait_for_release( XKeyEvent * ev )
{
    KeySym ksm;

    if ( ( ksm = XLookupKeysym( ev, 0 ) ) == NoSymbol )
		return;

    while ( fl_keysym_pressed( ksm ) )
    {
		XSync( flx->display, 0 );
		fl_msleep( 15 );
    }
}


/***************************************
 * Handles an event
 ***************************************/

static int
handle_it( FL_OBJECT * ob,
		   int         event,
		   FL_Coord    mx,
		   FL_Coord    my,
		   int         key,
		   void *      ev )
{
    static int oldval;
    int newval;
    SPEC *sp = ob->spec;
    FL_DrawButton drawit;
    FL_CleanupButton cleanup;

#if FL_DEBUG >= ML_DEBUG
    M_info2( "HandleButton", "Handling %s", fl_event_name( event ) );
#endif

    switch ( event )
    {
		case FL_DRAW:
			if (    ob->type != FL_HIDDEN_BUTTON
				 && ob->type != FL_HIDDEN_RET_BUTTON )
			{
				if ( ( drawit = lookup_drawfunc( ob->objclass ) ) )
					drawit( ob );
				else
					M_err( "ButtonDraw", "Unknown class: %d", ob->objclass );
			}
			sp->event = FL_DRAW;
			break;

		case FL_DRAWLABEL:
			sp->event = event;
			break;			/* TODO. Missing labels */

		case FL_LEAVE:
			/* FL_MENU_BUTTON objects never get a FL_RELEASE event,
			   so we have to "fake" it */

			if ( ob->type == FL_MENU_BUTTON )
			{
				sp->event = FL_RELEASE;
				sp->val = 0;
			}
			/* fall through */

		case FL_ENTER:
			sp->event = event;
			if ( ! fl_dithered( fl_vmode ) )
				fl_redraw_object(ob);
			break;

		case FL_PUSH:
			if ( key != FL_MBUTTON1 )
				break;

			sp->event = FL_PUSH;

			if ( ob->type == FL_RADIO_BUTTON )
			{
				sp->val = 1;
				fl_redraw_object( ob );
				return 1;
			}

			oldval = sp->val;
			sp->val = ! sp->val;
			sp->mousebut = key;
			sp->timdel = 1;
			fl_redraw_object( ob );
			fl_popup_form = ob->form;
			return    ob->type == FL_INOUT_BUTTON
				   || ob->type == FL_TOUCH_BUTTON
				   || ob->type == FL_MENU_BUTTON;

		case FL_RELEASE:
			if ( key != FL_MBUTTON1 )
				break;

			sp->event = FL_RELEASE;
			if ( ob->type == FL_PUSH_BUTTON )
				return sp->val != oldval;
			else if ( sp->val == 0 && ob->type != FL_MENU_BUTTON )
				return 0;
			sp->val = 0;
			fl_redraw_object( ob );
			return ob->type != FL_TOUCH_BUTTON && ob->type != FL_MENU_BUTTON;

		case FL_UPDATE:
			sp->event = FL_UPDATE;
			if ( ob->type != FL_RADIO_BUTTON && ob->type != FL_INOUT_BUTTON )
			{
				if (    mx < ob->x
					 || mx > ob->x + ob->w
					 || my < ob->y
					 || my > ob->y + ob->h )
					newval = oldval;
				else
					newval = ! oldval;
				if ( sp->val != newval )
				{
					sp->val = newval;
					fl_redraw_object( ob );
				}
			}
			return    sp->val
				   && ob->type == FL_TOUCH_BUTTON
				   && sp->timdel++ > 10
				   && ( sp->timdel & 1 ) == 0;

		case FL_SHORTCUT:
			sp->event = FL_SHORTCUT;

			/* this is a horrible hack */

			if ( ob->type == FL_PUSH_BUTTON || ob->type == FL_RADIO_BUTTON )
			{
				sp->val = ! sp->val;
				ob->pushed = ob->type == FL_RADIO_BUTTON;
				fl_redraw_object( ob );
				wait_for_release( ev );
			}
			else if (    ob->type == FL_NORMAL_BUTTON
					  || ob->type == FL_RETURN_BUTTON )
			{
				int obl = ob->belowmouse;

				sp->val = ob->belowmouse = 1;
				fl_redraw_object( ob );
				wait_for_release( ev );
				sp->val = 0;
				ob->belowmouse = obl;
				fl_redraw_object( ob );
			}
			sp->mousebut = FL_SHORTCUT + key;
			return 1;

		case FL_FREEMEM:
			if ( ( cleanup = lookup_cleanupfunc( ob->objclass ) ) )
				cleanup( sp );
			free_pixmap( sp );
			fl_free( ob->spec );
			return 0;
    }

    return 0;
}


/***************************************
 * Creates an object
 ***************************************/

FL_OBJECT *
fl_create_generic_button( int          objclass,
						  int          type,
						  FL_Coord     x,
						  FL_Coord     y,
						  FL_Coord     w,
						  FL_Coord     h,
						  const char * label )
{
    FL_OBJECT *ob;
	SPEC *sp;

    ob = fl_make_object( objclass, type, x, y, w, h, label, handle_it );
    if ( type == FL_RADIO_BUTTON )
		ob->radio = 1;
    if ( type == FL_RETURN_BUTTON || type == FL_HIDDEN_RET_BUTTON )
		fl_set_object_shortcut( ob, "^M", 0 );
    if ( type == FL_HIDDEN_BUTTON || type == FL_HIDDEN_RET_BUTTON )
		ob->boxtype = FL_NO_BOX;
	if ( type == FL_TOUCH_BUTTON )
		ob->want_update = 1;

    ob->spec_size = sizeof *sp;
    sp = ob->spec = fl_calloc( 1, sizeof *sp );
    sp->event     = FL_DRAW;
    sp->pixmap    = sp->mask = sp->focus_pixmap = sp->focus_mask = None;
	sp->cspecv    = NULL;
	sp-> filename = sp->focus_filename = NULL;

    if ( fl_cntl.buttonLabelSize )
		ob->lsize = fl_cntl.buttonLabelSize;
    return ob;
}


/***************************************
 * Sets the button
 ***************************************/

void
fl_set_button( FL_OBJECT * ob,
			   int         pushed )
{
	SPEC *sp = ob->spec;

	pushed = pushed ? 1 : 0;     /* button can only be on or off */

    if ( sp->val != pushed )
    {
		if ( ob->type == FL_RADIO_BUTTON )
		{
			if ( ! pushed )
				return;

			fl_do_radio_push( ob, ob->x, ob->y, FL_MBUTTON1, NULL );
		}			

		sp->val = pushed;
		fl_redraw_object( ob );
    }
}


/***************************************
 * Returns value of the button
 ***************************************/

int
fl_get_button( FL_OBJECT * ob )
{
    return ( ( SPEC * ) ob->spec )->val;
}


/***************************************
 * Returns the number of the last used mouse button.
 *  fl_mouse_button will also return the mouse number
 ***************************************/

int
fl_get_button_numb( FL_OBJECT * ob )
{
    return ( ( SPEC * ) ob->spec )->mousebut;
}


/***************************************
 * creates an object
 ***************************************/

FL_OBJECT *
fl_create_button( int          type,
				  FL_Coord     x,
				  FL_Coord     y,
				  FL_Coord     w,
				  FL_Coord     h,
				  const char * label )
{
    FL_OBJECT *ob;

    fl_add_button_class( FL_BUTTON, draw_button, 0 );
    ob = fl_create_generic_button( FL_BUTTON, type, x, y, w, h, label );
    ob->boxtype = FL_BUTTON_BOXTYPE;
    ob->col1 = FL_BUTTON_COL1;
    ob->col2 = FL_BUTTON_COL2;
    ob->align = FL_BUTTON_ALIGN;
    ob->lcol = FL_BUTTON_LCOL;
    return ob;
}


/***************************************
 * Adds an object
 ***************************************/

FL_OBJECT *
fl_add_button( int          type,
			   FL_Coord     x,
			   FL_Coord     y,
			   FL_Coord     w,
			   FL_Coord     h,
			   const char * label)
{
    FL_OBJECT *ob;

    ob = fl_create_button( type, x, y, w, h, label );
    fl_add_object( fl_current_form, ob );
    return ob;
}
