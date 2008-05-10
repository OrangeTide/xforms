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
 * \file menu.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *
 *  XForms Class FL_MENU.
 *    call PopUp to handle the actual random access
 *
 *  possible optimization:
 *   upon first creation of the popup, set extern_menu to the popup ID
 *   and let popup manage all the modes/values from then on.
 *
 *  OR use the following to simplify code for extern pup
 *    when extern_menu is used, gets all text and mode, then
 *    all the code would be the same for extern and native menu.
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_menu = "$Id: menu.c,v 1.15 2008/05/10 17:46:10 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pmenu.h"

#include <string.h>
#include <stdlib.h>


#define ISPUP( sp )   ( ( sp )->extern_menu >= 0 )


extern Window fl_popup_parent_window;


/***************************************
 * Due to the possibility of %t %x present in the item str, an item's
 * value and its index may be different.
 ***************************************/

static int
val_to_index( FLI_MENU_SPEC * sp,
			  int             val )
{
    int i;

    for ( i = 1; i <= sp->numitems; i++ )
		if ( val == sp->mval[ i ] )
			return i;

    return 0;
}


/***************************************
 * Creates the menu and shows it. Returns the item selected.
 ***************************************/

static int
do_menu_low_level( FL_OBJECT * ob )
{
    int popup_id,
		i,
		val;
    FLI_MENU_SPEC *sp = ob->spec;

    if ( sp->numitems == 0 && sp->extern_menu < 0 )
		return 0;

	fl_popup_parent_window = FL_ObjWin( ob );

    if ( sp->extern_menu >= 0 )
    {
		Window oparent,
			   win;

		fli_getpup_window( sp->extern_menu, &oparent, &win );

		if ( ob->label && *ob->label && ob->type != FL_PULLDOWN_MENU )
			fl_setpup_title( sp->extern_menu, ob->label );

		fli_reparent_pup( sp->extern_menu, FL_ObjWin( ob ) );
		val = fl_dopup( sp->extern_menu );

		if ( val > 0 )
			sp->val = val;

		/* menu might go away, need to restore old parent */

		fli_reparent_pup( sp->extern_menu, oparent );
		return val;
    }

    popup_id = fl_newpup( FL_ObjWin( ob ) );

    if ( ob->type != FL_PULLDOWN_MENU && ! sp->no_title )
		fl_setpup_title( popup_id, ob->label );
    else
    {
		fl_setpup_shadow( popup_id, 0 );
		fl_setpup_softedge( popup_id, 1 );
    }

    for ( i = 1; i <= sp->numitems; i++ )
    {
		fl_addtopup( popup_id, sp->items[ i ] );
		if (    ( sp->modechange[ i ] || sp->mode[ i ] != FL_PUP_NONE )
			 && sp->mval[ i ] )
		{
			fl_setpup_mode( popup_id, sp->mval[ i ], sp->mode[ i ] );
			sp->modechange[ i ] = 0;
		}
		fl_setpup_shortcut( popup_id, i, sp->shortcut[ i ] );
    }

    val = fl_dopup( popup_id );

    if ( val > 0 )
    {
		/* if shown for the first time, need to get all mode right as the
		   menu item string may have embedded mode setting strings in it
		   (e.g., R1 etc) */

		if ( sp->shown == 0 )
		{
			for ( i = 1; i <= sp->numitems; i++ )
			{
				int m = fl_getpup_mode( popup_id, sp->mval[ i ] );

				sp->modechange[ i ] = sp->mode[ i ] != m;
				sp->mode[ i ] = m;
				sp->shown = 1;
			}
		}
		else
		{
			int k = val_to_index( sp, val );

			sp->mode[ k ] = fl_getpup_mode( popup_id, val );
			sp->modechange[ k ] = 1;

			/* old val also might change mode if binary */

			if ( sp->val > 0 )
			{
				int m = fl_getpup_mode( popup_id, sp->val );

				k = val_to_index( sp, sp->val );
				sp->modechange[ k ] = sp->mode[ k ] != m;
				sp->mode[ k ] = m;
			}
		}

		sp->val = val;
    }

    fl_freepup( popup_id );

    return val;
}


