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
 * \file xsupport.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *
 *  Form initialization and Windowing support.
 *
 *  Further isolation of dependencies of FORMS on underlining window
 *  systems.
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_xsupt = "$Id: xsupport.c,v 1.20 2009/05/08 18:00:58 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"


/***************************************
 * for debugging only
 ***************************************/

void
fli_check_key_focus( const char * s,
					 Window       win )
{
    int r;
    Window w;

    if ( fli_cntl.debug > 1 )
    {
		XGetInputFocus( flx->display, &w, &r );
		M_info( "KBDFocus", "%s:%s FWin=%lu ReqW=%lu",
				s ? s : "", w == win ? "OK" : "Wrong", w, win );
    }
}


/*****************************************************************
 * Pointer query routines
 ***********************************************************{*****/

/***************************************
 * Return the window ID mouse currently is in and sets the mouse location
 * relative to root
 ***************************************/

Window
fl_get_mouse( FL_Coord     * x,
			  FL_Coord     * y,
			  unsigned int * keymask )
{
    Window rjunk,
		   childwin;
    int cx,
		cy,
		xx,
		yy;

    XQueryPointer( flx->display, fl_root, &rjunk, &childwin,
				   &xx, &yy, &cx, &cy, keymask );
    *x = xx;
    *y = yy;

    return childwin;
}


/***************************************
 * find the mouse position relative to win and return the child win
 * the mouse is currently in
 ***************************************/

Window
fl_get_win_mouse( Window         win,
				  FL_Coord     * x,
				  FL_Coord     * y,
				  unsigned int * keymask )
{
    Window rjunk,
		   childwin;
    int xx,
		yy,
		ix,
		iy;

    XQueryPointer( flx->display, win, &rjunk, &childwin,
				   &xx, &yy, &ix, &iy, keymask );
    *x = ix;
    *y = iy;

    return childwin;
}


/***************************************
 ***************************************/

Window
fl_get_form_mouse( FL_FORM      * form,
				   FL_Coord     * x,
				   FL_Coord     * y,
				   unsigned int * keymask )
{
	Window win = None;
	FL_pixmap *flp = form->flpixmap;

    if ( fli_get_visible_forms_index( form ) >= 0 )
    {
		win = ( flp && flp->win != None ) ? flp->win : form->window;
		fl_get_win_mouse( win, x, y, keymask );
    }

    return win;
}


/***************************************
 * warp mouse to (mx, my) relative to root window
 ***************************************/

void
fl_set_mouse( FL_Coord mx,
			  FL_Coord my )
{
    XWarpPointer( flx->display, None, fl_root, 0, 0, 0, 0, mx, my );
}


/*** End of pointer query routines ******************/


Pixmap fli_gray_pattern[ 3 ];
GC fli_bwgc[ 3 ];
GC fli_whitegc;
GC fl_drawgc[ 18 ];


static unsigned char gray40_bits[] =
{
    0xee, 0xbb, 0xee, 0xbb, 0xee, 0xbb, 0xee, 0xbb
};

static unsigned char gray50_bits[] =
{
    0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55
};

static unsigned char gray60_bits[] =
{
    0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44
};


/***************************************
 ***************************************/

void
fli_init_stipples( void )
{
    if ( ! fli_gray_pattern[ 0 ] )
    {
		fli_gray_pattern[ 0 ] =
			XCreateBitmapFromData( flx->display, fl_root,
								   ( char * ) gray40_bits, 8, 8 );
		fli_gray_pattern[ 1 ] =
			XCreateBitmapFromData( flx->display, fl_root,
								   ( char * ) gray50_bits, 8, 8 );
		fli_gray_pattern[ 2 ] =
			XCreateBitmapFromData( flx->display, fl_root,
								   ( char * ) gray60_bits, 8, 8 );
    }
}


/***************************************
 * must NOT allow object pixmap and form pixmap active at the
 * same time
 ***************************************/

static void
change_drawable( FL_pixmap * p,
				 FL_OBJECT * ob )
{
    p->x = ob->x;
    p->y = ob->y;
    p->win = FL_ObjWin( ob );
    ob->form->window = p->pixmap;
    ob->x = 0;
    ob->y = 0;
    fl_winset( p->pixmap );
}


