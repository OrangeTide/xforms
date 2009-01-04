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
 *
 * Copyright (c) 2009 Jens Thoms Toerring <jt@toerring.de>
 */


#if defined F_ID || defined DEBUG
char *fl_id_sel = "$Id: select.c,v 1.3 2009/01/04 00:45:22 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pselect.h"


static void timeout_cb( int, void * );
static int handle_select( FL_OBJECT *, int, FL_Coord, FL_Coord, int, void * );
static int handle_push( FL_OBJECT *, int );
static FL_POPUP_RETURN * find_first_item( FL_OBJECT * );
static FL_POPUP_RETURN * find_last_item( FL_OBJECT * );
static FL_POPUP_RETURN * find_next_item( FL_OBJECT *  );
static FL_POPUP_RETURN * find_prev_item( FL_OBJECT *  );
static void draw_select( FL_OBJECT * );
static void draw_droplist( FL_OBJECT * );


#define IS_ACTIVATABLE( e )                                               \
    (     ( e )->type != FL_POPUP_LINE                                    \
      && ! ( ( e ) ->state & ( FL_POPUP_HIDDEN | FL_POPUP_DISABLED ) ) )


/***************************************
 * Create a select object
 ***************************************/

FL_OBJECT *
fl_create_select( int          type,
				  FL_Coord     x,
				  FL_Coord     y,
				  FL_Coord     w,
				  FL_Coord     h,
				  const char * label )
{
    FL_OBJECT *obj;
    FLI_SELECT_SPEC *sp;

    obj = fl_make_object( FL_SELECT, type, x, y, w, h, label, handle_select );

	obj->boxtype = type == FL_NORMAL_SELECT ? FL_ROUNDED_BOX : FL_UP_BOX;

    obj->col1        = FL_SELECT_COL1;
    obj->col2        = FL_SELECT_COL2;
    obj->lcol        = FL_SELECT_LCOL;
    obj->align       = FL_SELECT_ALIGN;
	obj->want_update = 1;

    sp = obj->spec = fl_malloc( sizeof *sp );

	sp->popup      = NULL;
	sp->sel        = NULL;
	sp->title      = NULL;
    sp->align      = FL_ALIGN_CENTER;
	sp->lstyle     = FL_NORMAL_STYLE;
	sp->lsize      = FL_NORMAL_SIZE;
	sp->lcolor     = FL_BLACK;
	sp->timeout_id = -1;
	sp->repeat_ms  = 500;

	sp->p_bw = fl_popup_get_bw( NULL );
	fl_popup_get_title_font( NULL, &sp->p_title_font_style,
							 &sp->p_title_font_size );
	fl_popup_entry_get_font( NULL, &sp->p_entry_font_style,
							 &sp->p_entry_font_size );
	sp->p_bg_color = fl_popup_get_color( NULL, FL_POPUP_BACKGROUND_COLOR );
	sp->p_on_color = fl_popup_get_color( NULL, FL_POPUP_HIGHLIGHT_COLOR );
	sp->p_title_color = fl_popup_get_color( NULL, FL_POPUP_TITLE_COLOR );
	sp->p_text_color = fl_popup_get_color( NULL, FL_POPUP_TEXT_COLOR );
	sp->p_text_on_color =
		fl_popup_get_color( NULL, FL_POPUP_HIGHLIGHT_TEXT_COLOR );
	sp->p_text_off_color =
		fl_popup_get_color( NULL, FL_POPUP_DISABLED_TEXT_COLOR );
	sp->p_policy = fl_popup_get_policy( NULL );
	sp->p_min_width = 0;

    return obj;
}


/***************************************
 * Add a select object
 ***************************************/

FL_OBJECT *
fl_add_select( int          type,
			   FL_Coord     x,
			   FL_Coord     y,
			   FL_Coord     w,
			   FL_Coord     h,
			   const char * label )
{
    FL_OBJECT *obj;

    obj = fl_create_select( type, x, y, w, h, label );
    fl_add_object( fl_current_form, obj );

    return obj;
}


/***************************************
 * Remove the select objects popup
 ***************************************/

int
fl_clear_select( FL_OBJECT * obj )
{
	FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_clear_select_popup", "NULL object" );
		return -1;
	}

	sp = obj->spec;

	if ( sp->popup != NULL && fli_check_popup_exists( sp->popup ) != 1 )
	{
		fl_popup_delete( sp->popup );	
		sp->popup = NULL;
	}

	fl_redraw_object( obj );

	return 0;
}


/***************************************
 * Add item(s) to the select object
 ***************************************/

