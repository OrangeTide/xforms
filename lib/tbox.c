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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pbrowser.h"


#include <string.h>
#include <sys/types.h>
#include <stdlib.h>


#define TOP_MARGIN     1
#define RIGHT_MARGIN   2
#define BOTTOM_MARGIN  1
#define LEFT_MARGIN    3     /* must be at least 1 for selection box */


static int handle_tbox( FL_OBJECT *,
						int,
						FL_Coord,
						FL_Coord,
						int,
						void * );

static GC create_gc( FL_OBJECT *,
					 int,
					 int,
					 FL_COLOR,
					 int,
					 int,
					 int,
					 int  );

int tbox_do_not_redraw = 0;


/***************************************
 * Creates a new textbox
 ***************************************/

FL_OBJECT *
fli_create_tbox( int          type,
				 FL_Coord     x,
				 FL_Coord     y,
				 FL_Coord     w,
				 FL_Coord     h,
				 const char * label )
{
    FL_OBJECT *obj;
    FLI_TBOX_SPEC *sp;

    obj = fl_make_object( FL_TBOX, type, x, y, w, h, label, handle_tbox );
    obj->boxtype     = FLI_TBOX_BOXTYPE;
    obj->lcol        = FLI_TBOX_LCOL;
    obj->col1        = FLI_TBOX_COL1;
    obj->col2        = FLI_TBOX_COL2;
    obj->align       = FLI_TBOX_ALIGN;
    obj->wantkey     = FL_KEY_SPECIAL;
	obj->want_update = 0;
	obj->spec        = sp = fl_malloc( sizeof *sp );

	sp->attrib        = 1;
	sp->lines         = NULL;
	sp->num_lines     = 0;
	sp->callback      = NULL;
	sp->xoffset       = 0;
	sp->yoffset       = 0;
	sp->max_width     = 0;
	sp->max_height    = 0;
    sp->def_size      = fli_cntl.browserFontSize ?
		                fli_cntl.browserFontSize : FLI_TBOX_FONTSIZE;
    sp->def_style     = FL_NORMAL_STYLE;
	sp->def_align     = FL_ALIGN_LEFT;
	sp->def_col1      = obj->col1;
	sp->def_col2      = obj->col2;
	sp->def_lcol      = obj->lcol;
    sp->defaultGC     = None;
    sp->backgroundGC  = None;
    sp->selectGC      = None;
    sp->nonselectGC   = None;
    sp->bw_selectGC   = None;
	sp->specialkey    = '@';
	sp->select_line   = -1;
	sp->deselect_line = -1;
	sp->react_to_vert = sp->react_to_hori = 1;

    fl_set_object_dblbuffer( obj, 1 );

	/* Per default the object never gets returned, user must change that */

	fl_set_object_return( obj, FL_RETURN_NONE );

    return obj;
}


/***************************************
 * Deletes a line from the textbox
 ***************************************/

void
fli_tbox_delete_line( FL_OBJECT * obj,
					  int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int recalc_max_width = 0;
	int i;

	/* If line number is invalid do nothing */

	if ( line < 0 || line >= sp->num_lines )
		return;

	if ( sp->select_line == line )
		sp->select_line = -1;
	else if ( sp->select_line > line )
		sp->select_line--;

	if ( sp->deselect_line == line )
		sp->deselect_line = -1;
	else if ( sp->deselect_line > line )
		sp->deselect_line--;

	/* Check if recalculation of maximum line length is necessary */

	if ( sp->max_width == sp->lines[ line ]->w )
		recalc_max_width = 1;

	/* Set vertical position of following lines */

	for ( i = line + 1; i < sp->num_lines; i++ )
		sp->lines[ i ]->y -= sp->lines[ line ]->h;

	sp->max_height -= sp->lines[ line ]->h;

	/* Get rid of special GC for the line */

	if ( sp->lines[ line ]->specialGC )
	{
		XFreeGC( flx->display, sp->lines[ line ]->specialGC );
		sp->lines[ line ]->specialGC = None;
	}

	/* Deallocate memory for the text of the line to delete */

	fl_safe_free( sp->lines[ line ]->fulltext );

	/* Get rid of memory for the structure */

	fl_free( sp->lines[ line ] );

	/* Move pointers to following line structures  one up */

	if ( --sp->num_lines != line )
		memmove( sp->lines + line, sp->lines + line + 1,
				 ( sp->num_lines - line ) * sizeof *sp->lines );

	/* Reduce memory for array of structure pointers */

	sp->lines = fl_realloc( sp->lines, sp->num_lines * sizeof *sp->lines );

	/* If necessary find remaining longest line */

	if ( recalc_max_width )
	{
		sp->max_width = 0;
		for ( i = 0; i < sp->num_lines; i++ )
			sp->max_width = FL_max( sp->max_width, sp->lines[ i ]->w );
	}

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}


/***************************************
 * Inserts a line into the textbox
 ***************************************/

