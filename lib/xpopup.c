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
 * \file xpopup.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao
 *  All rights reserved.
 *.
 *
 *
 * Implementation of pop-up menus in Xlib. Not quite fit the
 * model of forms library, but it is needed to make other things
 * work.
 *
 * These functionalities should be someday rewritten using
 * forms construct rather than Xlib.
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_xpup = "$Id: xpopup.c,v 1.25 2008/05/09 12:33:02 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/flsnprintf.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>


#define ALWAYSROOT 1		     /* true to use root as parent. not working */

#define USE_SHADOW 0             /* shadows never really did work... */

#define FL_MAXPUP  32		     /* default maximum pups */
#define PADH       FL_PUP_PADH	 /* space between items */
#define PADW       8	         /* space on each side */
#define PADTITLE   14	         /* extran space for title  */
#define CHECKW     6	         /* check box size */

#define M_TITLE    1
#define M_ERR      2



/****************************************************************
 * pop up menu structure and some defaults
 ****************************************************************/

#define NSC          8		/* max hotkeys   */

typedef struct
{
    char         * str;				/* label               */
    FL_PUP_CB      icb;		    	/* call back           */
    long         * shortcut;		/* shortcut keys       */
    int            subm;			/* sub menu            */
    unsigned int   mode;			/* various attributes  */
    int            ret;				/* %x stuff            */
    short          ulpos;			/* hotkeys in label    */
    short          radio;			/* radio entry. 0 mean no radio */
    short          len;
} MenuItem;


typedef struct
{
    char           * title;		    /* Menu title            */
    Window           win;			/* menu window           */
    Window           parent_win;	/* and its parent        */
    Cursor           cursor;		/* cursor for the pup    */
#if USE_SHADOW
    GC               gc_shadow;		/* GC for the shadow     */
#endif
    GC               gc_active;		/* GC for maintext       */
    GC               gc_inactive;	/* GC for inactive text  */
    MenuItem       * item[ FL_MAXPUPI + 1 ];
    FL_PUP_CB        menu_cb;	   	/* call back routine     */
    FL_PUP_ENTERCB   enter_cb;	    /* enter callback routine */
    void *           enter_data;
    FL_PUP_ENTERCB   leave_cb;	    /* enter callback routine */
    void *           leave_data;
    unsigned long    event_mask;
    int              x,			    /* origin relative to root */
	                 y;
    unsigned int     w,		        /* total dimension       */
	                 h;
    int              win_x,
	                 win_y;
    short            titleh;
    short            nitems;		/* no. of item in menu   */
    short            title_width;	/* title width           */
    short            maxw;
#if USE_SHADOW
    short            noshadow;
	short            shade;
#endif
    short            bw;
    short            lpad;
    short            rpad;
    short            padh;
    short            cellh;
    short            isEntry;		/* true if menu is setup via entry struct */
	int              par_y;
} PopUP;


static void grab_both( PopUP * );
static void reset_radio( PopUP *,
						 MenuItem * );

/* Resources that control the fontsize and other things */

static int pup_font_style = FL_BOLDITALIC_STYLE;
static int pup_title_font_style = FL_BOLDITALIC_STYLE;

#ifdef __sgi
static int pup_font_size = FL_SMALL_FONT,
           pup_title_font_size = FL_SMALL_FONT;
#else
static int pup_font_size = FL_NORMAL_FONT,
           pup_title_font_size = FL_NORMAL_FONT;
#endif

static FL_COLOR pup_color         = FL_COL1;
static FL_COLOR pup_text_color    = FL_BLACK;
static FL_COLOR pup_checked_color = FL_BLUE;

static int pup_level = 0;
static int fl_maxpup = FL_MAXPUP;

static int pup_bw = -2;
static int pup_bw_is_set = 0;

static PopUP *menu_rec = NULL;

static XFontStruct *pup_font_struct = NULL;		   /* popup main text font */
static int pup_ascent = 0,			               /* font properties */
           pup_desc = 0;
static XFontStruct *pup_title_font_struct = NULL;  /* popup title text font */
static int pup_title_ascent = 0,
           pup_title_desc   = 0;
static Cursor pup_defcursor = 0;

Window fl_popup_parent_window = None;
FL_FORM *fl_popup_form = NULL;

static int pup_subreturn;

static int pup_using_keys = 0;
static int pup_internal_showpup_call = 0;

extern FL_Coord fli_mousex,	                    /* defined in forms.c */
                fli_mousey;
extern unsigned int fli_keymask;
extern unsigned int fli_query_age;


/************ data struct maintanance ******************{**/

/***************************************
 ***************************************/

static void
init_pupfont( void )
{
    XCharStruct chs;
    int junk;

    if ( ! pup_title_font_struct )
    {
		pup_title_font_struct = fl_get_fntstruct( pup_title_font_style,
												  pup_title_font_size );
		XTextExtents( pup_title_font_struct, "qjQb", 4, &junk,
					  &pup_title_ascent, &pup_title_desc, &chs );
    }

    if ( ! pup_font_struct )
    {
		pup_font_struct = fl_get_fntstruct( pup_font_style, pup_font_size );
		XTextExtents( pup_font_struct, "qjQb", 4, &junk, &pup_ascent,
					  &pup_desc, &chs );
    }
}


/***************************************
 * initialize a particular menu
 ***************************************/

static void
init_pup( PopUP * m )
{
    m->menu_cb = NULL;
    m->enter_cb = m->leave_cb = NULL;
    m->w = m->h = m->maxw = 0;
    m->nitems = 0;
	m->title_width = 0;
    m->parent_win = m->win = None;
    m->gc_active = m->gc_inactive = None;
#if USE_SHADOW
    m->noshadow = 0;
	m->shade = 2 * FL_abs( pup_bw );
#endif
	m->bw = pup_bw;
    m->title = NULL;
    m->item[ 0 ] = NULL;
    m->padh = PADH;
    if ( ! pup_defcursor )
		pup_defcursor = fl_get_cursor_byname( XC_sb_right_arrow );
    m->cursor = pup_defcursor;
    m->lpad = m->rpad = PADW;
    init_pupfont( );
    m->cellh = pup_ascent + pup_desc + 2 * m->padh;
    m->isEntry = 0;
}


/***************************************
 ***************************************/

static int
find_empty_index( Window win )
{
    PopUP *p;

    for ( p = menu_rec; p < menu_rec + fl_maxpup; p++ )
		if ( ! p->title && ! *p->item && ! p->parent_win )
		{
			init_pup( p );
			p->parent_win = win;
			return p - menu_rec;
		}

    M_err( "fl_newpup", "Too many popups (maximum is %d)", fl_maxpup );

    return -1;
}


static void convert_shortcut( const char *,
							  const char *,
							  MenuItem *,
							  int );

static void wait_for_close( Window );


/***************************************
 ***************************************/

static void
reset_max_width( PopUP * m )
{
    int i;
    MenuItem **item = m->item;
    char *t,
		 *b;


    if ( ! m->parent_win || m->nitems <= 0 )
		return;

	m->maxw = 0;

    for ( i = 0; i < m->nitems; i++ )
	{
		b = t = fl_strdup( item[ i ]->str );
		while ( ( b = strchr( b, '\b' ) ) )
			memmove( b, b + 1, strlen( b ) );
		m->maxw = FL_max( m->maxw,
						  fl_get_string_widthTAB( pup_font_style, pup_font_size,
												  t, strlen( t ) ) );
		fl_free( t );
	}

	if ( m->title && *m->title )
	{
		b = t = fl_strdup( m->title );
		while ( ( b = strchr( b, '\b' ) ) )
			memmove( b, b + 1, strlen( b ) );
		m->title_width = XTextWidth( pup_title_font_struct, t, strlen( t ) );
		fl_free( t );
	}
	else
		m->title_width = 0;

	m->cellh = pup_ascent + pup_desc + 2 * m->padh;
}


/***************************************
 * Parse the menu entries
 ***************************************/

#define MV( d, s )   memmove( d, s, strlen( s ) + 1 );