FL_POPUP_ENTRY *
fl_add_select_items( FL_OBJECT  * obj,
					 const char * items,
					 ... )
{
	FLI_SELECT_SPEC *sp;
	FL_POPUP_ENTRY *new_entries;
    va_list ap;

	if ( obj == NULL )
	{
		M_err( "fl_add_select_items", "NULL object" );
		return NULL;
	}

	sp = obj->spec;

	/* If no popup is yet associated with the object create one */

	if ( sp->popup == NULL )
	{
		if ( ( sp->popup = fli_popup_add( FL_ObjWin( obj ), sp->title,
										  "fl_add_select_items" ) ) == NULL )
			return NULL;

		fl_popup_set_bw( sp->popup, sp->p_bw );
		fl_popup_set_title_font( sp->popup, sp->p_title_font_style,
								 sp->p_title_font_size );
		fl_popup_entry_set_font( sp->popup, sp->p_entry_font_style,
								 sp->p_entry_font_size );
		
		fl_popup_set_color( sp->popup, FL_POPUP_BACKGROUND_COLOR,
							sp->p_bg_color );
		fl_popup_set_color( sp->popup, FL_POPUP_HIGHLIGHT_COLOR,
							sp->p_on_color );
		fl_popup_set_color( sp->popup, FL_POPUP_TITLE_COLOR,
							sp->p_title_color );
		fl_popup_set_color( sp->popup, FL_POPUP_TEXT_COLOR,
							sp->p_text_color );
		fl_popup_set_color( sp->popup, FL_POPUP_HIGHLIGHT_TEXT_COLOR,
							sp->p_text_on_color );
		fl_popup_set_color( sp->popup, FL_POPUP_DISABLED_TEXT_COLOR,
							sp->p_text_off_color );
		fl_popup_set_policy( sp->popup, sp->p_policy );
		if ( sp->p_min_width > 0 )
			fl_popup_set_min_width( sp->popup, sp->p_min_width );

		if ( sp->title && *sp->title )
			fl_popup_set_title( sp->popup, sp->title );
	}

	/* Create and append the new entries to the popup */

	va_start( ap, items );
	new_entries = fli_popup_add_entries( sp->popup, items, ap,
										 "fl_add_select_items", 1 );
	va_end( ap );

	/* If there's no currently selected entry try to find one */

	if ( sp->sel == NULL )
		sp->sel = find_first_item( obj );

	fl_redraw_object( obj );

	return new_entries;
}


/***************************************
 * Insert item(s) into the select object after an already existing item
 ***************************************/

FL_POPUP_ENTRY *
fl_insert_select_items( FL_OBJECT      * obj,
						FL_POPUP_ENTRY * after,
						const char     * items,
						... )
{
	FLI_SELECT_SPEC *sp;
	FL_POPUP_ENTRY *new_entries;
    va_list ap;

	if ( obj == NULL )
	{
		M_err( "fl_add_select_items", "NULL object" );
		return NULL;
	}

	sp = obj->spec;

	/* Check that a popup has been associated with the select object */

	if ( sp->popup == NULL )
	{
		M_err( "fl_insert_select_items", "No entries exist yet" );
		return NULL;
	}

	if ( after != NULL && fli_check_popup_entry_exists( after ) != 0 )
	{
		M_err( "fl_add_select_items", "Item to insert after doesn't exist" );
		return NULL;
	}

	va_start( ap, items );
	new_entries = fli_popup_insert_entries( sp->popup, after, items, ap,
											"fl_insert_select_items", 1 );
	va_end( ap );

	/* If there's no currently selected entry try to find one */

	if ( sp->sel == NULL )
		sp->sel = find_first_item( obj );

	fl_redraw_object( obj );

	return new_entries;
}


/***************************************
 * Replace an item by new item(s)
 ***************************************/

FL_POPUP_ENTRY *
fl_replace_select_item( FL_OBJECT      * obj,
						FL_POPUP_ENTRY * old_item,
						const char     * items,
						  ... )
{
	FLI_SELECT_SPEC *sp;
	FL_POPUP_ENTRY *new_entries;
    va_list ap;

	if ( obj == NULL )
	{
		M_err( "fl_replace_select_items", "NULL object" );
		return NULL;
	}

	if ( items == NULL || *items == '\0' )
	{
		M_err( "fl_replace_select_items", "Items string NULL or empty" );
		return NULL;
	}

	sp = obj->spec;

	/* Check that a popup has been associated with the select */

	if ( sp->popup == NULL )
	{
		M_err( "fl_replace_select_items", "No entries exist yet" );
		return NULL;
	}

	/* Test if the item we're supposed to replace exists */

	if ( fli_check_popup_entry_exists( old_item ) )
	{
		M_err( "fl_replace_select_items", "Item to replace doesn't exist" );
		return NULL;
	}

	/* Add the new item(s) after the one to replaced */

	va_start( ap, items );
	new_entries = fli_popup_insert_entries( sp->popup, old_item, items, ap,
											"fl_replace_select_items", 1 );
	va_end( ap );

	/* If the insert worked out ok delete the old item (and check if we
	   have to set a new displayed item) */

	if ( new_entries != NULL )
	{
		if ( sp->sel != NULL && sp->sel->entry == old_item )
			sp->sel = find_next_item( obj );

		fl_popup_entry_delete( old_item );

		if ( sp->sel->entry == old_item )
			sp->sel = find_first_item( obj );
	}
	else
		sp->sel = NULL;

	fl_redraw_object( obj );

	return new_entries;
}


/***************************************
 * Delete an item of a select object
 ***************************************/

int
fl_delete_select_item( FL_OBJECT      * obj,
					   FL_POPUP_ENTRY * item )
{
	FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_delete_select_item", "NULL object" );
		return -1;
	}

	sp = obj->spec;

	if ( sp->popup == NULL )
	{
		M_err( "fl_delete_select_item", "No popup defined yet" );
		return -1;
	}

	if ( fli_check_popup_entry_exists( item ) != 0 )
	{
		M_err( "fl_delete_select_item", "Item doesnt exist" );
		return -1;
	}

	/* Delete the entry */

	fl_popup_entry_delete( item );

	/* Check if we have to change the currently selected item */

	if ( item == sp->sel->entry )
		sp->sel = find_next_item( obj );

	fl_redraw_object( obj );

	return 0;
}
	

/***************************************
 * (Re)polulate a select objects popup via an array of FL_POPUP_ITEM structures
 ***************************************/

