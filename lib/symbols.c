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
 * \file symbols.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *
 *  Predefined symbols and rotuines for adding more symbols
 *
 *   Public routines:
 *
 *    fl_add_symbol(const char *name, FL_DRAWPTR how, int scalable);
 *    fl_draw_symbol(const char *name, x, y, w, h, FL_COLOR col)
 */


#if defined F_ID || defined DEBUG
char *fl_id_syml = "$Id: symbols.c,v 1.10 2008/12/27 22:20:52 jtt Exp $";
#endif


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"

#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef __EMX__
#include <float.h>
#endif

#ifndef M_PI
#define M_PI    3.141592654
#endif


typedef struct
{
    FL_DRAWPTR   drawit;		/* how to draw it              */
    char       * name;			/* symbol name                 */
	int          scalable;  	/* currently unused            */
} SYMBOL;

static SYMBOL * symbols = NULL;		/* The symbols */
static size_t nsymbols = 0;

#define swapit( type, a, b )  \
	do { type a_;             \
		 a_ = ( a );          \
		 a = ( b );           \
		 b = a_;              \
	} while ( 0 )

#define AddPoint( p, xp, yp )   \
	do { p->x = ( xp );         \
		 p->y = ( yp );         \
		 p++;                   \
	} while ( 0 )

#define ShrinkBox( x, y, w, h, d )  \
	do { x += ( d );                \
		 y += ( d );                \
		 w -= 2 * ( d );            \
		 h -= 2 * ( d );            \
	} while ( 0 )



/***************************************
 * check if symbol exsits and return it
 ***************************************/

static SYMBOL *
find_symbol( const char *name )
{
    size_t i;

	for ( i = 0; i < nsymbols; i++ )
		if ( ! strcmp( symbols[ i ].name, name ) )
			break;

    return i < nsymbols ? symbols + i : NULL;
}


/******************* PUBLIC ROUTINES ******************{**/

/***************************************
 ***************************************/

int
fl_add_symbol( const char * name,
			   FL_DRAWPTR   drawit,
			   int          scalable  FL_UNUSED_ARG )
{
    SYMBOL *s;

    if ( ! name || ! *name || ! drawit )
		return -1;

	if ( ! ( s = find_symbol( name ) ) )
	{
		symbols = fl_realloc( symbols, ++nsymbols * sizeof *symbols );
		s = symbols + nsymbols - 1;
		s->name = fl_strdup( name );
	}

    s->drawit   = drawit;
	s->scalable = scalable;
    return 1;
}


/***************************************
 * Draws the symbol with the given label
 ***************************************/

int
fl_draw_symbol( const char * label,
				FL_Coord     x,
				FL_Coord     y,
				FL_Coord     w,
				FL_Coord     h,
				FL_COLOR     col )
{
    int pos,
		shift,
		equalscale = 0;
    FL_Coord dx = 0,
		     dy = 0;
    int rotated = 0,
		delta = 0;
    short defr[ ] = { 0, 225, 270, 315, 180, 0, 0, 135, 90, 45 };
    SYMBOL *s;

    if ( ! label || *label != '@' )
		return 0;

    fli_init_symbols( );

    pos = 1;
    while (    ( label[ pos ] == '-' && isdigit( ( int ) label[ pos + 1 ] ) )
			|| ( label[ pos ] == '+' && isdigit( ( int ) label[ pos + 1 ] ) )
			|| label[ pos ] == '#' )
    {
		switch ( label[ pos ] )
		{
			case '+':
				delta = '0' - label[ ++pos ];
				break;

			case '-':
				delta = label[ ++pos ] - '0';
				break;

			case '#':
				equalscale = 1;
				break;
		}

		pos++;
    }

    shift = pos;

    if ( label[ pos ] >= '1' && label[ pos ] <= '9' )
    {
		rotated = defr[ label[ pos ] - '0' ];
		shift = pos + 1;
    }
    else if ( label[ pos ] == '0' )
    {
		rotated =   100 * ( label[ pos + 1 ] - '0' )
			      +  10 * ( label[ pos + 2 ] - '0' )
			      +   1 * ( label[ pos + 3 ] - '0' );
		shift = pos + 4;
    }

    /* need to special-casing labels like "@4" etc. */

    if ( ! ( s = label[ shift ] ? find_symbol( label + shift ) : symbols ) )
    {
		char *newlabel = fl_strdup( label );

		/* need to replace the first @ otherwise bad recursion */

		newlabel[ 0 ] = ' ';
		M_err( "DrawSymbol", "Bad symbol:@%s", newlabel + 1 );
		fl_free( newlabel );
		return 0;
    }

    if ( equalscale )
    {
		dx = w > h ? ( w - h ) / 2 : 0;
		dy = w > h ? 0 : ( h - w ) / 2;
		w = h = FL_min( w, h );
    }

    if ( delta )
		ShrinkBox( x, y, w, h, delta );

    /* also if rotated 90 degrees, switch w, h and the bounding box. TODO  */

    if ( rotated == 90 || rotated == 270 )
    {
		x += ( w - h ) / 2;
		y += ( h - w ) / 2;
		swapit( FL_Coord, w, h );
    }

    s->drawit( x + dx, y + dy, w, h, rotated, col );
    return 1;
}