static int
parse_entry( int          n,
			 const char * str,
			 va_list      ap )
{
    PopUP *m = menu_rec + n;
    MenuItem *item;
    char *s,
		 *c,
		 *p,
		 *e,
		 *sc = NULL;
    unsigned int flags = 0;
	long num;

    if ( n < 0 || n >= fl_maxpup || ! str )
		return -1;

    s = fl_strdup( str );

    for ( c = strtok( s, "|" );
		  c && m->nitems < FL_MAXPUPI;
		  c = strtok( NULL, "|" ) )
    {
		flags = 0;
		m->item[ m->nitems ] = item = fl_calloc( 1, sizeof *item );
		item->str = NULL;
		item->icb = NULL;
		item->shortcut = NULL;
		item->ret = m->nitems + 1;
		item->ulpos = -1;
		item->subm = -1;

		p = c;
		while ( ( p = strchr( p, '%' ) ) && ! ( flags & M_ERR ) )
		{
			switch ( p[ 1 ] )
			{
				case '%' :
					MV( p, p + 1 );
					p = p + 1;
					break;

				case 't' :
					flags |= M_TITLE;
					MV( p, p + 2 );
					break;

				case 'f' :
					item->icb = va_arg( ap, FL_PUP_CB );
					MV( p, p + 2 );
					break;

				case 'F' :
					m->menu_cb = va_arg( ap, FL_PUP_CB );
					MV( p, p + 2 );
					break;

				case 'e' :
					m->enter_cb = va_arg( ap, FL_PUP_ENTERCB );
					MV( p, p + 2 );
					break;

				case 'E' :
					fl_setpup_entries( n, va_arg( ap, FL_PUP_ENTRY * ) );
					MV( p, p + 2 );
					break;

				case 'm' :
					item->subm = va_arg( ap, int );
					MV( p, p + 2 );
					break;

				case 'l' :
					MV( p, p + 2 );
					MV( c + 1, c );
					*c = '\010';
					p = p + 1;
					break;

				case 'i' :
					item->mode |= FL_PUP_INACTIVE;
					MV( p, p + 2 );
					break;

				case 'd' :
					item->mode |= FL_PUP_GREY;
					MV( p, p + 2 );
					break;

				case 'x' :
					num = strtol( p + 2, &e, 10 );
					if ( e == p + 2 )
					{
						flags |= M_ERR;
						M_err( "parse_entry", "Missing number after %%x" );
						break;
					}
					item->ret = num;
					while ( isspace( ( int ) *e ) )
						e++;
					MV( p, e );
					break;

				case 'B' :
					item->mode |= FL_PUP_CHECK;
					/* fall through */

				case 'b' :
					item->mode |= FL_PUP_BOX;
					MV( p, p + 2 );
					break;

				case 'R' :
					item->mode |= FL_PUP_CHECK;
					/* fall through */

				case 'r' :
					item->mode |= FL_PUP_BOX;
					num = strtol( p + 2, &e, 10 );
					if ( e == p + 2 )
					{
						flags |= M_ERR;
						M_err( "parse_entry", "Missing number after %%%c",
							   p + 1 );
						break;
					}
					item->radio = num;
					while ( isspace( ( int ) *e ) )
						e++;
					MV( p, e );
					break;

				case 'h' :
				case 's' :
					sc = va_arg( ap, char * );
					MV( p, p + 2 );
					break;

				default :
					flags |= M_ERR;
					M_err( "parse_entry", "Unknown sequence %%%c", p[ 1 ] );
					break;
			}
		}

		if ( flags & M_ERR )
		{
			M_err( "parse_entry", "Error while parsing pup entry" );
			fl_free( item );
			m->item[ m->nitems ] = NULL;
			break;
		}

		if ( sc )
		{
			M_info( 0, "shortcut = %s for %s", sc, c );
			convert_shortcut( sc, c, item, NSC );
		}

		if ( item->mode & FL_PUP_BOX )
			m->lpad = PADW + CHECKW + 2;

		if ( item->subm >= 0 )
			m->rpad = PADW + 16;

		if ( flags & M_TITLE )
		{
			char *t,
				 *b;

			m->title = fl_strdup( c );
			b = t = fl_strdup( c );
			while ( ( b = strchr( b, '\b' ) ) )
				memmove( b, b + 1, strlen( b ) );
			m->title_width = XTextWidth( pup_title_font_struct,
										 t, strlen( t ) );
			fl_free( t );
			fl_free( item );
			m->item[ m->nitems ] = NULL;
		}
		else
		{
			char *t,
			     *b;

			item->str = fl_strdup( c );
			item->len = strlen( item->str );

			b = t = fl_strdup( item->str );
			while ( ( b = strchr( b, '\b' ) ) )
				memmove( b, b + 1, strlen( b ) );
			m->maxw = FL_max( m->maxw,
							  fl_get_string_widthTAB( pup_font_style,
													  pup_font_size,
													  t, strlen( t ) ) );
			fl_free( t );
			m->nitems++;
		}
    }

    if ( c )
		M_err( "parse_entry", "Too many menu items, max is %d", FL_MAXPUPI );

    fl_free( s );

    return ( flags & M_ERR ) ? -1 : 0;
}


/***************************************
 ***************************************/

static void
close_pupwin( PopUP * pup )
{
    if ( pup->win )
    {
		XDestroyWindow( flx->display, pup->win );
		wait_for_close( pup->win );
		pup->win = None;
    }
}


/***************************************
 * initialize the menu system. Must be called first. Made defpup/newpup
 * etc. auto call fli_init_pup (instead of letting fl_initialize to call
 * it) and we save about ~25k in exe size for app not using pups
 ***************************************/

void
fli_init_pup( void )
{
	PopUP *mr;
	size_t i;

    if ( ! menu_rec )
    {
		menu_rec = fl_calloc( fl_maxpup, sizeof *menu_rec );

		for ( mr = menu_rec; mr < menu_rec + fl_maxpup; mr++ )
		{
			mr->title = NULL;
			menu_rec->win = menu_rec->parent_win = None;
			mr->cursor = None;
			mr->gc_active = mr->gc_inactive = None;

#if USE_SHADOW
			mr->gc_shadow = None;
#endif

			for ( i = 0; i <= FL_MAXPUPI; i++ )
				mr->item[ i ] = NULL;
			mr->menu_cb = NULL;
			mr->enter_cb = mr->leave_cb = NULL;
			mr->enter_data = mr->leave_data = NULL;
		}

		fl_setpup_default_fontsize( fli_cntl.pupFontSize ?
									fli_cntl.pupFontSize : -2 );
    }
}


/***************************************
 ***************************************/

int
fl_setpup_default_fontsize( int size )
{
    PopUP *pup,
		  *pups;
    int old_pup_font_size = pup_font_size;


    if ( size <= 0 )
		return old_pup_font_size;

    fli_init_pup( );
    pup = menu_rec;

    pup_font_size = size;
    pup_title_font_size = size;

    pup_font_struct = pup_title_font_struct = 0;

    if ( ! flx->display )
		return old_pup_font_size;

    init_pupfont( );

    for ( pups = pup + fl_maxpup; pup < pups; pup++ )
    {
		reset_max_width( pup );
		close_pupwin( pup );
    }

    return old_pup_font_size;
}


/***************************************
 ***************************************/

int
fl_setpup_default_fontstyle( int style )
{
    PopUP *pup,
		  *pups;
    int old_pup_font_style = pup_font_style;

    if ( ! flx->display )
		return old_pup_font_style;

    if ( style < 0 )
		return pup_font_style;

    fli_init_pup( );
    pup = menu_rec;

    pup_font_style = style;
    pup_title_font_style = style;
    pup_font_struct = pup_title_font_struct = NULL;

    init_pupfont( );

    for ( pups = menu_rec + fl_maxpup; pup < pups; pup++ )
		reset_max_width( pup );

    return old_pup_font_style;
}


/***************************************
 ***************************************/

void
fl_setpup_default_color( FL_COLOR fg,
						 FL_COLOR bg )
{
    pup_color = fg;
    pup_text_color = bg;
}


/***************************************
 ***************************************/

void
fl_setpup_default_pup_checked_color( FL_COLOR col )
{
    pup_checked_color = col;
}


/********************************************************************
 * Public routines
 ****************************************************************{***/