long
fl_set_select_items( FL_OBJECT     * obj,
					 FL_POPUP_ITEM * items )
{
	FLI_SELECT_SPEC *sp;
	FL_POPUP_ENTRY *e;
	long count;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_items", "NULL object" );
		return -1;
	}

	sp = obj->spec;

	/* If no popup is yet associated with the object create one */

	if ( sp->popup == NULL )
	{
		if ( ( sp->popup = fli_popup_add( FL_ObjWin( obj ), sp->title,
										  "fl_set_select_items" ) ) == NULL )
			return 01;

		fl_popup_set_bw( sp->popup, sp->p_bw );
		fl_popup_set_title_font( sp->popup, sp->p_title_font_style,
								 sp->p_title_font_size );
		fl_popup_entry_set_font( sp->popup, sp->p_entry_font_style,
								 sp->p_entry_font_size );
		
		fl_popup_set_color( sp->popup, FL_POPUP_BACKGROUND_COLOR,
							sp->p_bg_color );
		fl_popup_set_color( sp->popup, FL_POPUP_HIGHLIGHT_COLOR,
							sp->p_on_color );
		fl_popup_set_color( sp->popup, FL_POPUP_TITLE_COLOR,
							sp->p_title_color );
		fl_popup_set_color( sp->popup, FL_POPUP_TEXT_COLOR,
							sp->p_text_color );
		fl_popup_set_color( sp->popup, FL_POPUP_HIGHLIGHT_TEXT_COLOR,
							sp->p_text_on_color );
		fl_popup_set_color( sp->popup, FL_POPUP_DISABLED_TEXT_COLOR,
							sp->p_text_off_color );
		fl_popup_set_policy( sp->popup, sp->p_policy );
		if ( sp->p_min_width > 0 )
			fl_popup_set_min_width( sp->popup, sp->p_min_width );

		if ( sp->title && *sp->title )
			fl_popup_set_title( sp->popup, sp->title );
	}
	
	/* Remove all existing entries and reset the popups internal counter */

	while ( sp->popup->entries != NULL )
		fl_popup_entry_delete( sp->popup->entries );

	fli_popup_reset_counter( sp->popup );

	/* Now add the new ones */

	for ( count = 0; items && items->text != NULL; count++, items++ )
	{
		size_t i = 9;
		char *txt;
		char *t = ( char * ) items->text;

		/* Figure out how many chars we need fot the text */

		while ( ( t = strchr( t, '%' ) ) != NULL )
			if ( *++t != 'S' )
				i++;

		t = txt = fl_malloc( i );

		strcpy( txt, items->text );
		
		while ( ( t = strchr( t, '%' ) ) != NULL )
		{
			if ( *++t == 'S' )
				continue;
			memmove( t + 1, t, strlen( t ) + 1 );
			*t++ = '%';
		}

		if ( items->state & FL_POPUP_DISABLED )
			strcat( txt, "%d" );
		if ( items->state & FL_POPUP_HIDDEN )
			strcat( txt, "%h" );
		strcat( txt, "%f%s" );

		e = fl_popup_add_entries( sp->popup, txt, items->callback,
								  items->shortcut );

		fl_free( txt );

		fl_safe_free( e->text );
		e->text = fl_strdup( items->text );
	}

	if ( count > 0 )
		sp->sel = find_first_item( obj );

	return count;
}


/***************************************
 * Set a (new) popup for a select object
 ***************************************/

int
fl_set_select_popup( FL_OBJECT * obj,
					 FL_POPUP  * popup )
{
	FLI_SELECT_SPEC *sp;
	FL_POPUP *old_popup;
	FL_POPUP_ENTRY *e;

	/* We need a valid object */

	if ( obj == NULL )
	{
		M_err( "fl_set_select_popup", "NULL object" );
		return -1;
	}

	sp = obj->spec;

	/* The popup must exist */

	if ( fli_check_popup_exists( popup ) )
	{
		M_err( "fl_set_select_popup", "Popup doesn't exist" );
		return -1;
	}

	/* The popup can't be a sub-popup nor may it contain any entries that
	   are not "normal" entries */

	if ( popup->parent != NULL )
	{
		M_err( "fl_set_select_popup", "Popup is a sub-popup" );
		return -1;
	}

	for ( e = popup->entries; e != NULL; e = e->next )
		if ( e->type != FL_POPUP_NORMAL )
		{
			M_err( "fl_set_select_popup", "Invalid entries in popup" );
			return -1;
		}

	/* Delete a popup already associated with the select object */

	old_popup = ( ( FLI_SELECT_SPEC * ) obj->spec )->popup;

	if ( old_popup != NULL && fli_check_popup_exists( old_popup ) != 1 )
		fl_popup_delete( old_popup );

	/* Set the new popup as the select pbjects popup and redraw */

	sp->popup = popup;

	/* Set all popup properties as they are set for the popup */

	fl_safe_free( sp->title );
	if ( fl_popup_get_title( sp->popup ) != NULL )
		sp->title = fl_strdup( fl_popup_get_title( sp->popup ) );

	sp->p_bw = fl_popup_get_bw( sp->popup );
	fl_popup_get_title_font( sp->popup, &sp->p_title_font_style,
							 &sp->p_title_font_size );
	fl_popup_entry_get_font( sp->popup, &sp->p_entry_font_style,
							 &sp->p_entry_font_size );
	sp->p_bg_color = fl_popup_get_color( sp->popup, FL_POPUP_BACKGROUND_COLOR );
	sp->p_on_color = fl_popup_get_color( sp->popup, FL_POPUP_HIGHLIGHT_COLOR );
	sp->p_title_color = fl_popup_get_color( sp->popup, FL_POPUP_TITLE_COLOR );
	sp->p_text_color = fl_popup_get_color( sp->popup, FL_POPUP_TEXT_COLOR );
	sp->p_text_on_color =
		fl_popup_get_color( sp->popup, FL_POPUP_HIGHLIGHT_TEXT_COLOR );
	sp->p_text_off_color =
		fl_popup_get_color( sp->popup, FL_POPUP_DISABLED_TEXT_COLOR );
	sp->p_policy = fl_popup_get_policy( sp->popup );
	sp->p_min_width = fl_popup_get_min_width( sp->popup );

	sp->sel = find_first_item( obj );

	fl_redraw_object( obj );

	return 1;
}