/***************************************
 ***************************************/

static void
change_form_drawable( FL_pixmap * p,
					  FL_FORM   * form )
{
    p->x = form->x;
    p->y = form->y;
    p->win = form->window;
    form->window = p->pixmap;
    form->x = 0;
    form->y = 0;
    fl_winset( p->pixmap );
}


/***************************************
 ***************************************/

static int
fl_xerror_handler( Display     * d  FL_UNUSED_ARG,
				   XErrorEvent * xev )
{
    if ( xev->error_code == BadAlloc )
		M_err( "XErrorHandler", "XError: can't allocate - ignored " );
    else
		M_err( "XErrorHandler", "XError: %d", xev->error_code );

    return 0;

}


/* non-square box can't be double buffered */

#define NON_SQB( a )  ( ( a )->boxtype == FL_NO_BOX )

/* Pixmap support   */


/***************************************
 ***************************************/

void
fli_create_object_pixmap( FL_OBJECT * ob )
{
    Window root;
    unsigned int junk;
    FL_pixmap *p;
    int i;
	int ( * oldhandler )( Display *, XErrorEvent * );

    /* Check to see if we need to create a pixmap. None-square boxes can't
	   be used as it is not easy to figure out the object color beneath the
	   object we are trying to paint */

    if (    ob->w <= 0
		 || ob->h <= 0
		 || (    ob->form->use_pixmap
			  && ob->form->flpixmap
		      && ( ( FL_pixmap * ) ob->form->flpixmap )->win )
		 || NON_SQB( ob )
		 || ! ob->use_pixmap )
		return;

    if ( ! ( p = ob->flpixmap ) )
		p = ob->flpixmap = fl_calloc( 1, sizeof *p );

    if (    p->pixmap
		 && ( int ) p->w == ob->w
		 && ( int ) p->h == ob->h
		 && p->depth == fli_depth( fl_vmode )
		 && p->visual == fli_visual( fl_vmode )
		 && p->dbl_background == ob->dbl_background
		 && p->pixel == fl_get_pixel( ob->dbl_background ) )
    {
		change_drawable( p, ob );
		return;
    }

    if ( p->pixmap )
		XFreePixmap( flx->display, p->pixmap );

    oldhandler = XSetErrorHandler( fl_xerror_handler );

    p->pixmap = XCreatePixmap( flx->display, FL_ObjWin( ob ), ob->w, ob->h,
							   fli_depth( fl_vmode ) );

    fl_winset( p->pixmap );
    fl_rectf( 0, 0, ob->w, ob->h, ob->dbl_background );

    M_info( "ObjPixmap", "Creating depth=%d for %s",
			fli_depth( fl_vmode ), ob->label ? ob->label : "unknown");

    /* Make sure it succeeds by forcing a two way request */

    if (    fli_cntl.safe
		 && ! XGetGeometry( flx->display, p->pixmap, &root, &i,
							&i, &junk, &junk, &junk, &junk ) )
    {
		M_err( "ObjPixmap", "Can't create" );
		p->pixmap = None;
		XSetErrorHandler( oldhandler );
		return;
    }

    XSetErrorHandler( oldhandler );

    p->w = ob->w;
    p->h = ob->h;
    p->depth = fli_depth( fl_vmode );
    p->visual = fli_visual( fl_vmode );
    p->dbl_background = ob->dbl_background;
    p->pixel = fl_get_pixel( ob->dbl_background );
    change_drawable( p, ob );
}


/***************************************
 ***************************************/

void
fli_show_object_pixmap( FL_OBJECT * ob )
{
    FL_pixmap *p = ob->flpixmap;

    if ( ! p || ! p->pixmap || ! p->win || NON_SQB( ob ) )
		return;

	XCopyArea( flx->display, p->pixmap, p->win, flx->gc,
			   0, 0, p->w, p->h, p->x, p->y );

	ob->x = p->x;
	ob->y = p->y;
	fl_winset( p->win );
	ob->form->window = p->win;
	p->win = None;

	/* Now handle the label */

	fli_handle_object( ob, FL_DRAWLABEL, 0, 0, 0, 0 );
}


/***************************************
 ***************************************/

void
fli_free_flpixmap( FL_pixmap * p )
{
    if ( p && p->pixmap )
    {
		XFreePixmap( flx->display, p->pixmap );
		p->pixmap = None;
    }
}