void
fli_tbox_insert_line( FL_OBJECT  * obj,
					  int          line,
					  const char * new_text )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	char *text;
	char *p;
	int done = 0;
	char *e;
	int is_bold = 0;
	int is_italic = 0;
	TBOX_LINE *tl;
	int i;

	/* Catch invalid 'line' argument */

	if ( line < 0 || ! new_text )
		return;

	/* If 'line' is too large correct that by appending to the end */

	if ( line >= sp->num_lines )
		line = sp->num_lines;

	/* Make sure the line marked as selected and deselected stay correct */

	if ( sp->select_line >= line )
		sp->select_line++;
	if ( sp->deselect_line >= line )
		sp->deselect_line++;

	/* Make a copy of the text of the line */

	p = text = strdup( new_text );

	/* Get memory for one more line */

	sp->lines = fl_realloc( sp->lines,
							++sp->num_lines * sizeof *sp->lines );

	/* If necessary move all following lines one down */

	if ( line < sp->num_lines - 1 )
		memmove( sp->lines + line + 1, sp->lines + line,
				 ( sp->num_lines - line - 1 ) * sizeof *sp->lines );

	sp->lines[ line ] = tl = fl_malloc( sizeof **sp->lines );

	/* Set up defaults for the line */

	tl->fulltext      = NULL;
	tl->text          = NULL;
	tl->len           = 0;
	tl->has_lf        = 0;
	tl->selected      = 0;
	tl->selectable    = 1;
	tl->is_separator  = 0;
	tl->is_underlined = 0;
	tl->x             = 0;
	tl->w             = 0;
	tl->h             = sp->def_size;
	tl->size          = sp->def_size;
	tl->style         = sp->def_style;
	tl->align         = sp->def_align;
	tl->color         = obj->lcol;
	tl->is_special    = 0;
	tl->specialGC     = None;

	/* Check for flags at the start of the line. When we're done 'p'
	   points to the start of the string to be shown in the textbox.*/

	while ( *p && *p == sp->specialkey && ! done )
	{
		if ( p[ 1 ] == sp->specialkey )
		{
				p += 1;
				done = 1;
				break;
		}

		switch ( p [ 1 ] )
		{
			case '\0' :
				done = 1;
				break;

			case 'h' :
				tl->size = FL_HUGE_SIZE;
				p += 2;
				break;

			case 'l' :
				tl->size = FL_LARGE_SIZE;
				p += 2;
				break;

			case 'm' :
				tl->size = FL_MEDIUM_SIZE;
				p += 2;
				break;

			case 's' :
				tl->size = FL_SMALL_SIZE;
				p += 2;
				break;;

			case 'L' :
				tl->size += 6;
				p += 2;
				break;

			case 'M' :
				tl->size += 4;
				p += 2;
				break;

			case 'S' :
				tl->size -= 2;
				p += 2;
				break;

			case 'b' :
				tl->style |= FL_BOLD_STYLE;
				is_bold = 1;
				p += 2;
				break;

			case 'i' :
				tl->style |= FL_ITALIC_STYLE;
				is_italic = 1;
				p += 2;
				break;

			case 'n' :
				tl->style = FL_NORMAL_STYLE;
				if ( is_bold )
					tl->style |= FL_BOLD_STYLE;
				if ( is_italic )
					tl->style |= FL_ITALIC_STYLE;
				p += 2;
				break;

			case 'f' :
				tl->style = FL_FIXED_STYLE;
				if ( is_bold )
					tl->style |= FL_BOLD_STYLE;
				if ( is_italic )
					tl->style |= FL_ITALIC_STYLE;
				p += 2;
				break;

			case 't' :
				tl->style = FL_TIMES_STYLE;
				if ( is_bold )
					tl->style |= FL_BOLD_STYLE;
				if ( is_italic )
					tl->style |= FL_ITALIC_STYLE;
				p += 2;
				break;

			case 'c' :
				tl->align = FL_ALIGN_CENTER;
				p += 2;
				break;

			case 'r' :
				tl->align = FL_ALIGN_RIGHT;
				p += 2;
				break;

			case '_' :
				tl->is_underlined = 1;
				p += 2;
				break;

			case '-' :
				sp->lines[ line ]->is_separator = 1;
				sp->lines[ line ]->selectable = 0;
				done = 1;
				break;

			case 'N' :
				sp->lines[ line ]->selectable = 0;
				tl->color = FL_INACTIVE;
				p += 2;
				break;

			case 'C' :
				tl->color = strtol( p + 2, &e, 10 );
				if ( e == p + 2 )
				{
					M_err( "fli_tbox_insert_line", "missing color" );
					p += 1;
					break;
				}

				if ( tl->color >= FL_MAX_COLS )
				{
					M_err( "fli_tbox_insert_line", "bad color %ld", tl->color );
					tl->color = obj->lcol;
				}
				p = e;
				break;

			default :
				M_err( "fli_tbox_insert_line", "bad flag %c", p[ 1 ] );
				p += 1;
				done = 1;
				break;
		}
	}

	tl->fulltext = text;
	if ( ! tl->is_separator )
		tl->text = p;
	else
	{
		tl->text = tl->fulltext + strlen( tl->fulltext );
		tl->has_lf = 1;
	}

	tl->len = strlen( tl->text );
	if ( tl->text[ tl->len - 1 ] == '\n' )
	{
		tl->len--;
		tl->has_lf = 1;
	}

	/* Figure out width and height of string */

	if ( ! tl->is_separator && *tl->text )
	{
		tl->w = fl_get_string_widthTAB( tl->style, tl->size,
										tl->text, tl->len );
		tl->h = fl_get_string_height( tl->style, tl->size,
									  tl->text, tl->len,
									  &tl->asc, &tl->desc );
	}
	else
	{
		tl->w = 0;
		tl->h = fl_get_string_height( tl->style, tl->size,
									  "X", 1, &tl->asc, &tl->desc );
	}

	/* If the new line is longer than all others we need to recalculate the
	   horizontal position of all lines that aren't left aligned */

	if ( tl->w > sp->max_width )
	{
		sp->max_width = tl->w;
		for ( i = 0; i < sp->num_lines; i++ )
			if ( sp->lines[ i ]->align == FL_ALIGN_CENTER )
				sp->lines[ i ]->x = ( sp->max_width - sp->lines[ i ]->w ) / 2;
			else if ( sp->lines[ i ]->align == FL_ALIGN_RIGHT )
				sp->lines[ i ]->x = sp->max_width - sp->lines[ i ]->w;
	}
	else
	{
		if ( tl->align == FL_ALIGN_CENTER )
			tl->x = ( sp->max_width - tl->w ) / 2;
		else if ( tl->align == FL_ALIGN_RIGHT )
			tl->x = sp->max_width - tl->w;
	}

	/* Calculate the vertical position of the line, shifting that of lines
	   that may come afterwards */

	if ( sp->num_lines == 1 )
		tl->y = 0;
	else if ( line == sp->num_lines - 1 )
		tl->y = sp->lines[ line - 1 ]->y + sp->lines[ line - 1 ]->h;
	else
	{
		tl->y = sp->lines[ line + 1 ]->y;
		for ( i = line + 1; i < sp->num_lines; i++ )
			sp->lines[ i ]->y += tl->h;
	}

	sp->max_height += tl->h;

	/* Set flag if the line isn't to be drawn in default style, size and
	   color. We don't create a GC yet since this might be called before
	   the textbox is visible! */

	if (    tl->style != sp->def_style
		 || tl->size  != sp->def_size
		 || ( tl->color != obj->lcol && tl->selectable ) )
		tl->is_special = 1;

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}


/***************************************
 * Appends a line to the end of the textbox
 ***************************************/

void
fli_tbox_add_line( FL_OBJECT  * obj,
				   const char * text )
{
   FLI_TBOX_SPEC *sp = obj->spec;

   fli_tbox_insert_line( obj, sp->num_lines, text );
}


/***************************************
 * Appends characters to the last line in the textbox
 ***************************************/

void
fli_tbox_add_chars( FL_OBJECT  * obj,
					const char * add )
{
	FLI_TBOX_SPEC *sp = obj->spec;
	TBOX_LINE *tl;
	int new_len;
	char *old_fulltext;
	char * old_text;
	int i;

	/* If there's nothing to add return */

	if ( ! add || ! *add )
		return;

	/* If there aren't any lines yet or of the last line ended in a line feed
	   this is equivalent to inserting a new one */

	if ( sp->num_lines == 0 || sp->lines[ sp->num_lines - 1 ]->has_lf )
	{
		fli_tbox_insert_line( obj, sp->num_lines, add );
		return;
	}

	tl = sp->lines[ sp->num_lines - 1 ];

	/* Make up the new text of the line from the old and the new text */

	new_len = strlen( tl->fulltext ) + strlen( add );
	old_text = tl->text;
	old_fulltext = tl->fulltext;

	tl->fulltext = fl_malloc( new_len + 1 );
	strcpy( tl->fulltext, old_fulltext );
	strcat( tl->fulltext, add );
	tl->text = tl->fulltext + ( old_text - old_fulltext );
	tl->len = new_len;

	if ( tl->text[ tl->len - 1 ] == '\n' )
	{
		tl->len--;
		tl->has_lf = 1;
	}

	fl_safe_free( old_fulltext );

	/* Text of a separator line never gets shown */

	if ( tl->is_separator )
		return;

	/* Figure out the new length of the line */

	if ( *tl->text )
		tl->w = fl_get_string_widthTAB( tl->style, tl->size,
										tl->text, tl->len );

	/* If line is now longer than all others we need to recalculate the
	   horizontal position of all lines that aren't left aligned */

	if ( tl->w > sp->max_width )
	{
		sp->max_width = tl->w;
		for ( i = 0; i < sp->num_lines; i++ )
			if ( sp->lines[ i ]->align == FL_ALIGN_CENTER )
				sp->lines[ i ]->x = ( sp->max_width - sp->lines[ i ]->w ) / 2;
			else if ( sp->lines[ i ]->align == FL_ALIGN_RIGHT )
				sp->lines[ i ]->x = sp->max_width - sp->lines[ i ]->w;
	}
	else
	{
		if ( tl->align == FL_ALIGN_CENTER )
			tl->x = ( sp->max_width - tl->w ) / 2;
		else if ( tl->align == FL_ALIGN_RIGHT )
			tl->x = sp->max_width - tl->w;
	}

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}