/***************************************
 ***************************************/

static int
do_menu( FL_OBJECT *ob )
{
    FLI_MENU_SPEC *sp = ob->spec;
	int val;

	ob->pushed = 1;
	fl_redraw_object( ob );

	if (    ob->type == FL_PULLDOWN_MENU
		 || ( ob->type == FL_PUSH_MENU && sp->no_title ) )
		fl_setpup_position( ob->form->x + ob->x + 2,
							ob->form->y + ob->y + ob->h + 2 * FL_PUP_PADH + 1 );

	val = do_menu_low_level( ob ) > 0;

	ob->pushed = 0;
	fl_redraw_object( ob );

	return val > 0;
}


/***************************************
 * Handles an event, return non-zero if an item has been selected.
 ***************************************/

static int
handle_menu( FL_OBJECT * ob,
			 int         event,
			 FL_Coord    mx   FL_UNUSED_ARG,
			 FL_Coord    my   FL_UNUSED_ARG,
			 int         key  FL_UNUSED_ARG,
			 void *      ev   FL_UNUSED_ARG )
{
    FLI_MENU_SPEC *sp = ob->spec;
    int val,
		boxtype = ob->boxtype;
    FL_COLOR col;

#if FL_DEBUG >= ML_DEBUG
    M_info2( "handle_menu", fli_event_name( event ) );
#endif

    switch ( event )
    {
		case FL_DRAW:
			/* Draw the object */

			if ( ob->pushed )
			{
				boxtype = FL_UP_BOX;
				col = ob->col2;
			}
			else
				col = ob->col1;

			fl_drw_box( boxtype, ob->x, ob->y, ob->w, ob->h, col, ob->bw );
			fl_drw_text( ob->align, ob->x, ob->y, ob->w, ob->h,
						 ob->lcol, ob->lstyle, ob->lsize, ob->label );

			if ( sp->showsymbol )
			{
				int dm = 0.85 * FL_min( ob->w, ob->h );

				fl_drw_text( 0, ob->x + ob->w - dm - 1, ob->y + 1,
							 dm, dm, col, 0, 0, "@menu" );
			}
			break;

		case FL_ENTER:
			if ( ob->type != FL_TOUCH_MENU )
				break;

			return do_menu( ob );

		case FL_PUSH:
			/* Touch menus and push menus without a title don't do anything
			   on a button press */

			if (    key != FL_MBUTTON1
				 || ob->type == FL_TOUCH_MENU
				 || ( ob->type == FL_PUSH_MENU && sp->no_title ) )
				break;

			return do_menu( ob );

		case FL_RELEASE :
			/* Button release is only important for push menus without a
			   title, all others get started by a button press or by just
			   moving the mouse on top of it (and they also don't expect
			   a release - that gets eaten by the popup handler) */

			if (    key != FL_MBUTTON1
				 || ! ( ob->type == FL_PUSH_MENU && sp->no_title )
				 || mx < ob->x
				 || mx > ob->x + ob->w
				 || my < ob->y
				 || my > ob->y + ob->h )
				break;

			return do_menu( ob );

		case FL_SHORTCUT:
			/* Show menu as highlighted */

			ob->pushed = 1;
			fl_redraw_object( ob );

			/* Pulldown menus and those without a title appear directly
			   below the menu itself, the others more or less on top of
			   the menu */

			if ( ob->type == FL_PULLDOWN_MENU || sp->no_title )
				fl_setpup_position( ob->form->x + ob->x + 2,
									  ob->form->y + ob->y + ob->h
									+ 2 * FL_PUP_PADH + 1 );
			else
				fl_setpup_position( ob->form->x + ob->x + 5,
									ob->form->y + ob->y + 5 + 2 * FL_PUP_PADH );

			/* Do interaction and then redraw without highlighting */

			val = do_menu( ob );
			ob->pushed = 0;
			fl_redraw_object( ob );
			return val > 0;

		case FL_FREEMEM:
			fl_clear_menu( ob );
			fl_free( ob->spec );
			return 0;
    }

	return 0;
}