/*********** END of PUBLIC ROTUINES ***********}**/


/***************************************
 ***************************************/

static void
rotate_it( FL_Coord xc,
		   FL_Coord yc,
		   FL_POINT xpnts[ ],
		   int      i,
		   int      a )
{
    FL_POINT *xp = xpnts,
		     *xps = xpnts + i;
    double tmpx,
		   tmpy;
    int tmp;

    if ( a == 0 || a == 360 )
		return;

    if ( a == 180 )
		for ( ; xp < xps; xp++ )
			xp->x = 2 * xc - xp->x;
    else if ( a == 90 )
		for ( ; xp < xps; xp++ )
		{
			tmp = xp->x;
			xp->x = xc + xp->y - yc;
			xp->y = yc - tmp + xc;
		}
    else if ( a == 270 )
		for ( ; xp < xps; xp++ )
	{
	    tmp = xp->x;
	    xp->x = xc + xp->y - yc;
	    xp->y = yc + tmp - xc;
	}

#define FACT  0.707106781187	/* sin45 */
    else if ( a == 45 )
		for ( ; xp < xps; xp++ )
		{
			tmpx = xp->x - xc;
			tmpy = xp->y - yc;
			xp->x = FL_nint( ( tmpx + tmpy ) * FACT + xc );
			xp->y = FL_nint( ( - tmpx + tmpy ) * FACT + yc );
		}
    else if ( a == 135 )
		for ( ; xp < xps; xp++ )
		{
			tmpx = xp->x - xc;
			tmpy = xp->y - yc;
			xp->x = FL_nint( ( - tmpx + tmpy ) * FACT + xc );
			xp->y = FL_nint( ( - tmpx - tmpy ) * FACT + yc );
		}
    else if ( a == 225 )
		for ( ; xp < xps; xp++ )
		{
			tmpx = xp->x - xc;
			tmpy = xp->y - yc;
			xp->x = FL_nint( ( - tmpx - tmpy ) * FACT + xc );
			xp->y = FL_nint( ( tmpx - tmpy ) * FACT + yc );
		}
    else if ( a == 315 )
		for ( ; xp < xps; xp++ )
		{
			tmpx = xp->x - xc;
			tmpy = xp->y - yc;
			xp->x = FL_nint( ( tmpx - tmpy ) * FACT + xc );
			xp->y = FL_nint( ( tmpx + tmpy ) * FACT + yc );
		}
    else
    {
		double sinfact = sin( a * M_PI / 180.0 );
		double cosfact = cos( a * M_PI / 180.0 );

		for ( ; xp < xps; xp++ )
		{
			tmpx = xp->x - xc;
			tmpy = xp->y - yc;
			xp->x = FL_nint( xc + tmpx * cosfact + tmpy * sinfact );
			xp->y = FL_nint( yc - tmpx * sinfact + tmpy * cosfact );
		}
    }
}


/******************** THE DEFAULT SYMBOLS ****************************/


/***************************************
 ***************************************/

static void
draw_returnarrow( FL_Coord x,
				  FL_Coord y,
				  FL_Coord w,
				  FL_Coord h,
				  int      angle  FL_UNUSED_ARG,
				  FL_COLOR col )
{
    double wm = w * 0.5,
		   hm = h * 0.5;
    int xc = FL_nint( x + wm ),
		yc = FL_nint( y + hm );
    FL_POINT xpoint[ 8 ],
		     *xp;

    xp = xpoint;

    AddPoint( xp, xc - 0.8 * wm, yc );
    AddPoint( xp, xc - 0.1 * wm, yc - 0.6 * hm );
    AddPoint( xp, xc - 0.1 * wm, yc + 0.6 * hm );

    fl_polyf( xpoint, 3, col );

    /* trailing line */

    xp = xpoint + 4;

    AddPoint( xp, xc - 0.1 * wm, yc );
    AddPoint( xp, xc + 0.8 * wm, yc );
    AddPoint( xp, xc + 0.8 * wm, yc - 0.7 * hm );

    fl_lines( xpoint + 4, 3, col );

    fl_polyl( xpoint, 3, FL_BLACK );
}


/***************************************
 * Thin arrow  -->
 ***************************************/