/***************************************
 * Replaces a line in the textbox
 ***************************************/

void
fli_tbox_replace_line( FL_OBJECT  * obj,
					   int          line,
					   const char * text )
{
   FLI_TBOX_SPEC *sp = obj->spec;
   int old_select_line = sp->select_line;

   if ( line < 0 || line >= sp->num_lines || ! text )
	   return;

   tbox_do_not_redraw = 1;
   fli_tbox_delete_line( obj, line );
   fli_tbox_insert_line( obj, line, text );
   if ( line == old_select_line && sp->lines[ line ]->selectable )
	   fli_tbox_select_line( obj, line );
}


/***************************************
 * Removes all lines from the textbox
 ***************************************/

void
fli_tbox_clear( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	FLI_BROWSER_SPEC *br = obj->parent->spec;
    int i;


	sp->select_line = sp->deselect_line = -1;

    if ( sp->num_lines == 0 )
		return;

    for ( i = 0; i < sp->num_lines; i++ )
    {
		if ( sp->lines[ i ]->specialGC )
		{
			XFreeGC( flx->display, sp->lines[ i ]->specialGC );
			sp->lines[ i ]->specialGC = None;
		}
		fl_safe_free( sp->lines[ i ]->fulltext );
		fl_safe_free( sp->lines[ i ] );
	}

	fl_safe_free( sp->lines );

    sp->num_lines  = 0;
    sp->max_width  = 0;
    sp->max_height = 0;
	sp->xoffset    = 0;
	sp->yoffset    = 0;

	fl_set_scrollbar_value( br->vsl, 0.0 );
	fl_set_scrollbar_value( br->hsl, 0.0 );

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}



/***************************************
 * Loads all lines from a file into the textbox
 ***************************************/

int
fli_tbox_load( FL_OBJECT  * obj,
			   const char * filename )
{
    FLI_TBOX_SPEC *sp = obj->spec;
    FILE *fp;
    char *text;

    fli_tbox_clear( obj );

    /* Load the file */

    if ( ! filename || ! *filename )
		return 0;

    if ( ! ( fp = fopen( filename, "r" ) ) )
		return 0;

	while ( ( text = fli_read_line( fp ) ) && *text != '\0' )
	{
		tbox_do_not_redraw = 1;
		fli_tbox_insert_line( obj, sp->num_lines, text );
		fl_free( text );
	}

	fl_safe_free( text );

    fclose( fp );

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;

    return 1;
}


/***************************************
 * Returns the text of a line in the textbox (including flags)
 ***************************************/

const char *
fli_tbox_get_line( FL_OBJECT * obj,
				   int         line )
{
   FLI_TBOX_SPEC *sp = obj->spec;

   if ( line < 0 || line >= sp->num_lines )
	   return NULL;

   return sp->lines[ line ]->fulltext;
}


/***************************************
 * Sets a new font size for all lines drawn with default settings
 ***************************************/

void
fli_tbox_set_fontsize( FL_OBJECT * obj,
					   int         size )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	double old_xrel;
	double old_yrel;
	int i;

	if ( size < FL_TINY_SIZE || size > FL_HUGE_SIZE )
		return;

	sp->def_size = size;

	sp->attrib = 1;

	if ( sp->num_lines == 0 )
		return;

	old_xrel = fli_tbox_get_rel_xoffset( obj );
	old_yrel = fli_tbox_get_rel_yoffset( obj );

	/* Calculate width and height for all lines */

	for ( i = 0; i < sp->num_lines; i++ )
	{
		TBOX_LINE *tl = sp->lines[ i ];

		if ( tl->is_special )
			continue;

		tl->size = size;

		/* Figure out width and height of string */

		if ( ! tl->is_separator && *tl->text )
		{
			tl->w = fl_get_string_widthTAB( tl->style, tl->size,
											tl->text, tl->len );
			tl->h = fl_get_string_height( tl->style, tl->size,
										  tl->text, tl->len,
										  &tl->asc, &tl->desc );
		}
		else
		{
			tl->w = 0;
			tl->h = fl_get_string_height( tl->style, tl->size,
										  "X", 1, &tl->asc, &tl->desc );
		}
	}

	/* Calculate vertical positions of all lines and max width */

	sp->max_width = sp->lines[ 0 ]->w;

	for ( i = 1; i < sp->num_lines; i++ )
	{
		sp->lines[ i ]->y = sp->lines[ i - 1 ]->y + sp->lines[ i - 1 ]->h;
		sp->max_width = FL_max( sp->max_width, sp->lines[ i ]->w );
	}

	/* Determine new height of all text */

	sp->max_height =   sp->lines[ sp->num_lines - 1 ]->y
		             + sp->lines[ sp->num_lines - 1 ]->h;

	tbox_do_not_redraw = 1;
	fli_tbox_set_rel_xoffset( obj, old_xrel );
	tbox_do_not_redraw = 1;
	fli_tbox_set_rel_yoffset( obj, old_yrel );
}


/***************************************
 * Sets a new font style for all lines drawn with default settings
 ***************************************/