/***************************************
 * Return currently selected item
 ***************************************/

FL_POPUP_RETURN *
fl_get_select_item( FL_OBJECT * obj )
{
	if ( obj == NULL )
	{
		M_err( "fl_get_select_item", "NULL object" );
		return NULL;
	}

	return ( ( FLI_SELECT_SPEC * ) obj->spec )->sel;
}


/***************************************
 * Return currently selected item
 ***************************************/

FL_POPUP_RETURN *
fl_set_select_item( FL_OBJECT      * obj,
					FL_POPUP_ENTRY * entry )
{
	FL_POPUP_ENTRY *e;
	FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_get_select_item", "NULL object" );
		return NULL;
	}

	if ( entry == NULL )
	{
		M_err( "fl_set_select_item", "NULL entry" );
		return NULL;
	}

	sp = obj->spec;

	for ( e = sp->popup->entries; e != NULL; e = e->next )
		if ( e == entry )
			break;

	if ( e == NULL )
	{
		M_err( "fl_set_select_item", "Entry does not exist" );
		return NULL;
	}

	if ( ! IS_ACTIVATABLE( entry ) )
	{
		M_err( "fl_set_select_item", "Entry can't be set as selected" );
		return NULL;
	}

	return fli_set_popup_return( entry );
}


/***************************************
 ***************************************/

FL_POPUP_ENTRY *
fl_get_select_item_by_value( FL_OBJECT * obj,
	                         long int    val )
{
	FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_get_select_item_by_value", "NULL object" );
		return NULL;
	}

	sp = obj->spec;

	if ( sp->popup == NULL )
		return NULL;

	return fl_popup_entry_get_by_value( sp->popup, val );
}


/***************************************
 ***************************************/

FL_POPUP_ENTRY *
fl_get_select_item_by_label( FL_OBJECT  * obj,
							 const char * label )
{
	FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_get_select_item_by_label", "NULL object" );
		return NULL;
	}

	sp = obj->spec;

	if ( sp->popup == NULL )
		return NULL;

	return fl_popup_entry_get_by_label( sp->popup, label );
}


/***************************************
 ***************************************/

FL_POPUP_ENTRY *
fl_get_select_item_by_text( FL_OBJECT  * obj,
							const char * text )
{
	FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_get_select_item_by_text", "NULL object" );
		return NULL;
	}

	sp = obj->spec;

	if ( sp->popup == NULL )
		return NULL;

	return fl_popup_entry_get_by_text( sp->popup, text );
}


/***************************************
 * Set a title for the popup of the select object
 ***************************************/

FL_OBJECT *
fl_set_select_popup_title( FL_OBJECT  * obj,
						   const char * title )
{
    FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_title", "NULL object" );
		return NULL;
	}

	sp = obj->spec;

	fl_safe_free( sp->title );

	if ( title == NULL || *title == '\0' )
		return obj;

	if ( ( sp->title = fl_strdup( title ) ) == NULL )
	{
		M_err( "fl_set_select_title", "Running out of memory" );
		return NULL;
	}

	if (    sp->popup != NULL
		 && fl_popup_set_title( sp->popup, sp->title ) == NULL )
	{
		fl_safe_free( sp->title );
		return NULL;
	}

	return obj;
}


/***************************************
 * Returns one of the different colors set for the object and its popup
 ***************************************/

FL_COLOR
fl_get_select_color( FL_OBJECT * obj,
					 int         type )
{
    FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_get_select_color", "NULL object" );
		return FL_MAX_COLORS;
	}

	sp = obj->spec;

	switch ( type )
	{
		case FL_SELECT_NORMAL_COLOR :
			return obj->col1;

		case FL_SELECT_HIGHLIGHT_COLOR :
			return obj->col2;

		case FL_SELECT_LABEL_COLOR :
			return obj->lcol;

		case FL_SELECT_TEXT_COLOR :
			return sp->lcolor;

		case FL_SELECT_POPUP_BACKGROUND_COLOR :
			return sp->p_bg_color;

		case FL_SELECT_POPUP_HIGHLIGHT_COLOR :
			return sp->p_on_color;

		case FL_SELECT_POPUP_TITLE_COLOR :
			return sp->p_title_color;

		case FL_SELECT_POPUP_TEXT_COLOR :
			return sp->p_text_color;

		case FL_SELECT_POPUP_HIGHLIGHT_TEXT_COLOR :
			return sp->p_text_on_color;

		case FL_SELECT_POPUP_DISABLED_TEXT_COLOR :
			return sp->p_text_off_color;
	}

	M_err( "fl_set_select_color", "Invalid type of color to set" );
	return FL_MAX_COLORS;
}


/***************************************
 * Set one of the different colors of the object
 ***************************************/