/***************************************
 * Allocate a new PopUP ID
 ***************************************/

int
fl_newpup( Window win )
{
    fli_init_pup( );

    if ( pup_level )
    {
		M_warn( "fl_newpup", "Inconsistent pup_level %d", pup_level );
		pup_level = 0;
    }

    if ( win == None )
		win = fl_root;

	if ( ! pup_bw_is_set )
	{
		pup_bw = fli_cntl.borderWidth ? fli_cntl.borderWidth : -2;
		pup_bw_is_set = 1;
	}

    /* if not private colormap, it does not matter who the popup's parent is
       and root probably makes more sense */

#if ALWAYSROOT
    return find_empty_index( fl_root );
#else
    {
		FL_State *fs = fl_state + fl_vmode;
		int nrt =    fs->pcm
			      || fli_visual( fl_vmode ) != DefaultVisual( flx->display,
															  fl_screen );

		return find_empty_index( nrt ? win : fl_root );
    }
#endif
}


/***************************************
 * Add pop-up entries
 ***************************************/

int
fl_addtopup( int          n,
			 const char * str,
			 ... )
{
    va_list ap;
	int ret;

    if ( n < 0 || n >= fl_maxpup )
		return -1;

#if FL_DEBUG >= ML_DEBUG
	{
		char *q = fl_strdup( str ),
			 *p;

		while ( ( p = strchr( q, '%' ) ) )
			*p = 'P';	/* % can cause problems */
		M_info( "fl_addtopup", q );
		fl_free( q );
	}
#endif

	va_start( ap, str );
	ret = parse_entry( n, str, ap );
	va_end( ap );

	return ret = 0 ? n : -1;
}


/***************************************
 * Allocate PopUP ID and optionally set all entries
 ***************************************/

int
fl_defpup( Window       win,
		   const char * str,
		   ... )
{
    int n;
	int ret;
    va_list ap;

    if ( ( n = fl_newpup( win ) ) < 0 )
    {
		M_err( "fl_defpup", "Can't Allocate" );
		return n;
    }

    va_start( ap, str );
    ret = parse_entry( n, str, ap );
    va_end( ap );

#if FL_DEBUG > ML_WARN
    if ( fli_cntl.debug > 1 )
    {
		PopUP *m = menu_rec + n;
		int i;

		fprintf( stderr, "Defpup for string: %s\n", str );
		for ( i = 0; i < m->nitems; i++ )
			fprintf( stderr, "%i %s ret=%d %s %s\n",
					 i, m->item[ i ]->str, m->item[ i ]->ret,
					 m->item[ i ]->shortcut ? "shortcut" : "",
					 m->item[ i ]->icb ? "callback" : "" );
    }
#endif

    return ret == 0 ? n : -1;
}


/***************************************
 * check to see if the requested value exists in popup m
 ***************************************/

static MenuItem *
ind_is_valid( PopUP * m,
			  int     ind )
{
    MenuItem **is = m->item,
		     **ise,
		     *item = NULL;

    for ( ise = is + m->nitems; is < ise && !item; is++ )
    {
		if ( ( *is )->ret == ind )
			item = *is;
		else if ( ( *is )->subm >= 0 )
			item = ind_is_valid( menu_rec + ( *is )->subm, ind );
    }

    return item;
}


/***************************************
 ***************************************/

static MenuItem *
requested_item_is_valid( const char * where,
						 int          nm,
						 int          ni )
{
    if ( nm < 0 || nm >= fl_maxpup )
    {
		M_err( where, "Bad popup index %d", nm );
		return NULL;
    }

    return ind_is_valid( menu_rec + nm, ni );
}


/***************************************
 * change attributes of a popup item
 ***************************************/

int
fl_setpup_mode( int          nm,
				int          ni,
				unsigned int mode )
{
    MenuItem *item;

    if ( ! ( item = requested_item_is_valid( "fl_setpup_mode", nm, ni ) ) )
		return -1;

	if ( ( item->mode = mode ) & FL_PUP_CHECK )
		item->mode |= FL_PUP_BOX;

	if ( item->mode & FL_PUP_RADIO )
	{
		item->mode |= FL_PUP_BOX;
		if ( ! item->radio )
			item->radio = 255;
	}

	if ( item->mode & FL_PUP_BOX )
		menu_rec[ nm ].lpad = PADW + CHECKW + 2;

    return 0;
}


#define AltMask  FL_ALT_VAL

/***************************************
 ***************************************/

static void
convert_shortcut( const char * sc,
				  const char * str,
				  MenuItem *   item,
				  int          n     FL_UNUSED_ARG )
{
    if ( ! item->shortcut )
		item->shortcut = fl_calloc( 1, NSC * sizeof *item->shortcut );

    item->ulpos = fli_get_underline_pos( str, sc ) - 1;
    fli_convert_shortcut( sc, item->shortcut );
    if ( sc[ 0 ] == '&' )
		M_info( "convert_shortcut", "sc=%s keysym=%ld\n",
				sc, item->shortcut[ 0 ] );
}


static void draw_popup( PopUP * );
static void draw_item( PopUP *,
					   int,
					   int );

#define TITLEH         ( pup_title_ascent + pup_title_desc + PADTITLE )

#define BLOCK

/***************************************
 ***************************************/

#ifndef BLOCK

static long old_delta = 0;

static int
popclose( XEvent * xev,
		  void *   data )
{
    if ( xev->type == DestroyNotify )
		fli_context->idle_delta = old_delta;
    return 0;
}

#endif


/***************************************
 ***************************************/

static void
wait_for_close( Window win )
{
#ifdef BLOCK
    long emask = AllEventsMask;
    XEvent xev;

	/* Drop all events for the window. We need to do sync before to be
	   sure all events are already in the event queue */

    XSync( flx->display, False );

    while ( XCheckWindowEvent( flx->display, win, emask, &xev ) )
		/* empty */ ;
#else
    fl_add_event_callback( win, DestroyNotify, popclose, 0 );
    fl_add_event_callback( win, UnmapNotify, popclose, 0 );
    old_delta = fli_context->idle_delta;
    fli_context->idle_delta = 10;
#endif
}


/****************************************************************
 * Global routine of doing pop-ups. Never returns unless user
 * does something with the pointer. For "hanging" pop-ups, a
 * pointer & focus grab will be activated and released upon returning.
 * Since requested item might be inactive, search for next active
 * item if current one is not
 ****************************************************************/

static int
get_valid_entry( PopUP * m,
				 int     target,
				 int     dir )
{
    if ( target < 1 )
		target = dir < 0 ? m->nitems : 1;
    if ( target > m->nitems )
		target = dir < 0 ? m->nitems : 1;

    for ( ; target > 0 && target <= m->nitems; target += dir )
		if ( ! (   m->item[ target - 1 ]->mode
				 & ( FL_PUP_GREY | FL_PUP_INACTIVE ) ) )
			return target;

    /* wrap */

    if ( target < 1 )
		target = dir < 0 ? m->nitems : 1;
    if ( target > m->nitems )
		target = dir < 0 ? m->nitems : 1;

    for ( ; target > 0 && target <= m->nitems; target += dir )
		if ( ! (   m->item[ target - 1 ]->mode
				 & ( FL_PUP_GREY | FL_PUP_INACTIVE ) ) )
			return target;

    M_err( "get_valid_entry", "No valid entries among total of %d", m->nitems );
    return 0;
}


#define alt_down    ( metakey_down( keymask ) != 0 )

/***************************************
 ***************************************/

static int
handle_shortcut( PopUP *      m,
				 KeySym       keysym,
				 unsigned int keymask )
{
    MenuItem **mi = m->item;
    int i,
		j;
    int sc,
		alt;

    for ( i = 0; i < m->nitems; i++ )
    {
		if (    ! ( mi[ i ]->mode & ( FL_PUP_GREY | FL_PUP_INACTIVE ) )
			 && mi[ i ]->shortcut )
			for ( j = 0; j < NSC && mi[ i ]->shortcut[ j ]; j++ )
			{
				sc = mi[ i ]->shortcut[ j ];
				alt = ( sc & AltMask ) == AltMask;
				sc &= ~ AltMask;
				if ( sc == ( int ) keysym && ! ( alt ^ alt_down ) )
					return i + 1;
			}
    }
    return 0;
}