void
fli_tbox_set_fontstyle( FL_OBJECT * obj,
						int         style )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	double old_xrel;
	double old_yrel;
	int i;

	if ( style < FL_NORMAL_STYLE || style > FL_TIMESBOLDITALIC_STYLE )
		return;

	sp->def_style = style;

	sp->attrib = 1;

	if ( sp->num_lines == 0 )
		return;

	old_xrel = fli_tbox_get_rel_xoffset( obj );
	old_yrel = fli_tbox_get_rel_yoffset( obj );

	/* Calculate width and height for all lines */

	for ( i = 0; i < sp->num_lines; i++ )
	{
		TBOX_LINE *tl = sp->lines[ i ];

		if ( tl->is_special )
			continue;

		tl->style = style;

		/* Figure out width and height of string */

		if ( ! tl->is_separator && *tl->text )
		{
			tl->w = fl_get_string_widthTAB( tl->style, tl->size,
											tl->text, tl->len );
			tl->h = fl_get_string_height( tl->style, tl->size,
										  tl->text, tl->len,
										  &tl->asc, &tl->desc );
		}
		else
		{
			tl->w = 0;
			tl->h = fl_get_string_height( tl->style, tl->size,
										  "X", 1, &tl->asc, &tl->desc );
		}
	}

	/* Calculate vertical positions of all lines and the width of the longest
	   line */

	sp->max_width = sp->lines[ 0 ]->w;

	for ( i = 1; i < sp->num_lines; i++ )
	{
		sp->lines[ i ]->y = sp->lines[ i - 1 ]->y + sp->lines[ i - 1 ]->h;
		sp->max_width = FL_max( sp->max_width, sp->lines[ i ]->w );
	}

	/* Determine new height of total text */

	sp->max_height =   sp->lines[ sp->num_lines - 1 ]->y
		             + sp->lines[ sp->num_lines - 1 ]->h;

	sp->attrib = 1;

	tbox_do_not_redraw = 1;
	fli_tbox_set_rel_xoffset( obj, old_xrel );
	tbox_do_not_redraw = 1;
	fli_tbox_set_rel_yoffset( obj, old_yrel );
}


/***************************************
 ***************************************/

int
fli_tbox_set_xoffset( FL_OBJECT * obj,
					  int         pixel )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( sp->max_width <= sp->w || pixel < 0 )
		pixel = 0;
	if ( pixel > sp->max_width - sp->w )
		pixel = FL_max( 0, sp->max_width - sp->w );

	sp->xoffset = pixel;

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;

	return pixel;
}


/***************************************
 ***************************************/

double
fli_tbox_set_rel_xoffset( FL_OBJECT * obj,
						  double      offset )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( sp->max_width <= sp->w || offset < 0.0 )
		offset = 0.0;
	if ( offset > 1.0 )
		offset = 1.0;

	sp->xoffset = FL_nint( offset * FL_max( 0, sp->max_width - sp->w ) );

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;

	return fli_tbox_get_rel_xoffset( obj );
}
	

/***************************************
 ***************************************/

int
fli_tbox_set_yoffset( FL_OBJECT * obj,
					  int         pixel )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( sp->max_height <= sp->h || pixel < 0 )
		pixel = 0;
	if ( pixel > sp->max_height - sp->h )
		pixel = FL_max( 0, sp->max_height - sp->h );

	sp->yoffset = pixel;

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;

	return pixel;
}


/***************************************
 ***************************************/

double
fli_tbox_set_rel_yoffset( FL_OBJECT * obj,
						  double      offset )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( sp->max_height <= sp->h || offset < 0.0 )
		offset = 0.0;
	if ( offset > 1.0 )
		offset = 1.0;

	sp->yoffset = FL_nint( offset * FL_max( 0, sp->max_height - sp->h ) );

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;

	return fli_tbox_get_rel_yoffset( obj );
}


/***************************************
 ***************************************/

int
fli_tbox_get_xoffset( FL_OBJECT * obj )
{
    return ( ( FLI_TBOX_SPEC * ) obj->spec )->xoffset;
}


/***************************************
 ***************************************/

double
fli_tbox_get_rel_xoffset( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( sp->max_width <= sp->w )
		return 0.0;

	return ( double ) sp->xoffset / ( sp->max_width - sp->w );
}


/***************************************
 ***************************************/

int
fli_tbox_get_yoffset( FL_OBJECT * obj )
{
    return ( ( FLI_TBOX_SPEC * ) obj->spec )->yoffset;
}


/***************************************
 ***************************************/

double
fli_tbox_get_rel_yoffset( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( sp->max_height <= sp->h )
		return 0.0;

	return ( double ) sp->yoffset / ( sp->max_height - sp->h );
}


/***************************************
 ***************************************/

int
fli_tbox_get_line_yoffset( FL_OBJECT * obj,
						   int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( line < 0 || line >= sp->num_lines )
		return -1;

	return sp->lines[ line ]->y;
}


/***************************************
 * Makes a line the topmost shown line (as far as possible)
 ***************************************/

void
fli_tbox_set_topline( FL_OBJECT * obj,
					  int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( line < 0 )
		line = 0;
	else if ( line >= sp->num_lines )
		line = sp->num_lines - 1;

	fli_tbox_set_yoffset( obj, sp->lines[ line ]->y );
}


/***************************************
 * Makes a line the lowest shown line (as far as possible)
 ***************************************/

void
fli_tbox_set_bottomline( FL_OBJECT * obj,
						 int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( line < 0 )
		line = 0;
	else if ( line >= sp->num_lines )
		line = sp->num_lines - 1;

	fli_tbox_set_yoffset( obj,
						  sp->lines[ line ]->y + sp->lines[ line ]->h - sp->h );
}


/***************************************
 * Makes a line shown in the center of the browser (as far as possible)
 ***************************************/

void
fli_tbox_set_centerline( FL_OBJECT * obj,
						 int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( line < 0 )
		line = 0;
	else if ( line >= sp->num_lines )
		line = sp->num_lines - 1;

	fli_tbox_set_yoffset( obj,
						    sp->lines[ line ]->y
						  + ( sp->lines[ line ]->h - sp->h ) / 2 );
}


/***************************************
 * Removes all selections in the browser
 ***************************************/

 void
 fli_tbox_deselect( FL_OBJECT * obj )
 {
	 FLI_TBOX_SPEC *sp = obj->spec;
	int i;

	for ( i = 0; i < sp->num_lines; i++ )
		sp->lines[ i ]->selected = 0;

	sp->select_line = -1;
	sp->deselect_line = -1;

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}


/***************************************
 * Deselects a line in the browser
 ***************************************/

 void
 fli_tbox_deselect_line( FL_OBJECT * obj,
						 int         line )
 {
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( line < 0 || line >= sp->num_lines || ! sp->lines[ line ]->selected )
		return;

	sp->lines[ line ]->selected = 0;

	/* Don't mark as deselected for FL_SELECT_BROWSER since otherwise it
	   would be impossible for the user to retrieve the selection */

	if ( obj->type != FL_SELECT_BROWSER )
	{
		sp->deselect_line = line;
		sp->select_line = -1;
	}

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}


/***************************************
 * Selects a line in the browser (if necessary deselecting another line)
 ***************************************/

 void
 fli_tbox_select_line( FL_OBJECT * obj,
					   int         line )
 {
    FLI_TBOX_SPEC *sp = obj->spec;

	if (    line < 0
		 || line >= sp->num_lines
		 || sp->lines[ line ]->selected
		 || ! sp->lines[ line ]->selectable )
		return;

	if ( sp->select_line != -1 && obj->type != FL_MULTI_BROWSER )
		sp->lines[ sp->select_line ]->selected = 0;

	sp->lines[ line ]->selected = 1;

	sp->select_line = line;
	sp->deselect_line = -1;

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}


/***************************************
 * Returns if a line in the browser is selected
 ***************************************/

int
fli_tbox_is_line_selected( FL_OBJECT * obj,
						   int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	return    line >= 0
		   && line < sp->num_lines
		   && sp->lines[ line ]->selected;
}


/***************************************
 * Sets if a line is selectable or not
 ***************************************/