FL_COLOR
fl_set_select_color( FL_OBJECT * obj,
					 int         type,
					 FL_COLOR    color )
{
    FLI_SELECT_SPEC *sp;
	FL_COLOR old_color;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_color", "NULL object" );
		return FL_MAX_COLORS;
	}

    if ( color >= FL_MAX_COLORS )
    {
        M_err( "fl_select_set_color", "Invalid color argument" );
        return FL_MAX_COLORS;
    }

	sp = obj->spec;

	switch ( type )
	{
		case FL_SELECT_NORMAL_COLOR :
			old_color = obj->col1;
			fl_set_object_color( obj, color, obj->col2 );
			break;

		case FL_SELECT_HIGHLIGHT_COLOR :
			old_color = obj->col2;
			fl_set_object_color( obj, obj->col1, color );
			break;

		case FL_SELECT_LABEL_COLOR :
			old_color = obj->lcol;
			fl_set_object_lcol( obj, color );
			break;

		case FL_SELECT_TEXT_COLOR :
			old_color = sp->lcolor;
			sp->lcolor = color;
			fl_redraw_object( obj );
			break;

		case FL_SELECT_POPUP_BACKGROUND_COLOR :
			old_color = sp->p_bg_color;
			sp->p_bg_color = color;
			if ( sp->popup != NULL )
				fl_popup_set_color( sp->popup, FL_POPUP_BACKGROUND_COLOR,
									color );
			break;

		case FL_SELECT_POPUP_HIGHLIGHT_COLOR :
			old_color = sp->p_on_color;
			sp->p_on_color = color;
			if ( sp->popup != NULL )
				fl_popup_set_color( sp->popup, FL_POPUP_HIGHLIGHT_COLOR,
									color );
			break;

		case FL_SELECT_POPUP_TITLE_COLOR :
			old_color = sp->p_title_color;
			sp->p_title_color = color;
			if ( sp->popup != NULL )
				fl_popup_set_color( sp->popup, FL_POPUP_TITLE_COLOR,
									color );
			break;

		case FL_SELECT_POPUP_TEXT_COLOR :
			old_color = sp->p_text_color;
			sp->p_text_color = color;
			if ( sp->popup != NULL )
				fl_popup_set_color( sp->popup, FL_POPUP_TEXT_COLOR,
									color );
			break;

		case FL_SELECT_POPUP_HIGHLIGHT_TEXT_COLOR :
			old_color = sp->p_text_on_color;
			sp->p_text_on_color = color;
			if ( sp->popup != NULL )
				fl_popup_set_color( sp->popup, FL_POPUP_HIGHLIGHT_TEXT_COLOR,
									color );
			break;

		case FL_SELECT_POPUP_DISABLED_TEXT_COLOR :
			old_color = sp->p_text_off_color;
			sp->p_text_off_color = color;
			if ( sp->popup != NULL )
				fl_popup_set_color( sp->popup, FL_POPUP_DISABLED_TEXT_COLOR,
									color );
			break;

		default :
			M_err( "fl_set_select_color", "Invalid type of color to set" );
			return FL_MAX_COLORS;
	}

	return old_color;
}


/***************************************
 * Returns style and size of one of the fonts used for the object
 ***************************************/

int
fl_get_select_font( FL_OBJECT * obj,
	                int         type,
					int       * style,
					int       * size )
{
    FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_get_select_font", "NULL object" );
		return -1;
	}

	sp = obj->spec;

	switch ( type )
	{
		case FL_SELECT_TEXT_FONT :
			if ( style != NULL )
				*style = sp->lstyle;
			if ( size != NULL )
				*size = sp->lsize;
			break;

		case FL_SELECT_POPUP_TEXT_FONT :
			if ( style != NULL )
				*style = sp->p_title_font_style;
			if ( size != NULL )
				*size = sp->p_title_font_size;
			break;

		case FL_SELECT_ITEM_TEXT_FONT :
			if ( style != NULL )
				*style = sp->p_entry_font_style;
			if ( size != NULL )
				*size = sp->p_entry_font_size;
			break;

		case FL_SELECT_LABEL_FONT :
			if ( style != NULL )
				*style = obj->lstyle;
			if ( size != NULL )
				*size = obj->lsize;
			break;

		default :
			M_err( "fl_set_select_font", "Invalid type of font to set" );
			return -1;
	}

	return 0;
}


/***************************************
 * Sets style and size of one of the fonts used for the object
 ***************************************/

int
fl_set_select_font( FL_OBJECT * obj,
	                int         type,
					int         style,
					int         size )
{
    FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_font", "NULL object" );
		return -1;
	}

	sp = obj->spec;

	switch ( type )
	{
		case FL_SELECT_TEXT_FONT :
			sp->lstyle = style;
			sp->lsize  = size;
			fl_redraw_object( obj );
			break;

		case FL_SELECT_POPUP_TEXT_FONT :
			sp->p_title_font_style = style;
			sp->p_title_font_size  = size;
			if ( sp->popup != NULL )
				fl_popup_set_title_font( sp->popup, style, size );
			break;

		case FL_SELECT_ITEM_TEXT_FONT :
			sp->p_entry_font_style = style;
			sp->p_entry_font_size  = size;
			if ( sp->popup != NULL )
				fl_popup_entry_set_font( sp->popup, style, size );
			break;

		case FL_SELECT_LABEL_FONT :
			fl_set_object_lstyle( obj, style );
			fl_set_object_lsize( obj, size );
			break;

		default :
			M_err( "fl_set_select_font", "Invalid type of font to set" );
			return -1;
	}

	return 0;
}


/***************************************
 * Sets the alignment of the text within the box of the object
 * (that's not the label alignment!)
 ***************************************/

