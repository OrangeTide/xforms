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
 * \file sldraw.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 *
 */

#if defined F_ID || defined DEBUG
char *fl_id_sldr = "$Id: sldraw.c,v 1.10 2008/03/25 12:41:29 jtt Exp $";
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pslider.h"		/* for FL_MINKNOB */


typedef FL_SLIDER_SPEC SPEC;


/***************************************
 ***************************************/

static double
flinear( double val,
		 double smin,
		 double smax,
		 double gmin,
		 double gmax )
{
    return smin == smax ?
		   gmax : ( gmin + ( gmax - gmin ) * ( val - smin ) / ( smax - smin ) );
}


/* minimum knob size */

#define MINKNOB_SB    FL_MINKNOB_SB
#define MINKNOB_SL    FL_MINKNOB_SL


#define IS_FLATORDOWN( t )  ( IS_FLATBOX( t ) || t == FL_DOWN_BOX )

#define IS_FLATORUPBOX( t ) ( IS_FLATBOX( t ) || t == FL_UP_BOX )


/***************************************
 ***************************************/

void
fl_calc_slider_size( FL_OBJECT         * ob,
					 FL_SCROLLBAR_KNOB * slb )
{
	SPEC *sp = ob->spec;
	FL_COORD x = sp->x,
		     y = sp->y,
		     w = sp->w,
		     h = sp->h;
	int sltype = ob->type;
	double val = sp->norm_val;
	double size = sp->slsize;
	FL_COORD bw = ob->bw;
    int absbw = FL_abs( bw );
    int fudge1 = IS_FLATORUPBOX( ob->boxtype ) ? 0 : ( bw == -2 || bw >= 2 );
    int fudge2 = IS_FLATORUPBOX( ob->boxtype ) ?
		         0 : ( 2 * ( bw == -2 ) + ( bw >= 2 ) );

    if ( sltype == FL_VERT_FILL_SLIDER )
    {
		int inv = sp->min > sp->max;

		slb->h = ( inv ? 1 - val : val ) * ( h - 2 * absbw );
		slb->y = inv ? y + h - absbw - slb->h : y + absbw;
		slb->w = w - 2 * absbw;
		slb->x = x + absbw;
		return;
    }

    if ( sltype == FL_HOR_FILL_SLIDER )
    {
		slb->w = val * ( w - 2 * absbw );
		slb->x = x + absbw;
		slb->h = h - 2 * absbw;
		slb->y = y + absbw;
		return;
    }

	if ( IS_VSLIDER( sltype ) )
    {
		slb->h = size * ( h - 2 * absbw );
	
		if ( IS_SCROLLBAR( sltype ) && slb->h < MINKNOB_SB )
			slb->h = MINKNOB_SB;
		else if ( ! IS_SCROLLBAR( sltype ) && slb->h < 2 * absbw + MINKNOB_SL )
			slb->h = 2 * absbw + MINKNOB_SL;

		if ( sltype == FL_VERT_BROWSER_SLIDER2 )
		{
			slb->h = size * h;
			slb->y = flinear( val, 0.0, 1.0, y, y + h - slb->h );
			slb->x = x + 1 + IS_FLATORDOWN( ob->boxtype );
			slb->w = w - 2 - 2 * IS_FLATORDOWN( ob->boxtype );
		}
		else if (    sltype == FL_VERT_THIN_SLIDER
				  || sltype == FL_VERT_BASIC_SLIDER )
		{
			slb->h = size * h;
			slb->y = flinear( val, 0.0, 1.0, y, y + h - slb->h );
			slb->w = w + fudge2;
			slb->x = x - fudge1;
		}
		else
		{
			slb->y = flinear( val, 0.0, 1.0, y + absbw,
							  y + h - absbw - slb->h );
			slb->w = w - 2 * absbw;
			slb->x = x + absbw;
		}

		return;
    }

    if ( IS_HSLIDER( sltype ) )
    {
		slb->w = size * ( w - 2 * absbw );

		if ( IS_SCROLLBAR( sltype ) && slb->w < MINKNOB_SB )
			slb->w = MINKNOB_SB;
		else if ( ! IS_SCROLLBAR( sltype ) && slb->w < 2 * absbw + MINKNOB_SL )
			slb->w = 2 * absbw + MINKNOB_SL;

		if ( sltype == FL_HOR_BROWSER_SLIDER2 )
		{
			slb->w = size * w;
			slb->x = flinear( val, 0.0, 1.0, x, x + w - slb->w );
			slb->h = h - 2 * ( 1 + IS_FLATORDOWN( ob->boxtype ) );
			slb->y = y + 1 + IS_FLATORDOWN( ob->boxtype );
		}
		else if (    sltype == FL_HOR_THIN_SLIDER
				  || sltype == FL_HOR_BASIC_SLIDER )
		{
			slb->w = size * w;
			slb->x = flinear( val, 0.0, 1.0, x, x + w - slb->w );
			slb->h = h + fudge2;	/* - 2 * absbw; */
			slb->y = y - fudge1;	/* + absbw; */
		}
		else
		{
			slb->x = flinear( val, 0.0, 1.0, x + absbw,
							  x + w - absbw - slb->w );
			slb->h = h - 2 * absbw;
			slb->y = y + absbw;
		}

		return;
    }

	M_err( "fl_calc_slider_size", "Bad slider type:%d", sltype );
}