void
fli_tbox_make_line_selectable( FL_OBJECT * obj,
							   int         line,
							   int         state )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	TBOX_LINE *tl;

    if (    line < 0
		 || line >= sp->num_lines
		 || sp->lines[ line ]->is_separator
		 || obj->type == FL_NORMAL_BROWSER )
		return;

	tl = sp->lines[ line ];
	state = state ? 1 : 0;

	if ( ! state )
	{
		if ( line == sp->select_line )
			sp->select_line = -1;
		if ( line == sp->deselect_line )
			sp->deselect_line = -1;
	}

	if ( tl->selectable != state )
	{
		tl->selectable = state;

		if ( tl->is_special )
		{
			if ( tl->specialGC )
			{
				XFreeGC( flx->display, tl->specialGC );
				sp->lines[ line ]->specialGC = None;
			}

			if ( FL_ObjWin( obj ) )
				tl->specialGC = create_gc( obj, tl->style, tl->size,
										   state ? obj->lcol : FL_INACTIVE,
										   sp->x, sp->y, sp->w, sp->h );
		}
	}

	if ( ! tbox_do_not_redraw )
		fl_redraw_object( obj );
	else
		tbox_do_not_redraw = 0;
}


/***************************************
 * Returns the last selected or deselected line (or 0 if there's none).
 * Please note: this function returns the index of the selected line
 * incremented by one and the negative of the index of the deselected
 * line decremented by 1.
 ***************************************/

int
fli_tbox_get_selection( FL_OBJECT *obj )
{
	FLI_TBOX_SPEC *sp = obj->spec;

	if ( sp->select_line >= 0 )
		return sp->select_line + 1;
	else if ( sp->deselect_line >= 0 )
		return - sp->deselect_line - 1;
	else
		return 0;
}


/***************************************
 * Installs a handler for double and triple clicks
 ***************************************/

void
fli_tbox_set_dblclick_callback( FL_OBJECT      * obj,
								FL_CALLBACKPTR   cb,
								long             data )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	sp->callback = cb;
	sp->callback_data = data;
    fl_set_object_dblclick( obj, cb ? FL_CLICK_TIMEOUT : 0 );
}
	

/***************************************
 ***************************************/

static GC
create_gc( FL_OBJECT * obj,
		   int         style,
		   int         size,
		   FL_COLOR    color,
		   int         clip_x,
		   int         clip_y,
		   int         clip_w,
		   int         clip_h )
{
	GC gc;
	XGCValues xgcv;
	unsigned long gcvm;

	if ( fli_cntl.safe )
		xgcv.graphics_exposures = 1;
	else
	{
		Screen *scr = ScreenOfDisplay( flx->display, fl_screen );

		xgcv.graphics_exposures =    ! DoesBackingStore( scr )
			                      || ! fli_cntl.backingStore;
	}

	gcvm = GCGraphicsExposures | GCForeground;

	xgcv.foreground = fl_get_flcolor( color );
	gc = XCreateGC( flx->display, FL_ObjWin( obj ), gcvm, &xgcv );

	if ( size > 0 && style >= 0 )
	{
		XFontStruct *xfs = fl_get_fntstruct( style, size );

		XSetFont( flx->display, gc, xfs->fid );
	}

	fl_set_gc_clipping( gc, obj->x + clip_x, obj->y + clip_y, clip_w, clip_h );

	return gc;
}


/***************************************
 ***************************************/

void
fli_tbox_recalc_area( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int dummy;

    sp->x = FL_abs( obj->bw ) + LEFT_MARGIN;
    sp->y = FL_abs( obj->bw ) + TOP_MARGIN;
    sp->w = obj->w - 2 * FL_abs( obj->bw ) - LEFT_MARGIN - RIGHT_MARGIN;
    sp->h = obj->h - 2 * FL_abs( obj->bw ) - TOP_MARGIN - BOTTOM_MARGIN;

	/* This is necessary because different box types haven't all the same
	   inside size but will look still wrong with anything but up and down
	   boxes... */

	if ( obj->boxtype == FL_UP_BOX )
	{
		sp->x++;
		sp->y++;
		sp->w -= 2;
		sp->h -= 2;
	}

	/* Calculate height of line with default font */

	sp->def_height = fl_get_string_height( sp->def_style, sp->def_size,
										   "X", 1, &dummy, &dummy );
}


/***************************************
 ***************************************/

static void
fli_tbox_prepare_drawing( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int i;
	double old_xrel = fli_tbox_get_rel_xoffset( obj );
	double old_yrel = fli_tbox_get_rel_yoffset( obj );

	fli_tbox_recalc_area( obj );

	/* Recalculate horizontal positions of all lines */

	for ( i = 0; i < sp->num_lines; i++ )
		if ( sp->lines[ i ]->align == FL_ALIGN_CENTER )
			sp->lines[ i ]->x = ( sp->max_width - sp->lines[ i ]->w ) / 2;
		else if ( sp->lines[ i ]->align == FL_ALIGN_RIGHT )
			sp->lines[ i ]->x = sp->max_width - sp->lines[ i ]->w;

	/* We might get called before the textbox is shown and then the
	   window is still unknown and GCs can't be created */

	if ( ! FL_ObjWin( obj ) )
		return;

	/* Create default GC for text drawing */

	if ( sp->defaultGC )
		XFreeGC( flx->display, sp->defaultGC );

	sp->defaultGC = create_gc( obj, sp->def_style, sp->def_size, sp->def_lcol,
							   sp->x, sp->y, sp->w, sp->h );

	/* Create background GC for redraw deselected lines */

	if ( sp->backgroundGC )
		XFreeGC( flx->display, sp->backgroundGC );

	sp->backgroundGC = create_gc( obj, -1, 0, sp->def_col1,
								  sp->x - ( LEFT_MARGIN > 0 ),
								  sp->y, sp->w + ( LEFT_MARGIN > 0 ), sp->h );

	/* Create select GC for marking selected lines */

	if ( sp->selectGC )
		XFreeGC( flx->display, sp->selectGC );

	sp->selectGC = create_gc( obj, -1, 0,
							  fli_dithered( fl_vmode ) ?
							  FL_BLACK : sp->def_col2,
							  sp->x - ( LEFT_MARGIN > 0 ), sp->y,
							  sp->w + ( LEFT_MARGIN > 0 ), sp->h );

	/* Create GC for text of non-selectable lines */

	if ( sp->nonselectGC )
		XFreeGC( flx->display, sp->nonselectGC );

	sp->nonselectGC = create_gc( obj, sp->def_style, sp->def_size, FL_INACTIVE,
								 sp->x, sp->y, sp->w, sp->h );

	/* Special GC for text of selected lines in B&W */

	if ( fli_dithered( fl_vmode ) )
	{
		if ( sp->bw_selectGC )
			XFreeGC( flx->display, sp->bw_selectGC );

		sp->bw_selectGC = create_gc( obj, sp->def_style, sp->def_size, FL_WHITE,
									 sp->x - ( LEFT_MARGIN > 0 ), sp->y,
									 sp->w + ( LEFT_MARGIN > 0 ), sp->h );
	}

	/* Lines with non-default fonts or colors have their own GCs */

	for ( i = 0; i < sp->num_lines;  i++ )
	{
		TBOX_LINE *tl = sp->lines[ i ];

		if ( ! tl->is_special )
			continue;

		if ( tl->specialGC )
		{
			XFreeGC( flx->display, tl->specialGC );
			tl->specialGC = None;
		}

		tl->specialGC = create_gc( obj, tl->style, tl->size, tl->color,
								   sp->x, sp->y, sp->w, sp->h );
	}

	tbox_do_not_redraw = 1;
	fli_tbox_set_rel_xoffset( obj, old_xrel );
	tbox_do_not_redraw = 1;
	fli_tbox_set_rel_yoffset( obj, old_yrel );
}