static void
draw_arrow( FL_Coord x,
			FL_Coord y,
			FL_Coord w,
			FL_Coord h,
			int      angle,
			FL_COLOR col )
{
    int xc,
		yc,
		dx,
		dy;
    FL_POINT xpoint[ 5 ],
		     *xp;
    int d = 3 + ( w + h ) * 0.03;

    xc = x + w / 2;
    yc = y + h / 2;
    ShrinkBox( x, y, w, h, d );

    dx = 0.35 * w;
    dy = 0.08 * h;
    if ( dy < 1 )
		dy = 1;

    xp = xpoint;
    AddPoint( xp, xc + dx, yc - dy );
    AddPoint( xp, x + w - 1, yc );
    AddPoint( xp, xc + dx, yc + dy );

    rotate_it( xc, yc, xpoint, 3, angle );

    fl_polyf( xpoint, 3, col );
    fl_polyl( xpoint, 3, FL_BLACK );

    xp = xpoint;
    AddPoint( xp, x, yc );
    AddPoint( xp, xc + dx, yc );
    AddPoint( xp, xc + dx, yc + 2 );
    AddPoint( xp, x, yc + 2 );

    rotate_it( xc, yc, xpoint, 4, angle );

    fl_polyf( xpoint, 4, FL_BLACK );
}


/***************************************
* An arrow ->
 ***************************************/

static void
draw_arrow1( FL_Coord x,
			 FL_Coord y,
			 FL_Coord w,
			 FL_Coord h,
			 int      angle,
			 FL_COLOR col )
{
    double wm = ( w - 4 ) * 0.5,
		   hm = ( h - 4 ) * 0.5;
    int xc = x + w / 2,
		yc = y + h / 2;
    FL_POINT xpoint[ 8 ],
		     *xp;
    double pl = 0.8,
		   ps = 0.4;
    int pshm = ps * hm + 0.1,
		plwm = pl * wm + 0.1,
		plhm = pl * hm + 0.1;

    xp = xpoint;

    AddPoint( xp, xc - plwm, yc + pshm );
    AddPoint( xp, xc - plwm, yc - pshm );
    AddPoint( xp, xc, yc - pshm );
    AddPoint( xp, xc, yc - plhm );
    AddPoint( xp, xc + plwm, yc );
    AddPoint( xp, xc, yc + plhm );
    AddPoint( xp, xc, yc + pshm );

    rotate_it( xc, yc, xpoint, 7, angle );

    fl_polyf( xpoint, 7, col );
    fl_polyl( xpoint, 7, FL_BLACK );
}


/***************************************
 *  An arrow head >
 ***************************************/

static void
draw_arrow2( FL_Coord x,
			 FL_Coord y,
			 FL_Coord w,
			 FL_Coord h,
			 int      angle,
			 FL_COLOR col )
{
    int xc = x + w / 2,
		yc = y + h / 2;
    FL_POINT xpoint[ 4 ],
		     *xp;
    double wm = ( w - 4 ) * 0.5,
		   hm = ( h - 4 ) * 0.5;
    double pl = 0.8,
		   ps = 0.3;
    int pswm = ps * wm + 0.1,
		plhm = pl * hm + 0.1;

    xp = xpoint;

    AddPoint( xp, xc - pswm, yc - plhm );
    AddPoint( xp, xc + 0.5 * wm, yc );
    AddPoint( xp, xc - pswm, yc + plhm );

    rotate_it( xc, yc, xpoint, 3, angle );

    fl_polyf( xpoint, 3, col );
    fl_polyl( xpoint, 3, FL_BLACK );
}


/***************************************
 * double arrow head >>
 ***************************************/

static void
draw_arrow3( FL_Coord x,
			 FL_Coord y,
			 FL_Coord w,
			 FL_Coord h,
			 int      angle,
			 FL_COLOR col )
{
    int xc = x + w / 2 - 1,
		yc = y + h / 2;
    FL_POINT xpoint[ 9 ],
		     *xp;
    double wm = ( w - 4 ) * 0.5,
		   hm = ( h - 4 ) * 0.5;
    double pl = 0.7,
		   ps = 0.15;
    int pswm = ps * wm + 0.1,
		plhm = pl * hm + 0.1;

    xp = xpoint;

    AddPoint( xp, xc + pswm, yc - plhm );
    AddPoint( xp, xc + 0.82 * wm, yc );
    AddPoint( xp, xc + pswm, yc + plhm );

    rotate_it( xc, yc, xpoint, 3, angle );

    pswm = 0.55 * wm + 0.1;
    xp = xpoint + 5;

    AddPoint( xp, xc - pswm, yc - plhm );
    AddPoint( xp, xc + 0.12 * wm, yc );
    AddPoint( xp, xc - pswm, yc + plhm );

    rotate_it( xc, yc, xpoint + 5, 3, angle );

    fl_polyf( xpoint, 3, col );
    fl_polyf( xpoint + 5, 3, col );
    fl_polyl( xpoint, 3, FL_BLACK );
    fl_polyl( xpoint + 5, 3, FL_BLACK );
}


/***************************************
 ***************************************/

static void
draw_arrow01( FL_Coord x,
			  FL_Coord y,
			  FL_Coord w,
			  FL_Coord h,
			  int      angle,
			  FL_COLOR col )
{
    if ( ( angle += 180 ) > 360 )
		angle -= 360;
    draw_arrow1( x, y, w, h, angle, col );
}