/***************************************
 ***************************************/

int
fl_slider_mouse_object( FL_OBJECT * ob,
						FL_Coord    mx,
						FL_Coord    my )
{
    FL_SCROLLBAR_KNOB slb;
	SPEC *sp = ob->spec;

    fl_calc_slider_size( ob, &slb );

    if ( IS_VSLIDER( ob->type ) )
	{
		my -= ob->y;
		if ( IS_FILL( ob->type ) )
			sp->mw = 0;
		else
			sp->mh = slb.h;

		if ( my < slb.y )
			return ABOVE_KNOB;
		else if ( my > slb.y + slb.h )
			return BELOW_KNOB;
		else
			sp->offy = slb.y + slb.h / 2 - my;

		if ( IS_FILL( ob->type ) )
			 sp->offy = 0;
	}
	else
	{
		mx -= ob->x;
		if ( IS_FILL( ob->type ) )
			sp->mw = 0;
		else
			 sp->mw = slb.w;

		if ( mx < slb.x )
			return LEFT_OF_KNOB;
		else if ( mx > slb.x + slb.w )
			return RIGHT_OF_KNOB;
		else
			sp->offx = slb.x + slb.w / 2 - mx;

		if ( IS_FILL( ob->type ) )
			 sp->offx = 0;
	}

	return ON_TOP_OF_KNOB;
}


/***************************************
 * val is normalized betweew 0 and 1
 ***************************************/