/***************************************
 * creates an object
 ***************************************/

FL_OBJECT *
fl_create_menu( int          type,
				FL_Coord     x,
				FL_Coord     y,
				FL_Coord     w,
				FL_Coord     h,
				const char * label )
{
    FL_OBJECT *ob;
    FLI_MENU_SPEC *sp;

    ob = fl_make_object( FL_MENU, type, x, y, w, h, label, handle_menu );

	ob->boxtype = FL_FLAT_BOX;

    ob->col1   = FL_MENU_COL1;
    ob->col2   = FL_MENU_COL2;
    ob->lcol   = FL_MENU_LCOL;
    ob->lstyle = FL_NORMAL_STYLE;
    ob->align  = FL_MENU_ALIGN;

    sp = ob->spec = fl_calloc( 1, sizeof *sp );
    sp->extern_menu = -1;

    return ob;
}


/***************************************
 * Adds an object
 ***************************************/

FL_OBJECT *
fl_add_menu( int          type,
			 FL_Coord     x,
			 FL_Coord     y,
			 FL_Coord     w,
			 FL_Coord     h,
			 const char * label )
{
    FL_OBJECT *ob;

    ob = fl_create_menu( type, x, y, w, h, label );
    fl_add_object( fl_current_form, ob );

    return ob;
}


/***************************************
 * Clears the menu object
 ***************************************/

void
fl_clear_menu( FL_OBJECT * ob )
{
    int i;
    FLI_MENU_SPEC *sp = ob->spec;

    sp->val = 0;
    sp->cur_val = 0;

    if ( ISPUP( sp ) )
    {
		fl_freepup( sp->extern_menu );
		sp->extern_menu = -1;
		return;
    }

    for ( i = 1; i <= sp->numitems; i++ )
    {
		if ( sp->items[ i ] )
			fl_free( sp->items[ i ] );
		if ( sp->shortcut[ i ] )
			fl_free( sp->shortcut[ i ] );
		sp->mode[ i ] = FL_PUP_NONE;
    }

    sp->numitems = 0;
}


/***************************************
 * Adds a line to the menu item.
 ***************************************/

static void
addto_menu( FL_OBJECT  * ob,
			const char * str )
{
    FLI_MENU_SPEC *sp = ob->spec;
    int n;

    if ( sp->numitems >= FL_MENU_MAXITEMS )
		return;

    n = ++sp->numitems;
    sp->items[ n ] = fl_strdup( str );
    sp->shortcut[ n ] = fl_strdup( "" );
    sp->mode[ n ] = FL_PUP_NONE;

    /* If we want to support %x, need to parse the string */

    if ( ! strstr( sp->items[ n ], "%t" ) )
		sp->mval[ n ] = ++sp->cur_val;
}


/***************************************
 * Sets the menu to a particular menu string
 ***************************************/

void
fl_set_menu( FL_OBJECT *  ob,
			 const char * menustr )
{
    fl_clear_menu( ob );
    fl_addto_menu( ob, menustr );
}


/***************************************
 * Adds a line to the menu item.
 ***************************************/

int
fl_addto_menu( FL_OBJECT  * ob,
			   const char * menustr )
{
    FLI_MENU_SPEC *sp= ob->spec;
    char *t,
		 *c;

#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_MENU ) )
    {
		M_err( "fl_addto_menu", "%s is not Menu class", ob ? ob->label : "" );
		return 0;
    }
#endif

	/* Split up menu string at '|' chars and create an entry for each part */

	t = fl_strdup( menustr );

    for ( c = strtok( t, "|" );
		  c && sp->numitems < FL_CHOICE_MAXITEMS;
		  c = strtok( NULL, "|" ) )
		addto_menu( ob, c );

	if ( t )
		fl_free( t );

    return sp->numitems;
}