/***************************************
 ***************************************/

static void
draw_arrow02( FL_Coord x,
			  FL_Coord y,
			  FL_Coord w,
			  FL_Coord h,
			  int      angle,
			  FL_COLOR col )
{
    if ( ( angle += 180 ) > 360 )
		angle -= 360;
    draw_arrow2( x, y, w, h, angle, col );
}


/***************************************
 ***************************************/

static void
draw_arrow03( FL_Coord x,
			  FL_Coord y,
			  FL_Coord w,
			  FL_Coord h,
			  int      angle,
			  FL_COLOR col )
{
    if ( ( angle += 180 ) > 360 )
		angle -= 360;
    draw_arrow3( x, y, w, h, angle, col );
}


/***************************************
 ***************************************/

static void
draw_circle( FL_Coord x,
			 FL_Coord y,
			 FL_Coord w,
			 FL_Coord h,
			 int      angle  FL_UNUSED_ARG,
			 FL_COLOR col )
{
    FL_Coord xo = x + w / 2,
		     yo = y + h / 2;
    int rr,
		s = 3 + 0.04 * ( w + h );

    ShrinkBox( x, y, w, h, s );

    if ( ( rr = 0.5 * FL_min( w, h ) ) <= 0 )
		rr = 1;

    fl_circf( xo, yo, rr, col );
    fl_circ( xo, yo, rr, FL_BLACK );
}


/***************************************
 ***************************************/

static void
draw_square( FL_Coord x,
			 FL_Coord y,
			 FL_Coord w,
			 FL_Coord h,
			 int      angle  FL_UNUSED_ARG,
			 FL_COLOR col )
{
    int s = ( 0.09 * w ) + 3;

    ShrinkBox( x, y, w, h, s );

    if ( w <= 1 )
		w = 2;

    if ( h <= 1 )
		h = 2;

    fl_rectbound( x, y, w - 1, h - 1, col );
}


/***************************************
 ***************************************/

static void
draw_plus( FL_Coord x,
		   FL_Coord y,
		   FL_Coord w,
		   FL_Coord h,
		   int      angle,
		   FL_COLOR col )
{
    int wm = ( w - 4 ) * 0.5,
		hm = ( h - 4 ) * 0.5;
    int xc = x + 2 + wm,
		yc = y + 2 + hm;
    int plw = FL_nint( 0.8  * wm ),
		psh = FL_nint( 0.15 * hm );
    int psw = FL_nint( 0.15 * wm ),
		plh = FL_nint( 0.8  * hm );
    FL_POINT xpoint[ 13 ],
		     *xp;

    xp = xpoint;

    AddPoint( xp, xc - plw, yc + psh );
    AddPoint( xp, xc - plw, yc - psh );
    AddPoint( xp, xc - psw, yc - psh );
    AddPoint( xp, xc - psw, yc - plh );
    AddPoint( xp, xc + psw, yc - plh );
    AddPoint( xp, xc + psw, yc - psh );
    AddPoint( xp, xc + plw, yc - psh );
    AddPoint( xp, xc + plw, yc + psh );
    AddPoint( xp, xc + psw, yc + psh );
    AddPoint( xp, xc + psw, yc + plh );
    AddPoint( xp, xc - psw, yc + plh );
    AddPoint( xp, xc - psw, yc + psh );

    rotate_it( xc, yc, xpoint, 12, angle );

    fl_polyf( xpoint, 12, col );
    fl_polyl( xpoint, 12, FL_BLACK );
}


/***************************************
 ***************************************/

static void
draw_menu( FL_Coord x,
		   FL_Coord y,
		   FL_Coord w,
		   FL_Coord h,
		   int      angle  FL_UNUSED_ARG,
		   FL_COLOR col )
{
    FL_Coord wm = ( w - 8 ) * 0.5,
		     hm = ( h - 8 ) * 0.5;
    int xc = x + w / 2,
		yc = y + h / 2;
    int dx = FL_nint( 0.6 * wm ),
		cur_x,
		cur_y;
    int shadow = FL_max( 2, 0.1 * FL_min( w, h ) ),
		t = FL_min( 3, 0.30 * hm );

    cur_x = xc - dx;
    fl_rectbound( cur_x, yc - hm + 1, 2 * dx, t, col );

    cur_y = yc - hm + t + t;
    fl_rectf( cur_x + shadow, cur_y + shadow, 2 * dx, 1.6 * hm, FL_RIGHT_BCOL );
    fl_rectbound( cur_x, cur_y, 2 * dx, 1.6 * hm, col );
}


/***************************************
 ***************************************/