/***************************************
 ***************************************/

static int
handle_submenu( PopUP *    m,
				MenuItem * item,
				int *      val )
{
    if ( ! ( item->mode & ( FL_PUP_GREY | FL_INACTIVE ) ) && item->subm >= 0 )
    {
		fl_setpup_position( m->x + m->w,
							m->y + m->cellh * ( *val - 1 )
							+ ( ( m->title && *m->title ) ?
								m->titleh - m->padh : 0 ) );
		if ( ( pup_subreturn = *val = fl_dopup( item->subm ) ) <= 0 )
			grab_both( m );
		else
			return 1;
    }

    return 0;
}


/***************************************
 * keyboard. Also checks shortcut
 ***************************************/

static int
pup_keyboard( XKeyEvent * xev,
			  PopUP *     m,
			  int *       val )
{
    KeySym keysym = NoSymbol;
    char buf[ 16 ];
    int i,
		oldval = *val;

    XLookupString( xev, buf, sizeof buf, &keysym, 0 );

    if ( IsHome( keysym ) )
    {
		draw_item( m, *val, FL_FLAT_BOX );
		*val = get_valid_entry( m, 1, -1 );
		draw_item( m, *val, FL_UP_BOX );
    }
    else if ( IsEnd( keysym ) )
    {
		draw_item( m, *val, FL_FLAT_BOX );
		*val = get_valid_entry( m, m->nitems, 1 );
		draw_item( m, *val, FL_UP_BOX );
    }
    else if ( IsUp( keysym ) )
    {
		draw_item( m, *val, FL_FLAT_BOX );
		*val = get_valid_entry( m, *val - 1, -1 );
		draw_item( m, *val, FL_UP_BOX );
    }
    else if ( IsDown( keysym ) )
    {
		draw_item( m, *val, FL_FLAT_BOX );
		*val = get_valid_entry( m, *val + 1, 1 );
		draw_item( m, *val, FL_UP_BOX );
    }
    else if ( IsRight( keysym ) )
    {
		if ( *val > 0 && *val <= m->nitems && m->item[ *val - 1 ]->subm )
		{
			oldval = *val;
			if ( handle_submenu( m, m->item[ *val - 1 ], val ) )
				keysym = XK_Return;
			else
				*val = oldval;
		}
    }
    else if ( IsLeft( keysym ) )
    {
#if 0
		if ( pup_level > 1 )	/* not allow closing the root menu */
#endif
		{
			*val = -1;
			keysym = XK_Escape;
		}
    }
    else if ( keysym == XK_Escape || keysym == XK_Cancel )
    {
		draw_item( m, *val, FL_FLAT_BOX );
		*val = -1;
    }
    else if ( keysym == XK_Return )
    {
		if ( *val > 0 && *val <= m->nitems && m->item[ *val - 1 ]->subm )
			handle_submenu( m, m->item[ *val - 1 ], val );
    }
    else
    {
		if ( ( i = handle_shortcut( m, keysym, xev->state ) ) )
		{
			*val = i;
			handle_submenu( m, m->item[ *val - 1 ], val );
			keysym = XK_Return;
		}
		else
			pup_using_keys = 0;
    }

    if ( oldval != *val && ( m->enter_cb || m->leave_cb ) )
    {
		if ( oldval > 0 && oldval <= m->nitems && m->leave_cb )
			m->leave_cb( m->item[ oldval - 1 ]->ret, m->leave_data );
		if ( *val > 0 && *val <= m->nitems && m->enter_cb )
			m->enter_cb( m->item[ *val - 1 ]->ret, m->enter_data );
    }

    return keysym == XK_Escape || keysym == XK_Return || keysym == XK_Cancel;
}


/***************************************
 * mouse moved. val is set to the item number (not value) upon return
 ***************************************/

static MenuItem *
handle_motion( PopUP * m,
			   int     mx,
			   int     my,
			   int *   val )
{
    int cval = -1;
    MenuItem *item = NULL;
    static MenuItem *lastitem = NULL;
	static PopUP *lastm = NULL;

	if (    mx >= 0 && mx <= ( int ) m->w
		 && my >= 0 && my <= ( int ) m->h - ( FL_abs( m->bw ) > 2 )
			                 - ( m->padh > 1 ) )
	{
		cval = m->nitems - ( m->h - ( FL_abs( m->bw ) > 2 )
							 - ( m->padh > 1 ) - my ) / m->cellh;
		if ( cval > 0 )
			item = m->item[ cval - 1 ];
	}

    if ( cval != *val || m != lastm )
    {
		draw_item( m, *val, FL_FLAT_BOX );
		draw_item( m, cval, FL_UP_BOX );
		*val = cval;
    }

    if ( item && item->mode & ( FL_PUP_GREY | FL_PUP_INACTIVE ) )
		item = NULL;

    if ( lastitem && item != lastitem && m->leave_cb )
		m->leave_cb( lastitem->ret, m->leave_data );

    if ( item && item != lastitem  && m->enter_cb )
		m->enter_cb( item->ret, m->enter_data );

    lastitem = item;
	lastm = m;

    return item;
}


/***************************************
 * Interaction routine. If mouse is released on the title bar,
 * consider its a "hanging" pop-up request else return
 ***************************************/

static int
pup_interact( PopUP * m )
{
    XEvent ev;
    int val       = 0,
		timeout   = 0,
		done      = 0,
		timer_cnt = 0;
    MenuItem *item;

    fli_reset_time( FLI_PUP_TIMER );
    m->event_mask |= KeyPressMask;
    ev.xmotion.time = 0;

	/* If the new popup was opened due to a kreypress mark the first
	   active entry as currently selected */

	if ( pup_using_keys )
	{
		int i;

		for ( i = 1; i < m->nitems; i++ )
		{
			if ( m->item[ i - 1 ]->mode & ( FL_PUP_GREY | FL_PUP_INACTIVE ) )
				continue;
			draw_item( m, i, FL_UP_BOX );
			val = i;
			break;
		}
	}

    while ( ! ( done || timeout ) )
    {
		long msec = fli_context->idle_delta;

		if ( fli_context->timeout_rec )
			fli_handle_timeouts( &msec );

		timeout = fli_time_passed( FLI_PUP_TIMER ) > 40.0;

		if ( ! XCheckWindowEvent( flx->display, m->win, m->event_mask, &ev ) )
		{
			/* If the mouse button was released or pressed not within the
			   popup's window we're through with the popup */

			if (    XCheckTypedEvent( flx->display, ButtonPress, &ev )
				 || XCheckTypedEvent( flx->display, ButtonRelease, &ev ) )
			{
				val = -1;
				break;
			}

			if ( timer_cnt++ % 10 == 0 )
			{
				timer_cnt = 0;
				fli_handle_idling( &ev, msec, 1 );
				fl_winset( m->win );
			}
			continue;
		}

		timer_cnt = 0;
		fli_query_age++;

		switch ( ev.type )
		{
			case Expose:
				draw_popup( m );
				XSync( flx->display, 0 );
				break;

			case MotionNotify:
				fli_compress_event( &ev, ButtonMotionMask );
				/* fall through */

			case ButtonPress:
				/* taking adv. of xbutton.x == xcrossing.x */

				fli_mousex  = ev.xmotion.x;
				fli_mousey  = ev.xmotion.y;
				fli_keymask = ev.xmotion.state;
				fli_query_age = 0;

				pup_using_keys = 0;
				item = handle_motion( m, ev.xbutton.x, ev.xbutton.y, &val );

				if ( item && item->subm >= 0 && ev.xbutton.x >= 0 )
				{
					unsigned int keymask;
					int old_val = val;

					done = handle_submenu( m, item, &val );
					fl_get_win_mouse( m->win, &ev.xbutton.x, &ev.xbutton.y,
									  &keymask );
					if ( ! ( done = keymask ? done : 1 ) )
						draw_item( m, old_val, FL_FLAT_BOX );
				}
				else if ( pup_level > 1 && val < 0 )
					done =    ev.xbutton.x < 0
						   && (    ev.xbutton.y <= m->par_y
				                || ev.xbutton.y > m->par_y + m->cellh );
				break;

			case ButtonRelease:
				fli_mousex  = ev.xbutton.x;
				fli_mousey  = ev.xbutton.y;
				fli_keymask = ev.xbutton.state;
				fli_query_age = 0;

				item = handle_motion( m, ev.xbutton.x, ev.xbutton.y, &val );
				if ( item && item->subm >= 0 && val != -1 )
					done = handle_submenu( m, item, &val );
				else
					done = val != 0;
				break;

			case KeyPress:
				fli_mousex  = ev.xkey.x;
				fli_mousey  = ev.xkey.y;
				fli_keymask = ev.xkey.state;
				fli_query_age = 0;

				pup_using_keys = 1;
				done = pup_keyboard( ( XKeyEvent * ) &ev, m, &val );
				break;

			case UnmapNotify:	/* must be by external routine */
				done = 1;
				val = -1;
				break;
		}
    }

    return timeout ? -1 : val;
}