int
fl_set_select_text_align( FL_OBJECT * obj,
						  int         align )
{
    FLI_SELECT_SPEC *sp;
	int old_align;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_text_align", "NULL object" );
		return -1;
	}

	if ( ! ( align & FL_ALIGN_INSIDE ) )
	{
		M_warn( "fl_set_select_text_align", "Adding FL_ALIGN_INSIDE flag" );
		align |= FL_ALIGN_INSIDE;
	}

	if (    align < ( FL_ALIGN_CENTER | FL_ALIGN_INSIDE )
		 || align > ( FL_ALIGN_BOTTOM_RIGHT | FL_ALIGN_INSIDE ) )
	{
		M_err( "fl_set_select_text_align", "Invalid value for align" );
		return -1;
	}

	sp = obj->spec;
	
	old_align = sp->align;
	sp->align = align;
	fl_redraw_object( obj );

	return old_align;
}


/***************************************
 * Sets the border width to be used for the popup of the popup
 ***************************************/

int
fl_set_select_popup_bw( FL_OBJECT * obj,
						int         bw )
{
    FLI_SELECT_SPEC *sp;
    int old_bw;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_popup_bw", "NULL object" );
		return INT_MIN;
	}

    /* Clamp border width to a reasonable range */

    if ( bw == 0 || FL_abs( bw ) > FL_MAX_BW )
    {
        bw = bw == 0 ? -1 : ( bw > 0 ? FL_MAX_BW : - FL_MAX_BW );
        M_warn( "fl_set_select_popup_bw", "Adjusting invalid border width "
				"to %d", bw ); 
    }

	sp = obj->spec;

	old_bw = sp->p_bw;
	sp->p_bw = bw;
	if ( sp->popup )
		fl_popup_set_bw( sp->popup, bw );

	return old_bw;
}


/***************************************
 * Sets how the popup of the object behaves
 ***************************************/

int
fl_set_select_policy( FL_OBJECT * obj,
					  int         policy )
{
    FLI_SELECT_SPEC *sp;
    int old_policy;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_policy", "NULL object" );
		return INT_MIN;
	}

    if ( policy < FL_POPUP_NORMAL_SELECT || policy > FL_POPUP_DRAG_SELECT )
    {
        M_err( "fl_set_select_policy", "Invalid policy argument" );
        return -1;
    }

	sp = obj->spec;

	old_policy = sp->p_policy;
	sp->p_policy = policy;
	if ( sp->popup != NULL )
		fl_popup_set_policy( sp->popup, policy );

	return old_policy;
}

/***************************************
 * Returns the state of one of the items of the object
 ***************************************/

unsigned int
fl_get_select_item_state( FL_OBJECT      * obj,
						  FL_POPUP_ENTRY * item )
{
	FL_POPUP_ENTRY *e;
    FLI_SELECT_SPEC *sp;

	if ( obj == NULL )
	{
		M_err( "fl_get_select_item_state", "NULL object" );
		return UINT_MAX;
	}

	sp = obj->spec;

	if ( sp->popup == NULL )
	{
		M_err( "fl_get_select_item_state", "Object has no popup yet" );
		return UINT_MAX;
	}

	for ( e = sp->popup->entries; e != NULL; e = e->next )
		if ( e == item )
			break;

	if ( e == NULL )
	{
		M_err( "fl_get_select_item_state", "Invalid item" );
		return UINT_MAX;
	}

	return fl_popup_entry_get_state( item );
}


/***************************************
 * Sets the state of one of the items of the object
 ***************************************/

unsigned int
fl_set_select_item_state( FL_OBJECT      * obj,
						  FL_POPUP_ENTRY * item,
						  unsigned int     state )
{
	FL_POPUP_ENTRY *e;
    FLI_SELECT_SPEC *sp;
	unsigned int old_state;

	if ( obj == NULL )
	{
		M_err( "fl_set_select_item_state", "NULL object" );
		return UINT_MAX;
	}

	sp = obj->spec;

	if ( sp->popup == NULL )
	{
		M_err( "fl_set_select_item_state", "Object has no popup yet" );
		return UINT_MAX;
	}

	for ( e = sp->popup->entries; e != NULL; e = e->next )
		if ( e == item )
			break;

	if ( e == NULL )
	{
		M_err( "fl_set_select_item_state", "Invalid item" );
		return UINT_MAX;
	}

	/* Mask out bits that can't be set for a select item */

	state &= FL_POPUP_DISABLED | FL_POPUP_HIDDEN;

	/* Set the new state */

	old_state = fl_popup_entry_set_state( item, state );

	/* If the object we changed was the selected one before and, due to the
	   new state, it isn't selectable anymore, set a new selected item */

	if (    state & ( FL_POPUP_DISABLED | FL_POPUP_HIDDEN )
		 && sp->sel->entry == item )
		sp->sel = find_next_item( obj );

	return old_state;
}


/***************************************
 * Callback for timeout used when mouse buttons 2 or 3 are kept pressed down
 ***************************************/

static void
timeout_cb( int    val  FL_UNUSED_ARG,
			void * data )
{
    ( ( FLI_SELECT_SPEC * ) data )->timeout_id = -1;
}


/***************************************
 * Central routine for interaction with object
 ***************************************/

