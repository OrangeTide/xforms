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
 * \file text.c
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * Text object
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"


/***************************************
 ***************************************/

static int
handle_text( FL_OBJECT * ob,
			 int         event,
			 FL_Coord    mx   FL_UNUSED_ARG,
			 FL_Coord    my   FL_UNUSED_ARG,
			 int         key  FL_UNUSED_ARG,
			 void *      ev   FL_UNUSED_ARG )
{
    switch ( event )
    {
		case FL_DRAW:
			ob->align |= FL_ALIGN_INSIDE;
			fl_drw_box( ob->boxtype, ob->x, ob->y, ob->w, ob->h, ob->col1,
						ob->bw );
			/* fall through */

		case FL_DRAWLABEL:
			fl_set_text_clipping( ob->x + FL_abs( ob->bw ), ob->y + 2,
								  ob->w - 2 * FL_abs( ob->bw ), ob->h - 4 );
			fl_draw_object_label( ob );
			fl_unset_text_clipping( );
			break;
    }

    return FL_RETURN_NONE;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_create_text( int          type,
				FL_Coord     x,
				FL_Coord     y,
				FL_Coord     w,
				FL_Coord     h,
				const char * label )
{
    FL_OBJECT *ob;

    ob = fl_make_object( FL_TEXT, type, x, y, w, h, label, handle_text );
    ob->boxtype = FL_TEXT_BOXTYPE;
    ob->col1 = FL_TEXT_COL1;
    ob->col2 = FL_TEXT_COL2;
    ob->lcol = FL_TEXT_LCOL;
    ob->align = FL_TEXT_ALIGN | FL_ALIGN_INSIDE;
    ob->active = 0;
    return ob;
}


/***************************************
 ***************************************/

FL_OBJECT *
fl_add_text( int          type,
			 FL_Coord     x,
			 FL_Coord     y,
			 FL_Coord     w,
			 FL_Coord     h,
			 const char * label)
{
    FL_OBJECT *ob;

    ob = fl_create_text( type, x, y, w, h, label );
    fl_add_object( fl_current_form, ob );

    return ob;
}