/***************************************
 ***************************************/

static void
grab_both( PopUP * m )
{
    unsigned int evmask = m->event_mask;

	/* Set the window we're using */

	fl_winset( m->win );

    /* get rid of all non-pointer events in event_mask */

    evmask &= ~ ( ExposureMask | KeyPressMask );
    XSync( flx->display, 0 );
    fl_msleep( 30 );
    XChangeActivePointerGrab( flx->display, evmask, m->cursor, CurrentTime );

    /* do both pointer and keyboard grab */

    if ( XGrabPointer( flx->display, m->win, False, evmask, GrabModeAsync,
					   GrabModeAsync, None, m->cursor, CurrentTime )
		                                                        != GrabSuccess )
		M_err( "grab_both", "Can't grab pointer" );

    if ( XGrabKeyboard( flx->display, m->win, False, GrabModeAsync,
						GrabModeAsync, CurrentTime ) != GrabSuccess )
	{
		M_err( "grab_both", "Can't grab keyboard" );
		XUngrabPointer( flx->display, CurrentTime );
	}
}


/***************************************
 ***************************************/

int
fl_dopup( int n )
{
    PopUP *m = menu_rec + n;
    int val = 0;
    MenuItem *item = 0;
	XEvent xev;

    if ( n < 0 || n >= fl_maxpup )
    {
		M_err( "fl_dopup", "bad pupID: %d\n", n );
		return -1;
    }

    if ( pup_level == 0 )
		fli_context->pup_id = n;

    pup_subreturn = -1;

    pup_level++;
	pup_internal_showpup_call = 1;
    fl_showpup( n );

	/* If one opens a touch menu and is fast enough to move the mouse out
	   of the window before the grab is active an extra EnterNotify event
	   comes in that results in the touch menu getting opened again after
	   closing it, so delete all such events for the form the touch menu
	   belongs to. */

	while ( XCheckWindowEvent( flx->display, fl_popup_parent_window,
							   EnterWindowMask, &xev ) )
		/* empty */ ;

    /* pup_interact returns the item number */

    val = pup_interact( m );

    if ( m->win )
    {
		XUnmapWindow( flx->display, m->win );
		wait_for_close( m->win );
    }
    else
		M_err( "fl_dopup", "Window already closed" );

	/* The following is necessary because 'save_under' may bot be supported. */

	if ( fl_popup_form )
	{
		fl_winset( fl_popup_form->window );
		fl_set_clipping( m->win_x - fl_popup_form->x,
						 m->win_y - fl_popup_form->y,
						 m->w, m->h );
		fl_redraw_form( fl_popup_form );
		fl_set_clipping( 0, 0, 0, 0 );
	}

    if ( pup_level > 1 )
    {
		/* need to remove all MotionNotify otherwise wrong coord */

		while ( XCheckMaskEvent( flx->display, ButtonMotionMask, &xev ) )
			/* empty */ ;
    }

    /* handle callback if any  */

    pup_level--;
    if (    val > 0
		 && val <= m->nitems
		 && ( pup_subreturn < 0 || ( pup_subreturn > 0 && pup_level > 0 ) ) )
    {
		item = m->item[ val - 1 ];
		if ( item->mode & ( FL_PUP_GREY | FL_PUP_INACTIVE ) )
			return -1;

		if ( item->subm >= 0 )
			return -1;

		if ( item->radio )
			reset_radio( m, item );
		else if ( item->mode & FL_PUP_CHECK )
		{
			item->mode &= ~ FL_PUP_CHECK;
			item->mode |= FL_PUP_BOX;
		}
		else if ( item->mode & FL_PUP_BOX )
			item->mode |= FL_PUP_CHECK;

		val = item->ret;
		if ( item->icb )
			val = item->icb( val );
		if ( m->menu_cb )
			val = m->menu_cb( val );
    }

    if ( pup_level <= 0 )
		fli_context->pup_id = -1;

    if ( pup_subreturn > 0 )
		val = pup_subreturn;

    return val;
}


/***************************************
 ***************************************/

void
fl_freepup( int n )
{
    PopUP *p = menu_rec + n;
    int i;

    if ( n < 0 || n >= fl_maxpup )
		return;

    if ( ! p->parent_win )
    {
		M_warn( "freepup", "freeing a unallocated/free'ed popup %d\n", n );
		return;
    }

    for ( i = 0; i < p->nitems; i++ )
    {
		if ( p->item[ i ] )
		{
			if ( p->item[ i ]->subm >= 0 && p->isEntry )
				fl_freepup( p->item[ i ]->subm );
			fl_safe_free( p->item[ i ]->str );
			fl_safe_free( p->item[ i ]->shortcut );
		}

		fl_safe_free( p->item[ i ] );
    }

    p->parent_win = None;

	if ( p->gc_active != None )
		XFreeGC( flx->display, p->gc_active );
	if ( p->gc_inactive != None )
		XFreeGC( flx->display, p->gc_inactive );

    fl_safe_free( p->title );

    close_pupwin( p );
}


/*
 * Some convenience functions
 */

/***************************************
 ***************************************/

void
fl_setpup_shortcut( int          nm,
					int          ni,
					const char * sc )
{
    MenuItem *item;

    if ( sc && ( item = requested_item_is_valid( "pupshortcut", nm, ni ) ) )
		convert_shortcut( sc, item->str, item, NSC );
}


/***************************************
 ***************************************/

FL_PUP_CB
fl_setpup_menucb( int       nm,
				  FL_PUP_CB cb )
{
    PopUP *m = menu_rec + nm;
    FL_PUP_CB oldcb = NULL;

    if ( nm >= 0 && nm < fl_maxpup && m->parent_win )
    {
		oldcb = m->menu_cb;
		m->menu_cb = cb;
    }

    return oldcb;
}


/***************************************
 ***************************************/

FL_PUP_ENTERCB
fl_setpup_entercb( int            nm,
				   FL_PUP_ENTERCB cb,
				   void *         data )
{
    FL_PUP_ENTERCB oldcb;
    PopUP *m;
    int n,
		subm;

    if ( nm < 0 || nm >= fl_maxpup )
		return NULL;

	m = menu_rec + nm;
	oldcb = m->enter_cb;
	m->enter_cb = cb;
	m->enter_data = data;
	for ( n = 0; n < m->nitems; n++ )
		if ( ( subm = m->item[ n ]->subm ) >= 0 && ! menu_rec[ subm ].enter_cb )
			fl_setpup_entercb( subm, cb, data );

    return oldcb;
}


/***************************************
 ***************************************/

FL_PUP_LEAVECB
fl_setpup_leavecb( int            nm,
				   FL_PUP_LEAVECB cb,
				   void *         data )
{
    FL_PUP_LEAVECB oldcb;
    PopUP *m;
    int n,
		subm;

    if ( nm < 0 || nm >= fl_maxpup )
		return NULL;

	m = menu_rec + nm;
	oldcb = m->leave_cb;
	m->leave_cb = cb;
	m->leave_data = data;
	for ( n = 0; n < m->nitems; n++ )
		if ( ( subm = m->item[ n ]->subm ) >= 0 && ! menu_rec[ subm ].enter_cb )
			fl_setpup_leavecb( subm, cb, data );

    return oldcb;
}


/***************************************
 ***************************************/