static void
draw_line( FL_Coord x,
		   FL_Coord y,
		   FL_Coord w,
		   FL_Coord h,
		   int      angle,
		   FL_COLOR col )
{
    FL_POINT xpoint[ 3 ],
		     *xp;
    FL_Coord xc = x + w / 2,
		     yc = y + h / 2;

    ShrinkBox( x, y, w, h, 3     /* FL_abs( FL_BOUND_WIDTH ) */ );

    xp = xpoint;

    AddPoint( xp, x, yc );
    AddPoint( xp, x + w - 1, yc );

    rotate_it( xc, yc, xpoint, 2, angle );

    fl_line( xpoint[ 0 ].x, xpoint[ 0 ].y, xpoint[ 1 ].x, xpoint[ 1 ].y, col );
}


/***************************************
 ***************************************/

static void
draw_ripplelines( FL_Coord x,
				  FL_Coord y,
				  FL_Coord w,
				  FL_Coord h,
				  int      angle,
				  FL_COLOR col  FL_UNUSED_ARG )
{
    int ym = y + ( h + 1 ) / 2,
		xm = x + ( w + 1 ) / 2;
    int xs,
		ys,
		i,
		mw = 3;	/* FL_abs( FL_BOUND_WIDTH ); */

    xs = xm - 5;
    ys = ym - 5;

    if ( angle == 0 || angle == 180 )
		for ( i = 0; i < 3; i++ )
		{
			fl_line( x + mw, ys, x + w - 1 - mw, ys, FL_LEFT_BCOL );
			ys += 1;
			fl_line( x + mw, ys, x + w - 1 - mw, ys, FL_RIGHT_BCOL );
			ys += 3;
		}
    else if ( angle == 90 || angle == 270 )
    {
		int e;

		y += ( h - w ) / 2;
		swapit( FL_Coord, w, h );

		e = h < 15;
		for ( i = 0; i < 3; i++ )
		{
			fl_line( xs, y + mw - e, xs, y + h - 1 - mw + e, FL_LEFT_BCOL );
			xs += 1;
			fl_line( xs, y + mw - e, xs, y + h - 1 - mw + e, FL_RIGHT_BCOL );
			xs += 3;
		}
    }
    else
		fprintf( stderr, "RippleLine: unsupported angle %d\n", angle );
}


/***************************************
 * draw a line that appears down
 ***************************************/

static void
draw_dnline( FL_Coord x,
			 FL_Coord y,
			 FL_Coord w,
			 FL_Coord h,
			 int      angle,
			 FL_COLOR col  FL_UNUSED_ARG )
{
    FL_POINT xpnt[ 3 ],
		     *xp;
    FL_Coord yc = y + h / 2;

    ShrinkBox( x, y, w, h, 3 );

    xp = xpnt;

    AddPoint( xp, x, yc );
    AddPoint( xp, x + w - 1, yc );

    rotate_it( x + w / 2, yc, xpnt, 2, angle );

    fl_line( xpnt[ 0 ].x, xpnt[ 0 ].y, xpnt[ 1 ].x, xpnt[ 1 ].y,
			 FL_RIGHT_BCOL );

    xp = xpnt;

    AddPoint( xp, x, yc + 1 );
    AddPoint( xp, x + w - 1, yc + 1 );

    rotate_it( x + w / 2, yc, xpnt, 2, angle );

    fl_line( xpnt[ 0 ].x, xpnt[ 0 ].y, xpnt[ 1 ].x, xpnt[ 1 ].y, FL_LEFT_BCOL );
}


/***************************************
 ***************************************/

static void
draw_upline( FL_Coord x,
			 FL_Coord y,
			 FL_Coord w,
			 FL_Coord h,
			 int      angle,
			 FL_COLOR col  FL_UNUSED_ARG )
{
    FL_POINT xpnt[ 3 ],
		     *xp;
    FL_Coord yc = y + h / 2;

    ShrinkBox( x, y, w, h, 3 );

    xp = xpnt;

    AddPoint( xp, x, yc );
    AddPoint( xp, x + w - 1, yc );

    rotate_it( x + w / 2, yc, xpnt, 2, angle );

    fl_line( xpnt[ 0 ].x, xpnt[ 0 ].y, xpnt[ 1 ].x, xpnt[ 1 ].y, FL_LEFT_BCOL );

    xp = xpnt;

	AddPoint( xp, x, yc + 1 );
    AddPoint( xp, x + w - 1, yc + 1 );

    rotate_it( x + w / 2, yc, xpnt, 2, angle );

    fl_line( xpnt[ 0 ].x, xpnt[ 0 ].y, xpnt[ 1 ].x, xpnt[ 1 ].y,
			 FL_RIGHT_BCOL );
}


/***************************************
 ***************************************/