/***************************************
 * Replaces a line in the menu item.
 ***************************************/

void
fl_replace_menu_item( FL_OBJECT *  ob,
					  int          numb,
					  const char * str )
{
    FLI_MENU_SPEC *sp = ob->spec;

    if ( ISPUP( sp ) )
		fli_replacepup_text( sp->extern_menu, numb, str );
    else
    {
		if ( numb < 1 || numb > sp->numitems )
			return;

		if ( sp->items[ numb ] )
			fl_free( sp->items[ numb ] );
		sp->items[ numb ] = fl_strdup( str );
    }
}


/***************************************
 * assign menu item values. Currently %x is not supported
 ***************************************/

#if 0
static void
gen_index( FL_OBJECT * ob )
{
    FLI_MENU_SPEC *sp = ob->spec;
    int i;

    sp->cur_val = 0;

    for ( i = 1; i <= sp->numitems; i++ )
		if ( ! strstr( sp->items[ i ], "%t" ) )
			sp->mval[ i ] = ++sp->cur_val;
}
#endif


/***************************************
 * Removes a line from the menu item.
 ***************************************/

void
fl_delete_menu_item( FL_OBJECT * ob,
					 int         numb )
{
    int i;
    FLI_MENU_SPEC *sp = ob->spec;

    if ( numb < 1 || numb > sp->numitems )
		return;

	if ( sp->items[ numb ] )
		fl_free( sp->items[ numb ] );
	if ( sp->shortcut[ numb ] )
		fl_free( sp->shortcut[ numb ] );

    for ( i = numb; i < sp->numitems; i++ )
    {
		sp->items[ i ]      = sp->items[ i + 1 ];
		sp->mode[ i ]       = sp->mode[ i + 1 ];
		sp->modechange[ i ] = sp->modechange[ i + 1 ];
		sp->mval[ i ]       = sp->mval[ i + 1 ] - 1;
		sp->shortcut[ i ]   = sp->shortcut[ i + 1 ];
    }

    sp->mode[ sp->numitems ] = FL_PUP_NONE;
    sp->items[ sp->numitems ] = NULL;
	sp->shortcut[ sp->numitems ] = NULL;
    sp->numitems--;
    sp->cur_val--;
}


/***************************************
 ***************************************/

void
fl_set_menu_item_shortcut( FL_OBJECT *  ob,
						   int          numb,
						   const char * str )
{
    FLI_MENU_SPEC *sp = ob->spec;

	if ( sp->shortcut[ numb ] )
		fl_free( sp->shortcut[ numb ] );
	sp->shortcut[ numb ] = fl_strdup( str ? str : "" );
}


/***************************************
 * Sets the display mode for the menu item
 ***************************************/

void
fl_set_menu_item_mode( FL_OBJECT *  ob,
					   int          numb,
					   unsigned int mode )
{
    FLI_MENU_SPEC *sp = ob->spec;

    if ( ISPUP( sp ) )
		fl_setpup_mode( sp->extern_menu, numb, mode );
    else
    {
		if ( numb < 1 || numb > sp->numitems )
			return;

		sp->mode[ numb ] = mode;
		sp->modechange[ numb ] = 1;

		if ( ( mode & FL_PUP_CHECK ) )
			sp->val = numb;
    }
}


/***************************************
 * Makes the menu symbol visible or not
 ***************************************/

void
fl_show_menu_symbol( FL_OBJECT * ob,
					 int         show )
{
    FLI_MENU_SPEC *sp = ob->spec;

    sp->showsymbol = show;
    fl_redraw_object( ob );
}


/***************************************
 * Returns the number of the menu item selected.
 ***************************************/

int
fl_get_menu( FL_OBJECT * ob )
{
#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_MENU ) )
    {
		M_err( "GetMenu", "%s is not Menu class", ob ? ob->label : "" );
		return 0;
    }
#endif

    return ( ( FLI_MENU_SPEC * ) ob->spec )->val;
}