FL_PUP_CB
fl_setpup_itemcb( int       nm,
				  int       ni,
				  FL_PUP_CB cb )
{
    MenuItem *item;
    FL_PUP_CB oldcb = NULL;

    if ( ( item = requested_item_is_valid( "fl_setpup_itemcb", nm, ni ) ) )
    {
		oldcb = item->icb;
		item->icb = cb;
    }

    return oldcb;
}


/***************************************
 ***************************************/

void
fl_setpup_title( int          nm,
				 const char * title )
{
    PopUP *m = menu_rec + nm;
	char *t,
		 *b;

    if ( nm < 0 || nm >= fl_maxpup || ! title )
		return;

	fl_safe_free( m->title );
	m->title = fl_strdup( title ? title : "" );
	b = t = fl_strdup( title ? title : "" );
	while ( ( b = strchr( b, '\b' ) ) )
		memmove( b, b + 1, strlen( b ) );
	m->title_width = XTextWidth( pup_title_font_struct, t, strlen( t ) );
	fl_free( t );
}


/***************************************
 ***************************************/

Cursor
fl_setpup_cursor( int nm,
				  int cursor )
{
    PopUP *m = menu_rec + nm;
    Cursor old;

    if ( nm < 0 || nm >= fl_maxpup )
		return 0;

	old = m->cursor;
	m->cursor = cursor ? fl_get_cursor_byname( cursor ) : pup_defcursor;

    return old;
}


/***************************************
 ***************************************/

Cursor
fl_setpup_default_cursor( int cursor )
{
    Cursor old_defcursor = pup_defcursor;

    pup_defcursor = fl_get_cursor_byname( cursor );
    return old_defcursor;
}


/***************************************
 ***************************************/

void
fl_setpup_pad( int n,
			   int padw,
			   int padh )
{
    PopUP *m = menu_rec + n;

    if ( n < 0 || n >= fl_maxpup )
		return;

	m->padh = padh;
	m->rpad = m->lpad = padw;
	m->cellh = pup_ascent + pup_desc + 2 * m->padh;
}


/***************************************
 ***************************************/

static void
recurse( PopUP * m,
		 void    ( * set )( int, int ),
		 int     val )
{
    int i;

    for ( i = 0; i < m->nitems; i++ )
		if ( m->item[ i ]->subm )
			set( m->item[ i ]->subm, val );
}


/***************************************
 ***************************************/

void
fl_setpup_shadow( int n,
				  int y  FL_UNUSED_ARG )
{
#if USE_SHADOW
    PopUP *m = menu_rec + n;
    int i;
#endif

    if ( n < 0 || n >= fl_maxpup )
		return;

#if USE_SHADOW
	m->noshadow = ! y;
	for ( i = 0; i < m->nitems; i++ )
		if ( m->item[ i ]->subm )
			fl_setpup_shadow( m->item[ i ]->subm, y );
#endif
}


/***************************************
 ***************************************/

void
fl_setpup_bw( int n,
			  int bw )
{
    PopUP *m = menu_rec + n;
    int i;

    if ( n < 0 || n >= fl_maxpup )
		return;

	m->bw = bw;
	for ( i = 0; i < m->nitems; i++ )
		if ( m->item[ i ]->subm )
			fl_setpup_bw( m->item[ i ]->subm, bw );
}


/***************************************
 ***************************************/

void
fl_setpup_softedge( int n,
					int y )
{
    PopUP *m = menu_rec + n;

    if ( n < 0 || n >= fl_maxpup )
		return;

	m->bw = y ? - FL_abs( m->bw ) : FL_abs( m->bw );
	recurse( m, fl_setpup_softedge, y );
}


/***************************************
 ***************************************/

static void
reset_radio( PopUP *    m,
			 MenuItem * item )
{
    MenuItem **ii;

    for ( ii = m->item; ii < m->item + m->nitems; ii++ )
		if ( ( *ii )->radio == item->radio )
			( *ii )->mode &= ~ FL_PUP_CHECK;
    item->mode |= FL_PUP_CHECK;
}


/***************************************
 ***************************************/

void
fl_setpup_selection( int nm,
					 int ni )
{
    MenuItem *item;

    if (    ( item = requested_item_is_valid( "fl_setpup_selection", nm, ni ) )
		 && item->radio )
		reset_radio( menu_rec + nm, item );
}


/***************************************
 ***************************************/

void
fl_setpup_submenu( int m,
				   int i,
				   int subm )
{
    MenuItem *item;

    if ( ( item = requested_item_is_valid( "fl_setpup_submenu", m, i ) ) )
    {
		menu_rec[ m ].rpad = PADW + 16;
		item->subm = subm;
    }
}


/**** End of PUBLIC routines for pop-ups *******************}*/


/**** ALL drawing routines */


/***************************************
 * draw item. Index starts from 1
 ***************************************/

static void
draw_item( PopUP * m,
		   int     i,
		   int     style )
{
    int j = i - 1;
    int bw = FL_abs( m->bw );
    int y = m->titleh + m->cellh * j + 1,
		dy = m->cellh - 2;
    char *str;
    MenuItem *item;
    GC gc;

    if ( j < 0 || j >= m->nitems )
		return;

    item = m->item[ j ];
    gc = ( item->mode & FL_PUP_GREY ) ? m->gc_inactive : m->gc_active;
    str = item->str;

    if ( ! ( item->mode & FL_PUP_GREY ) )
		fl_drw_box( style, bw + 1, m->titleh + m->cellh * j + 1,
					m->w - 2 * bw - 2, dy, pup_color,
					m->bw == -1 ? -1 : -2 );

    if ( item->mode & FL_PUP_BOX && ! ( item->mode & FL_PUP_CHECK ) )
    {
		int w = CHECKW + ( item->radio ? 0 : 2 );
		int bbw = item->radio ? -2 : -1;

		( item->radio ? fl_drw_checkbox : fl_drw_box )
			( FL_UP_BOX, 2 * bw + ( m->lpad - w ) / 2, y + ( dy - CHECKW ) / 2,
			  w, w, pup_color, bbw );
    }

    if ( item->mode & FL_PUP_CHECK )
    {
		int w = CHECKW + ( item->radio ? 0 : 2 );
		int bbw = item->radio ? -3 : -2;

		( item->radio ? fl_drw_checkbox : fl_drw_box )
			( FL_DOWN_BOX, 2 * bw + ( m->lpad - w ) / 2,
			  y + ( dy - CHECKW ) / 2, w, w,
			  fli_depth( fl_vmode ) == 1 ? FL_BLACK : pup_checked_color, bbw );
    }

    /* show text */

    j = str[ 0 ] == '\010';
    fli_drw_stringTAB( m->win, gc,
					   m->lpad + 2 * bw, y + m->padh + pup_ascent - 1,
					   pup_font_style, pup_font_size, str + j,
					   strlen( str ) - j, 0 );

    /* do underline */

    if ( item->ulpos >= 0 )
    {
		XRectangle *xr;

		xr = fli_get_underline_rect( pup_font_struct, m->lpad + 2 * bw,
									 y + m->padh + pup_ascent - 1,
									 str, item->ulpos );
		XFillRectangle( flx->display, m->win, gc,
						xr->x, xr->y, xr->width, xr->height );
    }

    if ( j )
		fl_draw_symbol( "@DnLine", 2 * bw, y + dy, m->w - 4 * bw, 1, FL_COL1 );

    if ( item->subm >= 0 )
		fl_draw_symbol( (    style == FL_UP_BOX
						  && ! (   item->mode
								 & ( FL_PUP_GREY | FL_PUP_INACTIVE ) ) ) ?
						"@DnArrow" : "@UpArrow",
						m->w - 2 * bw - 9 - m->rpad / 2,
						y + dy / 2 - 7,
						16, 16, FL_BLACK );
}


/***************************************
 ***************************************/

