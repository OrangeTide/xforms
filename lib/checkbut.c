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
 * \file checkbut.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao
 *  All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include "include/forms.h"
#include "flinternal.h"


/**************************************************************************
 * Check buttons
 **********************************************************************{*/

static void
draw_checkbutton( FL_OBJECT * ob )
{
    FL_Coord xx,
		     yy,
		     ww,
		     hh,
		     bw = FL_abs( ob->bw );
	FL_BUTTON_STRUCT *sp = ob->spec;

    if ( sp->event == FL_ENTER || sp->event == FL_LEAVE )
		return;

    fl_drw_box( ob->boxtype, ob->x, ob->y, ob->w, ob->h, ob->col1, ob->bw );

    ww = hh = 0.6 * FL_min( ob->w, ob->h );
    xx = ob->x + 4.5;
    yy = ob->y + ( ob->h - hh ) / 2;

    if ( sp->val )
		fl_drw_checkbox( FL_DOWN_BOX, xx, yy, ww, hh, ob->col2, bw );
    else
		fl_drw_checkbox( FL_UP_BOX, xx, yy, ww, hh, ob->col1, bw );

    if ( ob->align == FL_ALIGN_CENTER )
		fl_drw_text( FL_ALIGN_LEFT, xx + ww + 1, ob->y, ob->w - ww - 3, ob->h,
					 ob->lcol, ob->lstyle, ob->lsize, ob->label );
    else
		fl_drw_text_beside( ob->align, ob->x, ob->y, ob->w, ob->h,
							ob->lcol, ob->lstyle, ob->lsize, ob->label );

    if ( ob->type == FL_RETURN_BUTTON )
		fl_drw_text( 0,
					 ob->x + ob->w - 0.8f * ob->h,
					 ob->y + 0.2f * ob->h,
					 0.6f * ob->h, 0.6f * ob->h,
					 ob->lcol, 0, 0, "@returnarrow" );
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_create_checkbutton( int          type,
					   FL_Coord     x,
					   FL_Coord     y,
					   FL_Coord     w,
					   FL_Coord     h,
					   const char * label )
{
    FL_OBJECT *ob;

    fl_add_button_class( FL_CHECKBUTTON, draw_checkbutton, 0 );
    ob = fl_create_generic_button( FL_CHECKBUTTON, type, x, y, w, h, label );

    ob->boxtype = FL_CHECKBUTTON_BOXTYPE;
    ob->col1    = FL_CHECKBUTTON_COL1;
    ob->col2    = FL_CHECKBUTTON_COL2;
    ob->align   = FL_CHECKBUTTON_ALIGN;
    ob->lcol    = FL_CHECKBUTTON_LCOL;

    return ob;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_add_checkbutton( int          type,
					FL_Coord     x,
					FL_Coord     y,
					FL_Coord     w,
					FL_Coord     h,
					const char * label )
{
    FL_OBJECT *ob = fl_create_checkbutton( type, x, y, w, h, label );

    fl_add_object( fl_current_form, ob );
    return ob;
}