/***************************************
 ***************************************/

int
fl_get_menu_maxitems( FL_OBJECT * ob )
{
    FLI_MENU_SPEC *sp = ob->spec;

#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_MENU ) )
    {
		M_err( "GetMenuMaxitems", "%s is not Menu class", ob ? ob->label : "" );
		return 0;
    }
#endif

    return ISPUP( sp ) ? fl_getpup_items( sp->extern_menu ) : sp->numitems;
}


/***************************************
 * Returns the text of the menu item selected.
 ***************************************/

const char *
fl_get_menu_text( FL_OBJECT * ob )
{
    FLI_MENU_SPEC *sp = ob->spec;
    const char *s;

#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_MENU ) )
    {
		M_err( "GetMenuText", "%s is not Menu class", ob ? ob->label : "" );
		return NULL;
    }
#endif

    if ( ISPUP( sp ) )
		s = fl_getpup_text( sp->extern_menu, sp->val );
    else
		s = ( sp->val < 1 || sp->val > sp->numitems ) ?
			NULL : sp->items[ sp->val ];
    return s;
}


/***************************************
 ***************************************/

const char *
fl_get_menu_item_text( FL_OBJECT * ob,
					   int         n )
{
    FLI_MENU_SPEC *sp = ob->spec;
    const char *s;

#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_MENU ) )
    {
		M_err( "GetMenuItemText", "%s is not Menu class", ob ? ob->label : "" );
		return NULL;
    }
#endif

    if ( ISPUP(sp ) )
		s = fl_getpup_text( sp->extern_menu, n );
    else
		s = ( n < 1 || n > sp->numitems ) ? NULL : sp->items[ n ];
    return s;
}


/***************************************
 ***************************************/

unsigned int
fl_get_menu_item_mode( FL_OBJECT * ob,
					   int         n )
{
    FLI_MENU_SPEC *sp = ob->spec;

#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_MENU ) )
    {
		M_err( "GetMenuItemMode", "%s is not Menu class", ob ? ob->label : "" );
		return 0;
    }
#endif

    if ( ISPUP( sp ) )
		return fl_getpup_mode( sp->extern_menu, n );
    else
		return ( n > 0 && n <= sp->numitems ) ? sp->mode[ n ] : 0;
}


/***************************************
 ***************************************/

void
fl_set_menu_popup( FL_OBJECT * ob,
				   int         pup )
{
#if FL_DEBUG >= ML_ERR
    if ( ! IsValidClass( ob, FL_MENU ) )
    {
		M_err( "setmenuPup", "%s is not Menu class", ob ? ob->label : "" );
		return;
    }
#endif

    ( ( FLI_MENU_SPEC * ) ob->spec )->extern_menu = pup;
    if ( ob->type == FL_PULLDOWN_MENU )
		fl_setpup_shadow( pup, 0 );
}


/***************************************
 ***************************************/

int
fl_set_menu_entries( FL_OBJECT *    ob,
					 FL_PUP_ENTRY * ent )
{
    int n;

    fl_clear_menu( ob );

    n = fl_newpup( 0 );
    fl_set_menu_popup( ob, fl_setpup_entries( n, ent ) );

    if ( ob->type == FL_PULLDOWN_MENU )
    {
		fl_setpup_bw( n, ob->bw );
		fl_setpup_shadow( n, 0 );
    }

    return n;
}


/***************************************
 ***************************************/

int
fl_get_menu_popup( FL_OBJECT * ob )
{
    FLI_MENU_SPEC *sp = ob->spec;

    return ISPUP( sp ) ? sp->extern_menu : -1;
}


/***************************************
 ***************************************/

int
fl_set_menu_notitle( FL_OBJECT * ob,
					 int         off )
{
    FLI_MENU_SPEC *sp = ob->spec;
    int old = sp->no_title;

    sp->no_title = off;
    return old;
}