static void
draw_uparrow( FL_Coord x,
			  FL_Coord y,
			  FL_Coord w,
			  FL_Coord h,
			  int      a,
			  FL_COLOR col  FL_UNUSED_ARG )
{
    FL_Coord xc = x + ( w + 1 ) / 2,
		     dx;
    FL_Coord yc = y + ( h + 1 ) / 2,
		     dy;
    int d = 3 + ( w + h ) * 0.06;

    ShrinkBox( x, y, w, h, d );

    if ( a == 90 )
    {
		/* undo driver's transformation */

		swapit( FL_Coord, w, h );
		x -= ( w - h ) / 2;
		y -= ( h - w ) / 2;
		dx = w / 2;
		dy = h / 2;

		/* correct for human perception error */

		y++;
		h -= 2;

		fl_line( xc, yc - dy, xc - dx, yc + dy, FL_LEFT_BCOL );
		fl_line( xc, yc - dy, xc + dx, yc + dy, FL_RIGHT_BCOL );
		fl_line( xc - dx, yc + dy, xc + dx, yc + dy, FL_BOTTOM_BCOL );

    }
    else if ( a == 270 )
    {
		swapit( FL_Coord, w, h );
		x -= ( w - h ) / 2;
		y -= ( h - w ) / 2;
		dx = w / 2;
		dy = h / 2;

		fl_line( xc - dx, yc - dy, xc + dx, yc - dy, FL_TOP_BCOL );
		fl_line( xc + dx, yc - dy, xc, yc + dy, FL_RIGHT_BCOL );
		fl_line( xc, yc + dy, xc - dx, yc - dy, FL_LEFT_BCOL );
    }
    else if ( a == 180 )
    {
		dy = h / 2;
		dx = w / 2;

		fl_line( xc - dx, yc, xc + dx, yc - dy, FL_LEFT_BCOL );
		fl_line( xc + dx, yc - dy, xc + dx, yc + dy, FL_RIGHT_BCOL );
		fl_line( xc + dx, yc + dy, xc - dx, yc, FL_BOTTOM_BCOL );
    }
    else
    {
		dx = w / 2;
		dy = h / 2;
		fl_line( xc - dx, yc - dy, xc + dx, yc,
				 fli_dithered( fl_vmode ) ? FL_BLACK : FL_TOP_BCOL );
		fl_line( xc - dx, yc + dy, xc + dx, yc, FL_RIGHT_BCOL );
		fl_line( xc - dx, yc - dy, xc - dx, yc + dy,
				 fli_dithered( fl_vmode ) ? FL_BLACK : FL_LEFT_BCOL );
    }
}


/***************************************
 ***************************************/

static void
draw_dnarrow( FL_Coord x,
			  FL_Coord y,
			  FL_Coord w,
			  FL_Coord h,
			  int      a,
			  FL_COLOR col  FL_UNUSED_ARG )
{
    FL_Coord xc = x + ( w + 1 ) / 2,
		     dx;
    FL_Coord yc = y + ( h + 1 ) / 2,
		     dy;
    int d = 3 + ( w + h ) * 0.06;

    ShrinkBox( x, y, w, h, d );

    if ( a == 90 )
    {
		/* undo driver's transformation */

		swapit( FL_Coord, w, h );
		x -= ( w - h ) / 2;
		y -= ( h - w ) / 2;
		dx = w / 2;
		dy = h / 2;

		/* correct for human perception error */

		y++;
		h -= 2;

		fl_line( xc, yc - dy, xc - dx, yc + dy, FL_RIGHT_BCOL );
		fl_line( xc, yc - dy, xc + dx, yc + dy, FL_LEFT_BCOL );
		fl_line( xc - dx, yc + dy, xc + dx, yc + dy, FL_TOP_BCOL );
    }
    else if ( a == 270 )
    {
		swapit( FL_Coord, w, h );
		x -= ( w - h ) / 2;
		y -= ( h - w ) / 2;
		dx = w / 2;
		dy = h / 2;

		fl_line( xc - dx, yc - dy, xc + dx, yc - dy, FL_BOTTOM_BCOL );
		fl_line( xc + dx, yc - dy, xc, yc + dy, FL_LEFT_BCOL );
		fl_line( xc, yc + dy, xc - dx, yc - dy, FL_RIGHT_BCOL );
    }
    else if ( a == 180 )
    {
		dy = h / 2;
		dx = w / 2;
		fl_line( xc - dx, yc, xc + dx, yc - dy, FL_RIGHT_BCOL );
		fl_line( xc + dx, yc - dy, xc + dx, yc + dy, FL_LEFT_BCOL );
		fl_line( xc + dx, yc + dy, xc - dx, yc, FL_BOTTOM_BCOL );
    }
    else
    {
		dx = w / 2;
		dy = h / 2;
		fl_line( xc - dx, yc - dy, xc - dx, yc + dy, FL_RIGHT_BCOL );
		fl_line( xc - dx, yc - dy, xc + dx, yc, FL_RIGHT_BCOL );
		fl_line( xc - dx, yc + dy, xc + dx, yc, FL_TOP_BCOL );
    }
}


/***************************************
 * double arrow <-->. Partition the space into 1/4 1/2 1/4
 ***************************************/