/***************************************
 ***************************************/

static int
form_pixmapable( FL_FORM * form )
{
    /* Check to see if we need to create a pixmap. None-square boxes can't
	   be used as it is not easy to figure out the object color beneath the
	   object we are trying to paint. Take care, sometimes a form can have
	   a fake NO_BOX as the first object */

	return form->use_pixmap
		   && (    form->first
				&& (    ! NON_SQB( form->first )
					 || (    form->first->next
					      && ! NON_SQB( form->first->next ) ) ) );
}


/***************************************
 ***************************************/

void
fli_create_form_pixmap( FL_FORM * form )
{
    Window root;
    unsigned int junk;
    FL_pixmap *p;
    int i;
	int ( * oldhandler )( Display *, XErrorEvent * );

    if ( form->w <= 0 || form->h <= 0 || ! form_pixmapable( form ) )
		return;

    if ( ! ( p = form->flpixmap ) )
		p = form->flpixmap = fl_calloc( 1, sizeof *p );

    if (    p->pixmap
		 && ( int ) p->w == form->w
		 && ( int ) p->h == form->h
		 && p->depth == fli_depth( fl_vmode )
		 && p->visual == fli_visual( fl_vmode ) )
    {
		change_form_drawable( p, form );
		return;
    }

    if ( p->pixmap )
		XFreePixmap( flx->display, p->pixmap );

    oldhandler = XSetErrorHandler( fl_xerror_handler );

    p->pixmap = XCreatePixmap( flx->display, form->window,
							   form->w, form->h,
							   fli_depth( fl_vmode ) );

    M_info( "FormPixmap", "creating(w=%d h=%d)", form->w, form->h );

    /* Make sure it succeeds by forcing a two way request */

    if ( ! XGetGeometry( flx->display, p->pixmap, &root, &i, &i,
						 &junk, &junk, &junk, &junk ) )
    {
		M_warn( "FormPixmap", "Can't create pixmap" );
		p->pixmap = None;
		XSetErrorHandler( oldhandler );
		return;
    }

    XSetErrorHandler( oldhandler );

    p->w = form->w;
    p->h = form->h;
    p->depth = fli_depth( fl_vmode );
    p->visual = fli_visual( fl_vmode );
    change_form_drawable( p, form );

    M_info( "FormPixmap", "Creation Done" );
}


/***************************************
 ***************************************/

void
fli_show_form_pixmap( FL_FORM * form )
{
    FL_pixmap *p = form->flpixmap;

    if (    ! form_pixmapable( form )
		 || ! p
		 || ! p->pixmap
		 || ! p->win
		 || p->w <= 0
		 || p->h <= 0 )
		return;

    XCopyArea( flx->display, p->pixmap, p->win, flx->gc,
			   0, 0, p->w, p->h, 0, 0 );

    form->x = p->x;
    form->y = p->y;
    form->window = p->win;
    fl_winset( p->win );
    p->win = None;
}


/********  VisualClass name *************{*/

#define VNP( a )   { a, #a }

static FLI_VN_PAIR xvclass[ ] =
{
    VNP( PseudoColor ),
	VNP( TrueColor ),
	VNP( DirectColor ),
	VNP( StaticColor ),
    VNP( GrayScale ),
	VNP( GreyScale ),
	VNP( StaticGray ),
	VNP( StaticGrey ),
    { FL_DefaultVisual, "DefaultVisual" },
    { FL_DefaultVisual, "default"       },
    { FL_DefaultVisual, "Default"       },
    { FL_IllegalVisual, "XInvalidClass" },
    { -1,               "Invalid"       }
};


/***************************************
 ***************************************/

const char *
fl_vclass_name( int n )
{
    return fli_get_vn_name( xvclass, n );
}


/***************************************
 * fli_get_vn_val can't be used. caller relies on the returning
 * of FL_IllegalVisual
 ***************************************/

int
fl_vclass_val( const char * v )
{
    FLI_VN_PAIR *vn = xvclass;

    for ( ; vn->val >= 0 && v; vn++ )
		if ( strcmp( vn->name, v ) == 0 )
			return vn->val;

    return FL_IllegalVisual;
}