static int
handle_select( FL_OBJECT * obj,
			   int         event,
			   FL_Coord    mx   FL_UNUSED_ARG,
			   FL_Coord    my   FL_UNUSED_ARG,
			   int         key,
			   void *      ev   FL_UNUSED_ARG )
{
    FLI_SELECT_SPEC *sp = obj->spec;
	FL_POPUP_RETURN *ret = NULL;
	unsigned int w,
		         h;

    switch ( event )
	{
		case FL_DRAW :
			if ( obj->type != FL_DROPLIST_SELECT )
				draw_select( obj );
			else
				draw_droplist( obj );
			/* fall through */

		case FL_DRAWLABEL :
			obj->align &= ~ FL_ALIGN_INSIDE;
			fl_drw_text_beside( obj->align & ~ FL_ALIGN_INSIDE, obj->x, obj->y,
								obj->w, obj->h, obj->lcol, obj->lstyle,
								obj->lsize, obj->label );
			break;

		case FL_ENTER :
		case FL_LEAVE :
			fl_redraw_object( obj );
			break;

		case FL_SHORTCUT :
			obj->pushed = 1;
			key = FL_MBUTTON1;
			fl_popup_get_size( sp->popup, &w, &h );
			fl_popup_set_position( sp->popup,
								   obj->form->x + obj->x + ( obj->w - w ) / 2,
								   obj->form->y + obj->y + obj->h );
			/* fall through */

		case FL_PUSH :
			return handle_push( obj, key );

		case FL_RELEASE :
			if ( key != FL_MBUTTON2 && key != FL_MBUTTON3 )
				break;

			if ( sp->timeout_id != -1 )
			{
				fl_remove_timeout( sp->timeout_id );
				sp->timeout_id = -1;
			}

			fl_redraw_object( obj );
			break;

		case FL_UPDATE:
			if (    ( key == FL_MBUTTON2 || key == FL_MBUTTON3 )
				 && sp->timeout_id == -1 )
			{
				const FL_POPUP_ENTRY *old_entry = sp->sel ?
					                              sp->sel->entry : NULL;

				ret = ( key == FL_MBUTTON2 ?
						find_prev_item : find_next_item )( obj );
				if (    ret != NULL
					 && ret->entry != old_entry
					 && ret->entry->callback
					 && ret->entry->callback( ret ) == FL_IGNORE )
					ret = NULL;
				fl_redraw_object( obj );
				sp->timeout_id = fl_add_timeout( sp->repeat_ms, timeout_cb,
												 sp );
				return ret != NULL;
			}
			break;

		case FL_FREEMEM :
			if ( sp && sp->popup )
				fl_popup_delete( sp->popup );
			fl_safe_free( obj->spec );
			break;
	}

	return 0;
}


/***************************************
 * Deals with pushes on the select object
 ***************************************/

static int
handle_push( FL_OBJECT * obj,
			 int         key )
{
    FLI_SELECT_SPEC *sp = obj->spec;
	FL_POPUP_RETURN *ret = NULL;
	const FL_POPUP_ENTRY *old_entry = sp->sel ? sp->sel->entry : NULL;
	unsigned int w,
		         h;

	if ( key == FL_MBUTTON1 )
	{
		fl_redraw_object( obj );

		if ( obj->type == FL_DROPLIST_SELECT )
		{
			fl_popup_get_size( sp->popup, &w, &h );
			if ( obj->w >= 2 )
				fl_popup_set_position( sp->popup,
									   obj->form->x + obj->x + obj->w - w,
									   obj->form->y + obj->y + obj->h );
			else
				fl_popup_set_position( sp->popup,
									   obj->form->x + obj->x
									   + ( obj->w - w ) / 2,
									   obj->form->y + obj->y + obj->h );
		}

		if ( ( ret = fl_popup_do( sp->popup ) ) != NULL )
			sp->sel = ret;
		obj->pushed = 0;
	}
	else if ( key == FL_MBUTTON2 || key == FL_MBUTTON3 )
	{
		sp->timeout_id = fl_add_timeout( sp->repeat_ms, timeout_cb, sp );
		fl_redraw_object( obj );
		ret = ( key == FL_MBUTTON2 ?
				find_prev_item : find_next_item )( obj );
		if (    ret != NULL
			 && ret->entry != old_entry
			 && ret->entry->callback
			 && ret->entry->callback( ret ) == FL_IGNORE )
			ret = NULL;
	}
	else if ( key == FL_MBUTTON4 || key == FL_MBUTTON5 )
	{
		obj->pushed = 0;
		ret = ( key == FL_MBUTTON4 ?
				find_prev_item : find_next_item )( obj );
		if (    ret != NULL
			 && ret->entry != old_entry
			 && ret->entry->callback
			 && ret->entry->callback( ret ) == FL_IGNORE )
			ret = NULL;
	}
	fl_redraw_object( obj );
	return ret != NULL;
}


/***************************************
 * Returns the first "activatable" entry of a select object
 ***************************************/

static FL_POPUP_RETURN *
find_first_item( FL_OBJECT * obj )
{
	FL_POPUP_ENTRY *e = ( ( FLI_SELECT_SPEC * ) obj->spec )->popup->entries;

	for ( ; e != NULL; e = e->next )
		if ( IS_ACTIVATABLE( e ) )
			return fli_set_popup_return( e );

	return NULL;
}


/***************************************
 * Returns the last "activatable" entry of a select object
 ***************************************/

static FL_POPUP_RETURN *
find_last_item( FL_OBJECT * obj )
{
	FL_POPUP_ENTRY *ec = ( ( FLI_SELECT_SPEC * ) obj->spec )->popup->entries,
	               *e = ec->next;

	for ( ; e != NULL; e = e->next )
		if ( IS_ACTIVATABLE( e ) )
			ec = e;

	if ( ec != NULL )
		return fli_set_popup_return( ec );

	return NULL;
}


/***************************************
 * Returns the next "activatable" entry of a select object after the
 * currently selected entry
 ***************************************/