static void
draw_title( Display * d,
			Drawable  w,
			int       x,
			int       y,
			char *    s )
{
	char *t, *b;
	int n;

    if ( ! s || ! * s )
		return;
#if 0
    fl_drw_text( FL_ALIGN_CENTER, x - 2, y - 5,
				 XTextWidth( pup_font_struct, s, strlen( s ) ),
				 0, FL_SLATEBLUE, pup_title_font_size,
				 pup_title_font_style + FL_EMBOSSED_STYLE, s );
#else
	b = t = fl_strdup( s );
	while ( ( b = strchr( b, '\b' ) ) )
		memmove( b, b + 1, strlen( b ) );

	n = strlen( t );

    fl_set_font( pup_title_font_style, pup_title_font_size );
    fl_textcolor( pup_text_color );
    XDrawString( d, w, flx->textgc, x - 1, y - 1, t, n );
    XDrawString( d, w, flx->textgc, x, y - 1, t, n );
    XDrawString( d, w, flx->textgc, x + 1, y - 1, t, n );
    XDrawString( d, w, flx->textgc, x - 1, y, t, n );
    XDrawString( d, w, flx->textgc, x + 1, y, t, n );
    XDrawString( d, w, flx->textgc, x - 1, y + 1, t, n );
    XDrawString( d, w, flx->textgc, x, y + 1, t, n );
    XDrawString( d, w, flx->textgc, x + 1, y + 1, t, n );
    fl_textcolor( FL_WHITE );
    XDrawString( d, w, flx->textgc, x, y, t, n );

	fl_free( t );
#endif
}


/***************************************
 * Instead of poping up the menu at mouse location, use externally
 * set position. Good for programmatical pop-ups
 ***************************************/

static int extpos = 0;
static FL_Coord extx = 0,
                exty = 0;

void
fl_setpup_position( int x,
					int y )
{
    extpos = ! ( x == -1 && y == -1 );
    extx = x;
    exty = y;
}


/***************************************
 ***************************************/

static void
draw_popup( PopUP * m )
{
    int i;

    if ( m->title && *m->title )
		m->titleh = TITLEH;
    else
		m->titleh = m->padh;

#if USE_SHADOW
    if ( ! m->noshadow )
    {
        /** create the shadow  ***/

		XFillRectangle( flx->display, m->win, m->gc_shadow,
						m->w, m->shade, m->shade, m->h );
		XFillRectangle( flx->display, m->win, m->gc_shadow,
						m->shade, m->h, m->w - m->shade, m->shade );
    }
#endif

	/*** make the popup box  ***/

    fl_drw_box( FL_UP_BOX, 0, 0, m->w, m->h, pup_color, m->bw );

	/*** title box ***/

    if ( m->title && *m->title )
    {
		fl_drw_box( FL_FRAME_BOX, 3, 3, m->w - 6, m->titleh - 6, pup_color, 1 );

		draw_title( flx->display, m->win, ( m->w - m->title_width ) / 2,
					PADTITLE / 2 + pup_title_ascent, m->title );
    }

    for ( i = 1; i <= m->nitems; i++ )
		draw_item( m, i, FL_FLAT_BOX );
}


/***************************************
 ***************************************/

void
fl_showpup( int n )
{
    int x,
		y;
    FL_Coord px = 1,
		     py = 1,
		     pw = fl_scrw,
		     ph = fl_scrh,
		     mw,
		     mh;
    PopUP *m = menu_rec + n;
	int req_y = exty;

    if ( n < 0 || n >= fl_maxpup )
    {
		fprintf( stderr, "bad pupID: %d\n", n );
		return;
    }

    if ( m->title )
		m->titleh = TITLEH;
    else
		m->titleh = m->padh;

    if ( ! m->win )
    {
		int bw = 0,
			w,
			h;
		XSetWindowAttributes xswa;
		unsigned long int vmask;
		unsigned int depth = fli_depth( fl_vmode );
		Visual *visual = fli_visual( fl_vmode );

		m->maxw = FL_max( m->title_width, m->maxw );
		m->w = m->maxw + m->rpad + m->lpad + 4 * FL_abs( m->bw );
		m->h = m->nitems * m->cellh + m->titleh + 1 + ( m->padh > 1 )
			   + 2 * ( FL_abs( m->bw ) > 2 );

		m->event_mask =   ExposureMask
			            | ButtonPressMask
			            | ButtonReleaseMask
			            | ButtonMotionMask
			            | OwnerGrabButtonMask
			            | PointerMotionHintMask
			            | StructureNotifyMask 	/* for UnmapNotify */
						| EnterWindowMask
						| KeyPressMask;

		xswa.event_mask            = m->event_mask;
		xswa.save_under            = True;
		xswa.backing_store         = WhenMapped;
		xswa.cursor                = m->cursor;
		xswa.border_pixel          = 0;
		xswa.colormap              = fli_colormap( fl_vmode );
		xswa.do_not_propagate_mask = ButtonPress | ButtonRelease | KeyPress;
		vmask =   CWEventMask   | CWSaveUnder | CWBackingStore  | CWCursor
			    | CWBorderPixel | CWColormap  | CWDontPropagate;

		/* Setting the transient hint (done after the window is created)
		   does not do the trick if parent is the root window */

		if ( m->parent_win == fl_root )
		{
			xswa.override_redirect = True;
			vmask |= CWOverrideRedirect;
		}

		w = m->w;
		h = m->h;

#if USE_SHADOW
		if ( ! m->noshadow )
		{
			w += m->shade;
			h += m->shade;
		}
#endif

		m->win = XCreateWindow( flx->display, m->parent_win,
								0, 0, w, h, bw,
								depth, InputOutput, visual,
								vmask, &xswa );

		XSetTransientForHint( flx->display, m->win, m->parent_win );
		XStoreName( flx->display, m->win, m->title );

		if ( ! m->gc_active && ! m->gc_inactive )
		{
			XGCValues xgcv;

			xgcv.foreground     = fl_get_flcolor( pup_text_color );
			xgcv.font           = pup_font_struct->fid;
			xgcv.subwindow_mode = IncludeInferiors;
			xgcv.stipple        = FLI_INACTIVE_PATTERN;
			vmask = GCForeground | GCFont | GCSubwindowMode | GCStipple;

#if USE_SHADOW
			/* GC for the shadow */

			m->gc_shadow = XCreateGC( flx->display, m->win, vmask, &xgcv );
			XSetFillStyle( flx->display, m->gc_shadow, FillStippled );
#endif

			/* GC for main text */

			m->gc_active = XCreateGC( flx->display, m->win, vmask, &xgcv );

			/* GC for inactive text */

			xgcv.foreground = fl_get_flcolor( FL_INACTIVE );
			m->gc_inactive = XCreateGC( flx->display, m->win, vmask, &xgcv );

			/* special hack for B&W */

			if ( fli_dithered( fl_vmode ) )
				XSetFillStyle( flx->display, m->gc_inactive, FillStippled );
		}
    }

    /* external coord is given relative to root */

    if ( ! extpos )
	{
		unsigned int kmask;

		fl_get_mouse( &extx, &exty, &kmask );
	}
    else if ( extx < 0 )
		extx = - extx - m->w;
    else if ( exty < 0 )
		exty = - exty - m->h;

    /* if parent is not root, need to find its geometry */

    if ( m->parent_win != fl_root )
		fl_get_win_geometry( m->parent_win, &px, &py, &pw, &ph );

    x = extx;
    y = exty;
    mw = m->w;
    mh = m->h;

#if ! ALWAYSROOT
    /* check if the stuff is inside the window, if not, make it so  */

    if ( x + mw > px + pw )
		x = px + pw - mw;
    if ( y + mh > py + ph )
		y = py + ph - mh;
#endif

    /* parent might out of sight */

    if ( x + mw > fl_scrw )
		x = fl_scrw - mw;
    if ( y + mh > fl_scrh )
		y = fl_scrh - mh;

    /* if window is too small, show whatever we can */

    if ( x < 1 )
		x = 1;
    if ( y < 1 )
		y = 1;

    /* see if we need to warp mouse. If external coord, don't do it */

    if ( ! extpos && ( x != extx || y != exty ) )
		XWarpPointer( flx->display, None, None, 0, 0, 0, 0,
					  x - extx, y - exty );

    extpos = 0;
    m->x = x;
    m->y = y;

    /* Calculate where the window is supposed to appear */

	m->win_x = x - px;
	m->win_y = y - 2 * m->padh - py;

	/* Normally, one would move the window first and only then map it.
	   Unfortuantely, this results in some strange effect for submenu
	   windows if the window had been mapped before: if the old position
	   of the submenu window intersects with the new position of the
	   parent window this part of the parent window becomes transparent,
	   i.e the background suddenly is visible. Mapping first and only
	   then moving the window seems to get rid of the problem. Unfortunately
	   I haven't found a reason and start to susect it's a bug in X.   JTT */

	XMapWindow( flx->display, m->win );
	XMoveWindow( flx->display, m->win, m->win_x, m->win_y );
	XSetWMColormapWindows( flx->display, m->parent_win, &m->win, 1 );

	/* The function gets either called directly from a user program or via
	   the fl_dopup() function. In the first case we need to draw the pupup
	   and then remove all events the creation of the window produced (after
	   a sync so that we can be sure all events are already on the event
	   queue). Otherwise we can leave the drawing to the routine dealing
	   with the events for the window and just grab pointer and keyboard. */

	if ( ! pup_internal_showpup_call )
	{
		XEvent ev;

		fl_winset( m->win );
		draw_popup( m );
		XSync( flx->display, False );

		while ( XCheckWindowEvent( flx->display, m->win, AllEventsMask, &ev) )
			/* empty */ ;
	}
	else
	{
		m->par_y = m->padh + req_y - ( y - py );
		grab_both( m );
		pup_internal_showpup_call = 0;
	}
}