static void
draw_doublearrow( FL_Coord x,
				  FL_Coord y,
				  FL_Coord w,
				  FL_Coord h,
				  int      angle,
				  FL_COLOR col )
{
    int xc = x + w / 2,
		yc = y + h / 2;
    double wm = ( w - 4 ) * 0.5,
		   hm = ( h - 4 ) * 0.5;
    int dx1 = 0.5 * wm + 0.2,
		dx2 = 0.9 * wm + 0.2;
    int dy1 = 0.3 * hm + 0.2,
		dy2 = 0.7 * hm + 0.2;
    FL_POINT xpoint[ 11 ],
		     *xp;

    xp = xpoint;

    AddPoint( xp, xc - dx1, yc - dy1 );
    AddPoint( xp, xc + dx1, yc - dy1 );
    AddPoint( xp, xc + dx1, yc - dy2 );
    AddPoint( xp, xc + dx2, yc );
    AddPoint( xp, xc + dx1, yc + dy2 );
    AddPoint( xp, xc + dx1, yc + dy1 );
    AddPoint( xp, xc - dx1, yc + dy1 );
    AddPoint( xp, xc - dx1, yc + dy2 );
    AddPoint( xp, xc - dx2, yc );
    AddPoint( xp, xc - dx1, yc - dy2 );

    rotate_it( xc, yc, xpoint, 10, angle );

    fl_polyf( xpoint, 10, col );
    fl_polyl( xpoint, 10, FL_BLACK );
}



/***************************************
 * an arrow with a bar  ->|
 ***************************************/

static void
draw_arrowbar( FL_Coord x,
			   FL_Coord y,
			   FL_Coord w,
			   FL_Coord h,
			   int      angle,
			   FL_COLOR col )
{
    double wm = ( w - 6 ) * 0.5,
		   hm = ( h - 6 ) * 0.5;
    int xc = x + w / 2,
		yc = y + h / 2;
    FL_POINT xpoint[ 8 ],
		     *xp;
    double pl = 0.8,
		   ps = 0.4;
    int pshm = ps * hm + 0.1,
		plwm = pl * wm + 0.1,
		plhm = pl * hm + 0.1;

    xp = xpoint;

    xc--;

    AddPoint( xp, xc - plwm, yc + pshm );
    AddPoint( xp, xc - plwm, yc - pshm );
    AddPoint( xp, xc, yc - pshm );
    AddPoint( xp, xc, yc - plhm );
    AddPoint( xp, xc + plwm, yc );
    AddPoint( xp, xc, yc + plhm );
    AddPoint( xp, xc, yc + pshm );

    rotate_it( xc, yc, xpoint, 7, angle );

    fl_polyf( xpoint, 7, col );
    fl_polyl( xpoint, 7, FL_BLACK );

    xp = xpoint;
    xc++;

    AddPoint( xp, xc + plwm + 1, yc + plhm );
    AddPoint( xp, xc + plwm + 1, yc - plhm );
    AddPoint( xp, xc + ( 0.9 * plwm ), yc - plhm );
    AddPoint( xp, xc + ( 0.9 * plwm ), yc + plhm );

    rotate_it( xc, yc, xpoint, 4, angle );

    fl_polyf( xpoint, 4, col );
    fl_polyl( xpoint, 4, FL_BLACK );
}



/***************************************
 * same as arrow bar ->|, but reversed
 ***************************************/

static void
draw_arrowbar0( FL_Coord x,
				FL_Coord y,
				FL_Coord w,
				FL_Coord h,
				int      angle,
				FL_COLOR col )
{
    if ( ( angle += 180 ) >= 360 )
		angle -= 360;
    draw_arrowbar( x, y, w, h, angle, col );
}


/***************************************
 * An arrow head with a bar >|
 ***************************************/

static void
draw_arrowheadbar( FL_Coord x,
				   FL_Coord y,
				   FL_Coord w,
				   FL_Coord h,
				   int      angle,
				   FL_COLOR col )
{
    int xc = x + w / 2,
		yc = y + h / 2;
    FL_POINT xpoint[ 5 ],
		     *xp;
    double wm = ( w - 6 ) * 0.5,
		   hm = ( h - 6 ) * 0.5;
    double pl = 0.8,
		   ps = 0.45;
    int pswm = ps * wm + 0.1,
		plhm = pl * hm + 0.1;

    xp = xpoint;

    AddPoint( xp, xc - pswm, yc - plhm );
    AddPoint( xp, xc + pswm, yc );
    AddPoint( xp, xc - pswm, yc + plhm );
    rotate_it( xc, yc, xpoint, 3, angle );

    fl_polyf( xpoint, 3, col );
    fl_polyl( xpoint, 3, FL_BLACK );

    xp = xpoint;

    AddPoint( xp, xc + pswm + 2, yc + plhm );
    AddPoint( xp, xc + pswm + 2, yc - plhm );
    AddPoint( xp, xc + 0.9 * pswm, yc - plhm );
    AddPoint( xp, xc + 0.9 * pswm, yc + plhm );

    rotate_it( xc, yc, xpoint, 4, angle );

    fl_polyf( xpoint, 4, col );
    fl_polyl( xpoint, 4, FL_BLACK );
}