static FL_POPUP_RETURN *
find_next_item( FL_OBJECT * obj )
{
	FL_POPUP_ENTRY *e;
	FLI_SELECT_SPEC *sp = obj->spec;

	for ( e = sp->sel->entry->next; e != NULL; e = e->next )
		if ( IS_ACTIVATABLE( e ) )
			return fli_set_popup_return( e );

	return find_first_item( obj );;
}


/***************************************
 * Returns the previous "activatable" entry of a select object before the
 * currently selected entry
 ***************************************/

static FL_POPUP_RETURN *
find_prev_item( FL_OBJECT * obj )
{
	FL_POPUP_ENTRY *e;
	FLI_SELECT_SPEC *sp = obj->spec;

	for ( e = sp->sel->entry->prev; e != NULL; e = e->prev )
		if ( IS_ACTIVATABLE( e ) )
			return fli_set_popup_return( e );

	return find_last_item( obj );
}


/***************************************
 * Draws select objects of type FL_NORMAL_SELECT and FL_MENU_SELECT
 ***************************************/

static void
draw_select( FL_OBJECT * obj )
{
    FL_COLOR color;
    FLI_SELECT_SPEC *sp = obj->spec;
    int bw = FL_abs( obj->bw ) + ( obj->bw > 0 );
	int box_w = 0;

    color = ( obj->belowmouse && sp->popup ) ? obj->col2 : obj->col1;

	/* Draw the box of the object, possibly lowered if the object is pushed */

    if ( FL_IS_UPBOX( obj->boxtype ) && obj->pushed )
		fl_drw_box( FL_TO_DOWNBOX( obj->boxtype ), obj->x, obj->y, obj->w,
					obj->h, color, obj->bw );
	else
		fl_drw_box( obj->boxtype, obj->x, obj->y, obj->w, obj->h, color,
					obj->bw );

	/* The FL_MENU_SELECT type has a small raised box on the right hand
	   side */

	if ( obj->type == FL_MENU_SELECT )
    {
		int box_x,
			box_y,
		    box_h = FL_max( 6 + ( obj->bw > 0 ), 0.1 * obj->h ),
			box_bw = - FL_max( bw - ( obj->bw > 0 ), 1 );

		box_w = FL_max( 0.11 * obj->w, 13 );

		if (    box_w <= obj->w - 2 * bw
			 && box_h <= obj->h - 2 * bw
			 && box_w >= 2 * box_bw
			 && box_h >= 2 * box_bw )
		{
			box_x = obj->x + obj->w -box_w - bw - 2;
			box_y = obj->y + ( obj->h - box_h ) / 2;

			fl_drw_box( FL_UP_BOX, box_x, box_y, box_w, box_h, obj->col1,
						box_bw );

			box_w += 3;
		}
    }

    if ( sp->sel && sp->sel->label && *sp->sel->label )
    {
		fl_set_text_clipping( obj->x + bw, obj->y + bw,
							  obj->w - box_w - 2 * bw, obj->h - 2 * bw );
		fl_drw_text( sp->align, obj->x + bw, obj->y + bw,
					 obj->w - box_w - 2 * bw, obj->h - 2 * bw,
					 sp->lcolor, sp->lstyle, sp->lsize, sp->sel->label );
		fl_unset_text_clipping( );
    }
}


/***************************************
 * Draws select objects of type FL_DROPLIST_SELECT
 ***************************************/

static void
draw_droplist( FL_OBJECT * obj )
{
	int box_x = obj->x,
		box_y = obj->y,
		box_w,
		box_h,
		button_x,
		button_y,
		button_w,
		button_h;
    FL_COLOR color;
    FLI_SELECT_SPEC *sp = obj->spec;
    int bw = FL_abs( obj->bw ) + ( obj->bw > 0 );

	/* Calculate the size of the box with the arrow - if the object is
	   higher than wide place it below the text of the currently selected
	   item */

	if ( obj->w >= obj->h )
	{
		button_x = obj->x + obj->w - obj->h;
		button_y = obj->y;
		button_w = button_h = obj->h;
		box_w = obj->w - obj->h;
		box_h = obj->h;
	}
	else
	{
		button_x = obj->x;
		button_y = obj->y + obj->h - obj->w;
		button_w = button_h = obj->w;
		box_w = obj->w;
		box_h = obj->h - obj->w;
	}

    color = ( obj->belowmouse && sp->popup ) ? obj->col2 : obj->col1;

	/* Draw the box for the text of the selected item */

	fl_drw_box( obj->boxtype, box_x, box_y, box_w, box_h, obj->col1,
				obj->bw );

	/* Draw the box for the arrow button, possibly lowered if the object is
	   pushed */

    if ( FL_IS_UPBOX( obj->boxtype ) && obj->pushed )
		fl_drw_box( FL_TO_DOWNBOX( obj->boxtype ), button_x, button_y,
					button_w, button_h, color, obj->bw );
	else
		fl_drw_box( obj->boxtype, button_x, button_y, button_w, button_h,
					color, obj->bw );

	/* Draw the arrow */

    fl_drw_text( FL_ALIGN_CENTER, button_x + bw, button_y + bw,
				 button_w - 2 * bw, button_h - 2 * bw, sp->lcolor, 0, 0,
				 "@#2->" );

	/* Draw the text of the currently selected item */

    if ( sp->sel && sp->sel->label && *sp->sel->label )
    {
		fl_set_text_clipping( box_x + bw, box_y + bw,
							  box_w - 2 * bw, box_h - 2 * bw );
		fl_drw_text( sp->align, box_x + bw, box_y + bw,
					 box_w - 2 * bw, box_h - 2 * bw,
					 sp->lcolor, sp->lstyle, sp->lsize, sp->sel->label );
		fl_unset_text_clipping( );
    }
}