/***************************************
 * Frees all resources needed for the FLI_TBOX_SPEC structure
 ***************************************/

static void
free_tbox_spec( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int i;

	for ( i = 0; i < sp->num_lines; i++ )
	{
		if ( sp->lines[ i ]->specialGC )
			XFreeGC( flx->display, sp->lines[ i ]->specialGC );

		fl_safe_free( sp->lines[ i ]->fulltext );
		fl_safe_free( sp->lines[ i ] );
	}

	if ( sp->defaultGC )
		XFreeGC( flx->display, sp->defaultGC );

	if ( sp->backgroundGC )
		XFreeGC( flx->display, sp->backgroundGC );

	if ( sp->selectGC )
		XFreeGC( flx->display, sp->selectGC );

	if ( sp->nonselectGC )
		XFreeGC( flx->display, sp->nonselectGC );

	if ( sp->bw_selectGC )
		XFreeGC( flx->display, sp->bw_selectGC );

	fl_safe_free( obj->spec );
}


/***************************************
 * Draws a line of text
 ***************************************/

static void
draw_tboxline( FL_OBJECT * obj,
			   int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	GC activeGC = sp->defaultGC;
	TBOX_LINE *tl;

	/* Nothing to be drawn for illegal line numbers */

	if (    line < 0
		 || line >= sp->num_lines )
	{
		M_err( "draw_tboxline", "Invalid line number" );
		return;
	}

	/* Also nothing to be done if line isn't visible */

	tl = sp->lines[ line ];

	if (    tl->y >= sp->h + sp->yoffset
		 || tl->y + tl->h < sp->yoffset )
		return;

	/* Draw the background, either the normal one or the one for selected
	   lines */

	XFillRectangle( flx->display, FL_ObjWin( obj ),
					tl->selected ? sp->selectGC : sp->backgroundGC,
					obj->x + sp->x - ( LEFT_MARGIN > 0 ),
					obj->y + sp->y + tl->y - sp->yoffset,
					sp->w + ( LEFT_MARGIN > 0 ), tl->h );

	/* Separator lines obviously are to be treated differently from normal
	   text */

	if ( tl->is_separator )
	{
		/* The extra horizontal pixels here are due to the function called
		   subtracting them! */

		fl_drw_text( 0, obj->x + sp->x - 3,
					 obj->y + sp->y - sp->yoffset + tl->y + tl->h / 2,
					 sp->w + 6, 1,
					 FL_COL1, FL_NORMAL_STYLE, sp->def_size, "@DnLine" );
		return;
	}

	/* If there's no text or the text isn't within the textbox nothing needs
	   to be drawn */

	if (    ! *tl->text
		 || tl->x - sp->xoffset >= sp->w
		 || tl->x + tl->w - sp->xoffset < 0 )
		return;

	/* If the line needs a different font or color than the default use
	   a special GC just for that line */

	if ( ! tl->selectable )
		activeGC = sp->nonselectGC;

    if ( tl->is_special )
	{
		if ( ! tl->specialGC )
			tl->specialGC = create_gc( obj, tl->style, tl->size,
									   tl->selectable ? tl->color : FL_INACTIVE,
									   sp->x, sp->y, sp->w, sp->h );

		activeGC = tl->specialGC;
	}

    /* Set up GC for selected lines in B&W each time round - a bit slow,
	   but I guess there are hardly any machines left with a B&W display */

    if ( fli_dithered( fl_vmode ) && tl->selected )
    {
		XFontStruct *xfs = fl_get_fntstruct( tl->style, tl->size );

		XSetFont( flx->display, sp->bw_selectGC, xfs->fid );
		XSetForeground( flx->display, sp->bw_selectGC,
						fl_get_flcolor( FL_WHITE ) );
		activeGC = sp->bw_selectGC;
    }

	/* Now draw the line, underlined if necessary */

	if ( tl->is_underlined )
		fl_diagline( obj->x + sp->x - sp->xoffset + tl->x,
					 obj->y + sp->y - sp->yoffset + tl->y + tl->h - 1,
					 FL_min( sp->w + sp->xoffset - tl->x, tl->w ), 1,
					 ( fli_dithered( fl_vmode ) && tl->selected ) ?
					 FL_WHITE : tl->color );

    fli_drw_stringTAB( FL_ObjWin( obj ), activeGC,
					   obj->x + sp->x - sp->xoffset + tl->x,
					   obj->y + sp->y - sp->yoffset + tl->y + tl->asc,
					   tl->style, tl->size, tl->text, tl->len, 0 );
}


/***************************************
 * Tries to find the index of the next selectable line following
 * 'line', returns the total number of lines if none can be found.
 ***************************************/

int find_next_selectable( FL_OBJECT * obj,
						  int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( line < -1 || line >= sp->num_lines )
		line = -1;

	while ( ++line < sp->num_lines )
		if ( sp->lines[ line ]->selectable )
			break;

	return line < sp->num_lines ? line : -1;
}


/***************************************
 * Tries to find the index of the next selectable line
 * before 'line', returns -1 if none can be found.
 ***************************************/

int find_previous_selectable( FL_OBJECT * obj,
							  int         line )
{
    FLI_TBOX_SPEC *sp = obj->spec;

	if ( line < 0 || line > sp->num_lines )
		line = sp->num_lines;

	while ( --line >= 0 )
		if ( sp->lines[ line ]->selectable )
			break;

	return line;
}


/***************************************
 * Returns the index of the first line that's completete shown on the screen
 ***************************************/

int
fli_tbox_get_topline( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int i = sp->yoffset / sp->def_height;   /* guess index of topline */

	if ( sp->lines[ i ]->y < sp->yoffset )
		while (    ++i < sp->num_lines
				&& sp->lines[ i ]->y < sp->yoffset )
			/* empty */ ;
	else if ( sp->lines[ i ]->y > sp->yoffset )
		while (    sp->num_lines && sp->lines[ i - 1 ]->y > sp->yoffset
				&& i-- > 0  )
			/* empty */ ;

	return i < sp->num_lines ? i : -1;
}


/***************************************
 * Returns the index of the last line that's completete shown on the screen
 ***************************************/

int
fli_tbox_get_bottomline( FL_OBJECT * obj )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int i = sp->num_lines;

	while ( --i >= 0
			&& sp->lines[ i ]->y + sp->lines[ i ]->h > sp->yoffset+ sp->h )
		/* empty */ ;

	return i;
}