void fl_drw_slider( FL_OBJECT  * ob,
					FL_COLOR     col1,
					FL_COLOR     col2,
					const char * str,
					int          d )
{
	SPEC *sp = ob->spec;
	FL_COORD x = sp->x,
		     y = sp->y,
		     w = sp->w,
		     h = sp->h;
	int sltype = ob->type;
	int bw = ob->bw;
    FL_Coord xsl,
		     ysl,
		     hsl,
		     wsl;
    FL_Coord absbw = FL_abs( bw ),
		     bw2,
		     absbw2;
    int slbox;
    FL_SCROLLBAR_KNOB slb;

    fl_calc_slider_size( ob, &slb );

    xsl = slb.x;
    ysl = slb.y;
    wsl = slb.w;
    hsl = slb.h;

    /* Draw the slider */

    if ( d & FL_SLIDER_BOX )
    {
		int btype = ob->boxtype;
		int actual_bw = bw;

		if (    sltype == FL_VERT_BROWSER_SLIDER2
			 || sltype == FL_HOR_BROWSER_SLIDER2 )
		{
/*          btype = FL_UP_BOX; */
			actual_bw = bw > 0 ? 1 : -1;
		}
		else if (    sltype == FL_VERT_THIN_SLIDER
				  || sltype == FL_VERT_BASIC_SLIDER )
		{
			btype = FL_FLAT_BOX;
			actual_bw = bw > 0 ? 1 : -1;
		}
		else if (    sltype == FL_HOR_THIN_SLIDER
				  || sltype == FL_HOR_BASIC_SLIDER )
		{
			btype = FL_FLAT_BOX;
			actual_bw = 1;
		}
		else
			btype = btype == FL_SHADOW_BOX ? FL_BORDER_BOX : btype;

		fl_drw_box( btype, x, y, w, h, col1, actual_bw );
    }

    if ( sltype == FL_VERT_NICE_SLIDER || sltype == FL_VERT_NICE_SLIDER2 )
    {
		fl_drw_box( FL_FLAT_BOX, x + w / 2 - 2, y + absbw,
					4, h - 2 * absbw, FL_BLACK, 0 );
		fl_drw_box( FL_UP_BOX, xsl, ysl, wsl, hsl, col1,
					IS_FLATBOX( ob->boxtype ) ? -1 : bw );
		fl_drw_box( FL_DOWN_BOX, xsl + 2, ysl + hsl / 2 - 2,
					wsl - 4, 5, col2, 1 );
    }
    else if ( sltype == FL_HOR_NICE_SLIDER || sltype == FL_HOR_NICE_SLIDER2 )
    {
		int yoff = hsl > 15 ? 3 : 2;

		fl_drw_box( FL_FLAT_BOX, x + absbw, y + h / 2 - 2,
					w - 2 * absbw, 4, FL_BLACK, 0 );
		fl_drw_box( FL_UP_BOX, xsl, ysl, wsl, hsl, col1,
				    IS_FLATBOX( ob->boxtype ) ? -1 : bw );
		fl_drw_box( FL_DOWN_BOX, xsl + wsl / 2 - 2, ysl + yoff,
				    5, hsl - 2 * yoff, col2, 1 /* absbw - 2 */ );
    }
    else
    {
		switch ( ob->boxtype )
		{
			case FL_UP_BOX:
				slbox = FL_UP_BOX;
				break;

			case FL_DOWN_BOX:
				slbox = FL_UP_BOX;
				break;

			case FL_FRAME_BOX:
			case FL_EMBOSSED_BOX:
				slbox = ob->boxtype;
				break;

			case FL_ROUNDED_BOX:
				slbox = FL_ROUNDED_BOX;
				break;

			case FL_RFLAT_BOX:
				slbox = FL_ROUNDED_BOX;
				break;

			case FL_RSHADOW_BOX:
				slbox = FL_ROUNDED_BOX;
				break;

			default:
				slbox = FL_BORDER_BOX;
				break;
		}

		/* this is the height of the sliding bar */

		absbw2 = absbw >= 2 ? absbw - 1 : absbw - ( bw < 0 );
		if ( absbw2 == 0 )
			absbw2 = 1;
		bw2 = bw > 0 ? absbw2 : - absbw2;

		if (    sltype == FL_VERT_THIN_SLIDER
			 || sltype == FL_HOR_THIN_SLIDER
			 || sltype == FL_VERT_BASIC_SLIDER
			 || sltype == FL_HOR_BASIC_SLIDER )
		{
			absbw2 = absbw - ( absbw > 2 ) - ( bw == 2 );
			if ( absbw2 == 0 )
				absbw2 = 1;
			bw2 = bw > 0 ? absbw2 : - absbw2;
		}

		if ( sltype == FL_HOR_THIN_SLIDER )
			sltype = FL_HOR_BROWSER_SLIDER2;
		if ( sltype == FL_VERT_THIN_SLIDER )
			sltype = FL_VERT_BROWSER_SLIDER2;

		if ( d & FL_SLIDER_KNOB )
		{
			fl_drw_box( slbox, xsl, ysl, wsl, hsl, col2, bw2 );

			if (    sltype == FL_VERT_BROWSER_SLIDER
				 || sltype == FL_VERT_BROWSER_SLIDER2 )
			{
				/* if soft edge, we can squeeze one more pixel */

				int extra = bw2 < 0;

				fl_drw_text( FL_ALIGN_CENTER,
							 xsl - extra, ysl, wsl + 2 * extra, hsl, 0,
							 FL_NORMAL_STYLE, FL_TINY_SIZE, "@RippleLines" );
			}
			else if (    sltype == FL_HOR_BROWSER_SLIDER
					  || sltype == FL_HOR_BROWSER_SLIDER2 )
				fl_drw_text( FL_ALIGN_CENTER, xsl - 1, ysl, wsl, hsl,
							 0, 10, 1, "@2RippleLines" );
		}
    }

    if ( ! str || ! *str )
		return;

    /* Draw the label */

    fl_drw_text( FL_ALIGN_CENTER, xsl, ysl, wsl, hsl, 0, FL_NORMAL_STYLE,
				 FL_TINY_FONT, str );
}