/***************************************
 * same as arrowheadbar >|, but reversed |<
 ***************************************/

static void
draw_arrowheadbar0( FL_Coord x,
					FL_Coord y,
					FL_Coord w,
					FL_Coord h,
					int      angle,
					FL_COLOR col )
{
    if ( ( angle += 180 ) >= 360 )
		angle -= 360;
    draw_arrowheadbar( x, y, w, h, angle, col );
}


/***************************************
 ***************************************/

static void
draw_bararrowhead( FL_Coord x,
				   FL_Coord y,
				   FL_Coord w,
				   FL_Coord h,
				   int      angle,
				   FL_COLOR col )
{
    int xc = x + ( w + 1 ) / 2,
		yc = y + ( h + 1 ) / 2;
    int dx,
		dy,
		dbar,
		mar,
		xl;
    int d = 3 + ( w + h ) * 0.0;
    FL_POINT point[ 5 ],
		     *p;

    ShrinkBox( x, y, w, h, d );
    dx = w / 2;
    dy = h / 2;
    dbar = dx * 0.4;
    mar = 0.2 * dx;

    xl = xc - dx + 1.1 * mar;

    p = point;

    AddPoint( p, xl, yc - dy );
    AddPoint( p, xl + dbar, yc - dy );
    AddPoint( p, xl + dbar, yc + dy );
    AddPoint( p, xl, yc + dy );

    rotate_it( xc, yc, point, 4, angle );

    fl_polyf( point, 4, col );
    fl_polyl( point, 4, FL_RIGHT_BCOL );

    p = point;

    AddPoint( p, xc - mar, yc - dy );
    AddPoint( p, xc - mar + dx, yc );
    AddPoint( p, xc - mar, yc + dy );

    rotate_it( xc, yc, point, 3, angle );

    fl_polyf( point, 3, col );
    fl_polyl( point, 3, FL_RIGHT_BCOL );
}


/***************************************
 ***************************************/

static void
draw_bararrowhead0( FL_Coord x,
					FL_Coord y,
					FL_Coord w,
					FL_Coord h,
					int      angle,
					FL_COLOR col )
{
    if ( ( angle += 180 ) >= 360 )
		angle -= 360;
    draw_bararrowhead( x, y, w, h, angle, col );
}


/***************************************
 ***************************************/

void
fli_init_symbols( void )
{
	if ( symbols )
		return;

	fl_add_symbol( "",            draw_arrow1,        1 );
	fl_add_symbol( "->",          draw_arrow1,        1 );
	fl_add_symbol( ">",           draw_arrow2,        1 );
	fl_add_symbol( ">>",          draw_arrow3,        1 );
	fl_add_symbol( "<-",          draw_arrow01,       1 );
	fl_add_symbol( "<",           draw_arrow02,       1 );
	fl_add_symbol( "<<",          draw_arrow03,       1 );
	fl_add_symbol( "returnarrow", draw_returnarrow,   1 );
	fl_add_symbol( "circle",      draw_circle,        1 );
	fl_add_symbol( "square",      draw_square,        1 );
	fl_add_symbol( "plus",        draw_plus,          1 );
	fl_add_symbol( "menu",        draw_menu,          1 );
	fl_add_symbol( "line",        draw_line,          1 );
	fl_add_symbol( "=",           draw_ripplelines,   1 );
	fl_add_symbol( "DnLine",      draw_dnline,        1 );
	fl_add_symbol( "UpLine",      draw_upline,        1 );
	fl_add_symbol( "UpArrow",     draw_uparrow,       1 );
	fl_add_symbol( "DnArrow",     draw_dnarrow,       1 );
	fl_add_symbol( "-->",         draw_arrow,         1 );
	fl_add_symbol( "<->",         draw_doublearrow,   1 );
	fl_add_symbol( "->|",         draw_arrowbar,      1 );
	fl_add_symbol( "|<-",         draw_arrowbar0,     1 );
	fl_add_symbol( ">|",          draw_arrowheadbar,  1 );
	fl_add_symbol( "|<",          draw_arrowheadbar0, 1 );
	fl_add_symbol( "|>",          draw_bararrowhead,  1 );
	fl_add_symbol( "<|",          draw_bararrowhead0, 1 );

	/* aliases */

	fl_add_symbol( "arrow",       draw_arrow,         1 );
	fl_add_symbol( "RippleLines", draw_ripplelines,   1 );
	fl_add_symbol( "+",           draw_plus,          1 );
}


/***************************************
 ***************************************/

void
fli_release_symbols( void )
{
	size_t i;

	for ( i = 0; i < nsymbols; i++ )
		fl_safe_free( symbols[ i ].name );
		
	fl_safe_free( symbols );
	nsymbols = 0;
}