/***************************************
 * Deals with keyboard input (and indirectly with mouse wheel "clicks")
 ***************************************/

static int
handle_keyboard( FL_OBJECT * obj,
				 int         key )
{
    FLI_TBOX_SPEC *sp = obj->spec;
    int old_select_line = sp->select_line;
	int old_yoffset = sp->yoffset;
	int old_xoffset = sp->xoffset;
	int ret = FL_RETURN_NONE;

    if ( IsHome( key ) && sp->react_to_vert )
		fli_tbox_set_rel_yoffset( obj, 0.0 );
    else if ( IsEnd( key ) && sp->react_to_vert )
		fli_tbox_set_rel_yoffset( obj, 1.0 );
    else if ( IsPageUp( key ) && sp->react_to_vert )
		fli_tbox_set_yoffset( obj, sp->yoffset - sp->h );
    else if ( IsHalfPageUp( key ) && sp->react_to_vert )
		fli_tbox_set_yoffset( obj, sp->yoffset - sp->h / 2 );
    else if ( Is1LineUp( key ) && sp->react_to_vert )
		fli_tbox_set_yoffset( obj, sp->yoffset - sp->def_height );
	else if ( ( IsPageDown( key ) || key == ' ' ) && sp->react_to_vert )
		fli_tbox_set_yoffset( obj, sp->yoffset + sp->h );
    else if ( IsHalfPageDown( key ) && sp->react_to_vert )
		fli_tbox_set_yoffset( obj, sp->yoffset + sp->h / 2 );
    else if ( Is1LineDown( key ) && sp->react_to_vert )
		fli_tbox_set_yoffset( obj, sp->yoffset + sp->def_height );
    else if ( IsLeft( key ) && sp->react_to_hori )
		fli_tbox_set_xoffset( obj, sp->xoffset - 3 );
    else if ( IsRight( key ) && sp->react_to_hori )
		fli_tbox_set_xoffset( obj, sp->xoffset + 3 );
    else if ( IsUp( key ) )
	{
		if (    sp->react_to_vert
			 && (    obj->type == FL_NORMAL_BROWSER
				  || obj->type == FL_SELECT_BROWSER
				  || obj->type == FL_MULTI_BROWSER ) )
		{
			int topline = fli_tbox_get_topline( obj );

			if ( --topline >= 0 )
				fli_tbox_set_yoffset( obj, sp->lines[ topline ]->y );
		}
		else if ( obj->type == FL_HOLD_BROWSER )
		{
			TBOX_LINE *tl;
			int line = find_previous_selectable( obj, sp->select_line );

			if ( line >= 0 )
			{
				tl = sp->lines[ line ];

				if ( sp->react_to_vert
					 || ( tl->y + tl->h >= sp->yoffset
						  && tl->y < sp->h + sp->yoffset ) )
				{
					fli_tbox_select_line( obj, line );

					tl = sp->lines[ sp->select_line ];

					/* Bring the selection into view if necessary */

					if ( tl->y < sp->yoffset )
						fli_tbox_set_topline( obj, sp->select_line );
					else if ( tl->y + tl->h - sp->yoffset >= sp->h )
						fli_tbox_set_bottomline( obj, sp->select_line );
				}
			}
		}
    }
    else if ( IsDown( key ) )
	{
		if (    sp->react_to_vert
			 && (    obj->type == FL_NORMAL_BROWSER
				  || obj->type == FL_SELECT_BROWSER
				  || obj->type == FL_MULTI_BROWSER ) )
		{
			int topline = fli_tbox_get_topline( obj );

			if ( topline < sp->num_lines - 1 )
			{
				if ( sp->lines[ topline ]->y - sp->yoffset == 0 )
					topline++;

				fli_tbox_set_yoffset( obj, sp->lines[ topline ]->y );
			}
			else
				fli_tbox_set_yoffset( obj, sp->max_height );
		}
		else if ( obj->type == FL_HOLD_BROWSER )
		{
			TBOX_LINE *tl;
			int line = find_next_selectable( obj, sp->select_line );

			if ( line >= 0 )
			{
				tl = sp->lines[ line ];

				if ( sp->react_to_vert
					 || ( tl->y + tl->h >= sp->yoffset
						  && tl->y < sp->h + sp->yoffset ) )
				{
					fli_tbox_select_line( obj, line );

					tl = sp->lines[ sp->select_line ];

					/* Bring the selection into view if necessary */

					if ( tl->y + tl->h < sp->yoffset )
						fli_tbox_set_topline( obj, sp->select_line );
					else if ( tl->y + tl->h - sp->yoffset >= sp->h )
						fli_tbox_set_bottomline( obj, sp->select_line );
				}
			}
		}
    }

	if ( old_select_line != sp->select_line )
		ret |= FL_RETURN_SELECTION;
	if ( old_yoffset != sp->yoffset || old_xoffset != sp->xoffset )
		ret |= FL_RETURN_CHANGED | FL_RETURN_END;

    return ret;
}


/***************************************
 * Mouse wheel hack - the (release) event is converted
 * to a key event (function is also used by input.c)
 ***************************************/

int
fli_handle_mouse_wheel( int       * ev,
						int       * key,
						void      * xev )
{
    if ( *ev == FL_PUSH )
        return 0;

	if ( *ev == FL_RELEASE )
	{
		*ev = FL_KEYBOARD;

		if ( xev && shiftkey_down( ( ( XButtonEvent * ) xev )->state ) )
		{
			( ( XButtonEvent * ) xev )->state &= ~ ShiftMask;
			( ( XKeyEvent * ) xev )->state &= ~ ShiftMask;
			*key = *key == FL_MBUTTON4 ? FLI_1LINE_UP : FLI_1LINE_DOWN;
		}
		else if ( xev && controlkey_down( ( ( XButtonEvent * ) xev )->state ) )
		{
			( ( XButtonEvent * ) xev )->state &= ~ ControlMask;
			( ( XKeyEvent * ) xev )->state &= ~ ControlMask;
			*key = *key == FL_MBUTTON4 ? XK_Prior : XK_Next;
		}
		else
			*key = *key == FL_MBUTTON4 ? FLI_HALFPAGE_UP : FLI_HALFPAGE_DOWN;
	}

    return 1;
}


/***************************************
 * Tries to find the index of the line under the mouse,
 * returns -1 if there's none
 ***************************************/

static int
find_mouse_line( FL_OBJECT * obj,
				 FL_Coord    my )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int line;

	if ( my < obj->y + sp->y || my > obj->y + sp->y + sp->h )
		return -1;

	my += sp->yoffset - sp->y - obj->y;

	line = FL_min( sp->num_lines - 1, 
				   obj->y / ( ( double ) sp->max_height / sp->num_lines ) );

	if ( sp->lines[ line ]->y > my )
		while ( line-- > 0
				&& sp->lines[ line ]->y > my )
			/* empty */ ;
	else
		while ( sp->lines[ line ]->y + sp->lines[ line ]->h < my
			    && ++line < sp->num_lines )
			/* empty */ ;

	if ( line < 0 || line >= sp->num_lines )
		return -1;

	return line;
}


/***************************************
 ***************************************/

