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
 * \file sldraw.c
 *.
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"
#include "private/pslider.h"


#define IS_FLATORDOWN( t )  ( IS_FLATBOX( t ) || t == FL_DOWN_BOX )

#define IS_FLATORUPBOX( t ) ( IS_FLATBOX( t ) || t == FL_UP_BOX )


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


/***************************************
 ***************************************/

void
fli_calc_slider_size( FL_OBJECT          * ob,
					  FLI_SCROLLBAR_KNOB * slb )
{
	FLI_SLIDER_SPEC *sp = ob->spec;
	int sltype = ob->type;
	double val  = sp->min == sp->max ?
		          0.5 : ( sp->val - sp->min ) / ( sp->max - sp->min );
	double size = sp->slsize;
	FL_COORD bw = ob->bw;
    int absbw = FL_abs( bw );
    int fudge1 = IS_FLATORUPBOX( ob->boxtype ) ? 0 : ( bw == -2 || bw >= 2 );
    int fudge2 = IS_FLATORUPBOX( ob->boxtype ) ?
		         0 : ( 2 * ( bw == -2 ) + ( bw >= 2 ) );

    if ( sltype == FL_VERT_FILL_SLIDER )
    {
		int inv = sp->min > sp->max;

		slb->h = ( inv ? 1 - val : val ) * ( sp->h - 2 * absbw );
		slb->y = inv ? sp->h - absbw - slb->h : absbw;
		slb->w = sp->w - 2 * absbw;
		slb->x = absbw;
		return;
    }
    else if ( sltype == FL_HOR_FILL_SLIDER )
    {
		slb->w = val * ( sp->w - 2 * absbw );
		slb->x = absbw;
		slb->h = sp->h - 2 * absbw;
		slb->y = absbw;
		return;
    }

	if ( IS_VSLIDER( sltype ) )
    {
		slb->h = size * ( sp->h - 2 * absbw );
	
		if ( IS_SCROLLBAR( sltype ) && slb->h < MINKNOB_SB )
			slb->h = MINKNOB_SB;
		else if (    ! IS_SCROLLBAR( sltype )
				  && slb->h < 2 * absbw + MINKNOB_SL )
			slb->h = 2 * absbw + MINKNOB_SL;

		if ( sltype == FL_VERT_BROWSER_SLIDER2 )
		{
			slb->h = size * sp->h;
			slb->y = flinear( val, 0.0, 1.0, 0, sp->h - slb->h );
			slb->x = 1 + IS_FLATORDOWN( ob->boxtype );
			slb->w = sp->w - 2 - 2 * IS_FLATORDOWN( ob->boxtype );
		}
		else if (    sltype == FL_VERT_THIN_SLIDER
				  || sltype == FL_VERT_BASIC_SLIDER )
		{
			slb->h = size * sp->h;
			slb->y = flinear( val, 0.0, 1.0, 0, sp->h - slb->h );
			slb->w = sp->w + fudge2;
			slb->x = - fudge1;
		}
		else
		{
			slb->y = flinear( val, 0.0, 1.0, absbw, sp->h - absbw - slb->h );
			slb->w = sp->w - 2 * absbw;
			slb->x = absbw;
		}

		return;
    }
	else
    {
		slb->w = size * ( sp->w - 2 * absbw );

		if ( IS_SCROLLBAR( sltype ) && slb->w < MINKNOB_SB )
			slb->w = MINKNOB_SB;
		else if (    ! IS_SCROLLBAR( sltype )
				  && slb->w < 2 * absbw + MINKNOB_SL )
			slb->w = 2 * absbw + MINKNOB_SL;

		if ( sltype == FL_HOR_BROWSER_SLIDER2 )
		{
			slb->w = size * sp->w;
			slb->x = flinear( val, 0.0, 1.0, 0, sp->w - slb->w );
			slb->h = sp->h - 2 * ( 1 + IS_FLATORDOWN( ob->boxtype ) );
			slb->y = 1 + IS_FLATORDOWN( ob->boxtype );
		}
		else if (    sltype == FL_HOR_THIN_SLIDER
				  || sltype == FL_HOR_BASIC_SLIDER )
		{
			slb->w = size * sp->w;
			slb->x = flinear( val, 0.0, 1.0, 0, sp->w - slb->w );
			slb->h = sp->h + fudge2;
			slb->y = - fudge1;
		}
		else
		{
			slb->x = flinear( val, 0.0, 1.0, absbw, sp->w - absbw - slb->w );
			slb->h = sp->h - 2 * absbw;
			slb->y = absbw;
		}

		return;
    }

	M_err( "fli_calc_slider_size", "Bad slider type:%d", sltype );
}


/***************************************
 * val is normalized betweew 0 and 1
 ***************************************/

void fli_drw_slider( FL_OBJECT  * ob,
					 FL_COLOR     col1,
					 FL_COLOR     col2,
					 const char * str,
					 int          d )
{
	FLI_SLIDER_SPEC *sp = ob->spec;
	FL_COORD x = ob->x + sp->x,
		     y = ob->y + sp->y,
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
    FLI_SCROLLBAR_KNOB slb;

    fli_calc_slider_size( ob, &slb );

    xsl = ob->x + sp->x + slb.x;
    ysl = ob->y + sp->y + slb.y;
    wsl = slb.w;
    hsl = slb.h;

    /* Draw the slider */

    if ( d & FLI_SLIDER_BOX )
    {
		int btype = ob->boxtype;
		int actual_bw = bw;

		if (    sltype == FL_VERT_BROWSER_SLIDER2
			 || sltype == FL_HOR_BROWSER_SLIDER2 )
		{
			btype = FL_UP_BOX;
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

		if ( d & FLI_SLIDER_KNOB )
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
