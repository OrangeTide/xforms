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
 *  \file pslider.h
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1995-1997  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *.
 *
 * private header for slider object
 *
 */

#ifndef PSLIDER_H
#define PSLIDER_H

#if 0

typedef const char * ( * SL_FILTER )( FL_OBJECT *, double, int );


/* The special information for sliders. */

typedef struct
{
    double    min;			/* minimal value of slider */
    double    max;			/* maximal value of slider */
    double    val;			/* current value of slider */
    int       prec;			/* precision when printing value */
    int       how_return;	/* whether always returning value */
    double    step;			/* step size             */
    double    ldelta;		/* "left mouse" step   */
    double    rdelta;		/* "right mouse " step   */
    double    slsize;		/* size of the slider    */
    double    oldval;
    double    norm_val;		/* normalized value between 0 & 1 */
    double    oldnorm_val;	/* normalized value between 0 & 1 */
    int       drawtype;
    SL_FILTER filter;
    FL_Coord  offx;
    FL_Coord  offy;
    FL_Coord  x,	        /* draw (dbl buffer): adjustment report box */
	          y,
	          w,
	          h;
    FL_Coord  mx,
	          my,
	          mw,
	          mh;	        /* mouse: after adjustment for report box   */
    int       mouse,		/* part the mouse is on                     */
	          lmouse;
} FL_SLIDER_SPEC;

#else

#include "pvaluator.h"

typedef FL_VALUATOR_SPEC FL_SLIDER_SPEC;

#endif

#define FL_MINKNOB_SB   16		/* scrollbar        */
#define FL_MINKNOB_SL   10		/* regular sliders  */

#define ON_TOP_OF_KNOB  0
#define LEFT_OF_KNOB   -1
#define ABOVE_KNOB     -1
#define RIGHT_OF_KNOB   1
#define BELOW_KNOB      1

#define IS_HSLIDER( t )      (    t == FL_HOR_SLIDER          	\
							   || t == FL_HOR_FILL_SLIDER     	\
							   || t == FL_HOR_NICE_SLIDER     	\
							   || t == FL_HOR_NICE_SLIDER2     	\
							   || t == FL_HOR_BROWSER_SLIDER  	\
							   || t == FL_HOR_BROWSER_SLIDER2 	\
							   || t == FL_HOR_THIN_SLIDER     	\
							   || t == FL_HOR_BASIC_SLIDER )

#define IS_VSLIDER( t )      ( ! IS_HSLIDER( t ) )

#define IS_FILL( t )         (    t == FL_VERT_FILL_SLIDER      \
                               || t == FL_HOR_FILL_SLIDER )

#define IS_SCROLLBAR( t )    (    t == FL_HOR_NICE_SLIDER2      \
						       || t == FL_HOR_BROWSER_SLIDER2   \
						       || t == FL_HOR_THIN_SLIDER       \
						       || t == FL_HOR_BASIC_SLIDER      \
						       || t == FL_VERT_NICE_SLIDER2     \
						       || t == FL_VERT_BROWSER_SLIDER2  \
						       || t == FL_VERT_THIN_SLIDER      \
                               || t == FL_VERT_BASIC_SLIDER )

#define IS_FLATBOX( t )      (    t == FL_FRAME_BOX        \
							   || t == FL_EMBOSSED_BOX 	  \
							   || t == FL_BORDER_BOX       \
                               || t == FL_ROUNDED_BOX )


#endif /* PSLIDER_H */