void
fli_tbox_react_to_vert( FL_OBJECT * obj,
						int         state )
{
    ( ( FLI_TBOX_SPEC * ) obj->spec )->react_to_vert = state ? 1 : 0;
}


/***************************************
 ***************************************/

void
fli_tbox_react_to_hori( FL_OBJECT * obj,
						int         state )
{
    ( ( FLI_TBOX_SPEC * ) obj->spec )->react_to_hori = state ? 1 : 0;
}


/***************************************
 * Handles a mouse change, returns whether a selection change has occured
 ***************************************/

#define DESELECT 0
#define SELECT   1

static int
handle_mouse( FL_OBJECT * obj,
			  FL_Coord    my,
			  int         ev )
{
    FLI_TBOX_SPEC *sp = obj->spec;
    int line;
	int ret = FL_RETURN_NONE;
	static int mode;
	static int last_multi = -1;

    /* Check whether there are any lines at all */

    if ( sp->num_lines == 0 )
		return ret;

	/* Figure out the index of the line the mouse is on, if mouse us below
	   or above the text area scroll up or down */

	if (    ev == FL_UPDATE
		 && sp->react_to_vert
		 && (    my < obj->y + sp->y
			  || my > obj->y + sp->y + sp-> h ) )
	{
		if ( my < obj->y + sp->y )
		{
			line = fli_tbox_get_topline( obj );
			if ( line > 0 )
			{
				fli_tbox_set_topline( obj, --line );
				ret |= FL_RETURN_CHANGED;
			}
		}
		else
		{
			line = fli_tbox_get_bottomline( obj );
			if ( line > 0 && line < sp->num_lines - 1 )
			{
				fli_tbox_set_bottomline( obj, ++line );
				ret |= FL_RETURN_CHANGED;
			}
		}
	}
	else if ( obj->type != FL_NORMAL_BROWSER )		
		line = find_mouse_line( obj, my );

	/* A normal textbox doesn't react to the mouse otherwise */

	if ( obj->type == FL_NORMAL_BROWSER )
		return ret;
	else if (    obj->type == FL_SELECT_BROWSER
		 || obj->type == FL_HOLD_BROWSER )
	{
		if ( line < 0 || ! sp->lines[ line ]->selectable )
			return ret;

		/* For FL_SELECT_BROWSER browsers the selection is undone when the
		   mouse is released */

		if ( ev == FL_RELEASE && obj->type == FL_SELECT_BROWSER )
		{
			if ( sp->select_line >= 0 )
				fli_tbox_deselect_line( obj, sp->select_line );
			return ret;
		}

		if ( line < 0 )
			return ret;

		if ( ev == FL_PUSH )
		{
			if ( ! sp->lines[ line ]->selected )
			{
				fli_tbox_select_line( obj, line );
				ret |= FL_RETURN_SELECTION;
			}
		}
		else  if ( line >= 0 && line != sp->select_line )	
		{
			fli_tbox_select_line( obj, line );
			ret |= FL_RETURN_SELECTION;
		}

		return ret;
	}
	else  /* FL_MULTI_BROWSER */
	{
		if ( line < 0 )
			return ret;

		if ( ev == FL_PUSH )
		{
			if ( ! sp->lines[ line ]->selectable )
				return ret;

			mode = sp->lines[ line ]->selected ? DESELECT : SELECT;

			if ( mode == SELECT )
			{
				fli_tbox_select_line( obj, line );
				last_multi = line;
				ret |= FL_RETURN_SELECTION;
			}
			else
			{
				fli_tbox_deselect_line( obj, line );
				last_multi = line;
				ret |= FL_RETURN_DESELECTION;
			}
		}
		else
		{
			/* Mouse may have been moved that fast that one or more lines
			   got skipped */

			if ( last_multi != -1 && FL_abs( line - last_multi ) > 1 )
			{
				int incr = line - last_multi > 1 ? 1 : -1;

				while ( ( last_multi += incr ) != line )
					if ( sp->lines[ last_multi ]->selectable )
					{
						if (    mode == SELECT
							 && ! sp->lines[ last_multi ]->selected )
						{
							fli_tbox_select_line( obj, last_multi );
							ret |= FL_RETURN_SELECTION;
						}
						else if (    mode == DESELECT
								  && sp->lines[ last_multi ]->selected )
						{
							fli_tbox_deselect_line( obj, last_multi );
							ret |= FL_RETURN_DESELECTION;
						}
					}
			}

			if ( sp->lines[ line ]->selectable )
			{
				if (    mode == SELECT
					 && ! sp->lines[ line ]->selected )
				{
					fli_tbox_select_line( obj, line );
					last_multi = line;
					ret |= FL_RETURN_SELECTION;
				}
				else if (    mode == DESELECT
						  && sp->lines[ line ]->selected )
				{
					fli_tbox_deselect_line( obj, line );
					ret |= FL_RETURN_DESELECTION;
					last_multi = line;
				}
			}

			if ( ev == FL_RELEASE )
				last_multi = -1;
		}
	}

	return ret;
}


/***************************************
 * Handles the textbox
 ***************************************/

static int
handle_tbox( FL_OBJECT * obj,
			 int         ev,
			 FL_Coord    mx  FL_UNUSED_ARG,
			 FL_Coord    my,
			 int         key,
			 void      * xev )
{
    FLI_TBOX_SPEC *sp = obj->spec;
	int ret = FL_RETURN_NONE;
	int i;
	static int old_yoffset = -1;

    /* Mouse wheel hack */

	if (    ( key == FL_MBUTTON4 || key == FL_MBUTTON5 )
		 && ! obj->want_update
		 && ! fli_handle_mouse_wheel( &ev, &key, xev ) )
		return ret;

    switch ( ev )
    {
		case FL_ATTRIB :
		case FL_RESIZED :
			sp->attrib = 1;
			break;

		case FL_DRAW :
			if ( sp->attrib )
			{
				fli_tbox_prepare_drawing( obj );
				sp->attrib = 0;
			}

			fl_drw_box( obj->boxtype, obj->x, obj->y, obj->w, obj->h,
						sp->def_col1, obj->bw );

			for ( i = 0; i < sp->num_lines; i++ )
				draw_tboxline( obj, i );
			break;

		case FL_DBLCLICK :
		case FL_TRPLCLICK :
			if ( sp->callback )
				sp->callback( obj, sp->callback_data );
			break;

		case FL_KEYBOARD :
			ret = handle_keyboard( obj, key );
			break;

		case FL_PUSH :
			if ( key != FL_MBUTTON1 )
				break;
			obj->want_update = 1;
			old_yoffset = sp->yoffset;
			ret |= handle_mouse( obj, my, ev );
			break;

		case FL_UPDATE :
			ret |= handle_mouse( obj, my, ev );
			break;

		case FL_RELEASE :
			ret |= handle_mouse( obj, my, ev ) | FL_RETURN_END;
			if ( sp->yoffset != old_yoffset )
				ret |= FL_RETURN_CHANGED;
			obj->want_update = 0;
			break;

		case FL_FREEMEM :
			free_tbox_spec( obj );
			break;
	}

	return ret;
}