/***************************************
 ***************************************/

void
fl_hidepup( int n )
{
    if ( n >= 0 && n < fl_maxpup )
		close_pupwin( menu_rec + n );
    if ( n == fli_context->pup_id )
		fli_context->pup_id = -1;
}


/***************************************
 ***************************************/

unsigned int
fl_getpup_mode( int nm,
				int ni )
{
    MenuItem *item;

    if ( ( item = requested_item_is_valid( "fl_getpup_mode", nm, ni ) ) )
		return item->mode;

    return 0;
}


/***************************************
 ***************************************/

const char *
fl_getpup_text( int nm,
				int ni )
{
    MenuItem *item;

    if ( ( item = requested_item_is_valid( "fl_getpup_text", nm, ni ) ) )
		return item->str;

    return NULL;
}


/***************************************
 ***************************************/

void
fli_replacepup_text( int          nm,
					 int          ni,
					 const char * nt )
{
    MenuItem *item;

    if ( ! nt )
		nt = "";

    if ( ( item = requested_item_is_valid( "fli_replacepup_text", nm, ni ) ) )
    {
		fl_safe_free( item->str );
		item->str = fl_strdup( nt );
    }
}


/***************************************
 ***************************************/

int
fl_setpup_maxpup( int n )
{
    int i,
		j;

    if ( n < FL_MAXPUP )
		return FL_MAXPUP;

    fli_init_pup( );

    menu_rec = fl_realloc( menu_rec, n * sizeof *menu_rec );
    for ( i = fl_maxpup; i < n; i++ )
    {
		menu_rec[ i ].title = NULL;
		menu_rec[ i ].parent_win = menu_rec[ i ].win = None;
		menu_rec[ i ].cursor = None;

#if USE_SHADOW
		menu_rec[ i ].gc_shadow = None;
#endif

		menu_rec[ i ].gc_active = menu_rec[ i ].gc_inactive = None;

		for ( j = 0; j <= FL_MAXPUPI; j++ )
			menu_rec[ i ].item[ j ] = NULL;

		menu_rec[ i ].menu_cb = NULL;
		menu_rec[ i ].enter_cb = menu_rec[ i ].leave_cb = NULL;
		menu_rec[ i ].enter_data = menu_rec[ i ].leave_data = NULL;
    }

    return fl_maxpup = n;
}


/***************************************
 * Build the menu using low-level pup support
 ***************************************/

static int
generate_menu( int                  n,
			   const FL_PUP_ENTRY * pup,
			   int                  top )
{
    static const FL_PUP_ENTRY *p = NULL;
    static PopUP *menu = NULL;
    static int val = 0;

    if ( top )
    {
		val = 1;
		menu = menu_rec + n;
		menu->isEntry = 1;
		p = pup;
    }

	if ( ! p || ! p->text )
		return n;

    for ( ; p && p->text; p++, val++ )
    {
		int cnt = 0;
		char *t,
			 *w;

		/* Count number of '%' */

		for ( w = ( char * ) p->text; *w; w++ )
			if ( *w == '%' )
				cnt++;

		/* Get copy of the string with enough room for all further additions */

		w = t = fl_malloc( strlen( p->text ) + cnt + 6 + log10( INT_MAX ) );
		strcpy( t, p->text );

		/* Double all '%' */

		while ( *w && ( w = strchr( w, '%' ) ) )
		{
			memmove( w + 1, w, strlen( w ) + 1 );
			w += 2;
		}

		if ( *t != '/' )
		{
			if ( *t == '_' )
				*t = '\010';

			sprintf( t + strlen( t ), "%%x%d", val );

			fl_addtopup( n, t );

			if ( p->mode )
				fl_setpup_mode( n, val, p->mode );

			if ( p->shortcut && *p->shortcut )
				fl_setpup_shortcut( n, val, p->shortcut );

			if ( p->callback )
				fl_setpup_itemcb( n, val, p->callback );
		}
		else
		{
			int m = fl_newpup( menu->parent_win );

			if ( t[ 1 ] == '_' )
				t[ 1 ] = '\010';

			sprintf( t + strlen( t ), "%%x%d%%m", val );

			fl_addtopup( n, t + 1, m );

			if ( p->shortcut && *p->shortcut )
				fl_setpup_shortcut( n, val, p->shortcut );

			if ( p->mode & FL_PUP_GREY )
				fl_setpup_mode( n, val, p->mode & FL_PUP_GREY );

			val++;
			generate_menu( m, ++p, 0 );
			menu_rec[ m ].isEntry = 1;
		}

		fl_free( t );
    }

	val--;

    return n;
}


/***************************************
 ***************************************/

int
fl_setpup_entries( int            nm,
				   FL_PUP_ENTRY * entries )
{
    return generate_menu( nm, entries, 1 );
}


/***************************************
 ***************************************/

void
fli_reparent_pup( int    n,
				  Window newwin )
{
    FL_State *fs = fl_state + fl_vmode;
    int nrt =    fs->pcm
		      || fli_visual( fl_vmode ) != DefaultVisual( flx->display,
														  fl_screen );

    if ( newwin == 0 )
		newwin = fl_root;

    /* if we are using default visual/depth, root is a good choice */

    if ( ! nrt )
		newwin = fl_root;
#if ALWAYSROOT
    newwin = fl_root;
#endif

    if ( n >= 0 && n < fl_maxpup )
    {
		if ( menu_rec[ n ].win )
		{
			XEvent xev;

			XReparentWindow( flx->display, menu_rec[ n ].win, newwin, 0, 0 );
			while ( ! XCheckTypedEvent( flx->display, ReparentNotify, &xev ) )
				/* empty */ ;
		}
		else
			menu_rec[ n ].parent_win = newwin;
    }
}


/***************************************
 ***************************************/

void
fli_getpup_window( int      n,
				   Window * parent_win,
				   Window * win )
{
    if ( n >= 0 && n < fl_maxpup )
    {
		*parent_win = menu_rec[ n ].parent_win;
		*win = menu_rec[ n ].win;
    }
    else
		*parent_win = *win = None;
}


/***************************************
 ***************************************/

int
fl_getpup_items( int n )
{
    int m = 0,
		k,
		i;

    if ( n >= 0 && n < fl_maxpup )
    {
		m = k = menu_rec[ n ].nitems;

		for ( i = 0; i < k; i++ )
			if ( menu_rec[ n ].item[ i ]->subm >= 0 )
				m += fl_getpup_items( menu_rec[ n ].item[ i ]->subm );
    }

    return m;
}


/***************************************
 ***************************************/

int
fl_current_pup( void )
{
    return fli_context->pup_id;
}


/***************************************
 ***************************************/

int
fl_setpup_default_bw( int bw )
{
    int ori = pup_bw;

    pup_bw = bw;
	pup_bw_is_set = 1;

    return ori;
}
