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
 * \file spline.c
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1998-2002  T.C. Zhao
 *  All rights reserved.
 *
 * interpolate_spline interpolates one-dimensional, non-uniform
 * tabulated data onto a working grid, using cubic splines.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "include/forms.h"
#include "flinternal.h"


/*
  NOTE: In the two functions memory gets allocated that is never
        deallocated, not even on fl_finish().
*/


/***************************************
 * The input x-coordinate should be monotonically increasing
 ***************************************/

int
fl_spline_interpolate( const float * wx,
                       const float * wy,
                       int           nin,
                       float *       x,
                       float *       y,
                       double        grid )
{
    int i,
        j,
        k,
        jo,
        ih,
        im,
        nout;
    double sig,
           p,
           h,
           a,
           b;
    static int nwork = 0;
    static double *y2 = NULL,
                  *u = NULL;

    if ( nin <= 3 )
    {
        M_warn( "fl_spline_interpolate",
                "too few points (less than 4) for interpolation" );
        return -1;
    }

    if ( nwork < nin )
    {
        y2 = fl_realloc( y2, nin * sizeof *y2 );
        u = fl_realloc( u, nin * sizeof *u );
        nwork = nin;
    }

    /* Compute the second derivative */

    y2[ 0 ] = u[ 0 ] = 0.0;

    for ( i = 1; i < nin - 1; i++ )
    {
        sig = ( double ) ( wx[ i ] - wx[ i - 1 ] ) /
              ( wx[ i + 1 ] - wx[ i - 1 ] );
        p = sig * y2[ i - 1 ] + 2.0;
        y2[ i ] = ( sig - 1.0 ) / p;
        u[ i ] =   ( double ) ( wy[ i + 1 ] - wy[ i ] ) /
                              ( wx[ i + 1 ] - wx[ i ] )
                 - ( double ) ( wy[ i ] - wy[ i - 1 ] ) /
                              ( wx[ i ] - wx[ i - 1 ] );
        u[ i ] = (   6.0 * u[i] / ( wx[ i + 1 ] - wx[ i - 1 ] )
                   - sig * u[ i - 1 ] ) / p;
    }

    y2[ nin - 1 ] = 0.0;
    for ( k = nin - 2; k >= 0; k-- )
        y2[ k ] = y2[ k ] * y2[ k + 1 ] + u[ k ];

    /* Outputs */

    nout = ( int ) ( ( wx[ nin - 1 ] - wx[ 0 ] ) / grid + 1.01);

    x[ 0 ] = wx[ 0 ];
    y[ 0 ] = wy[ 0 ];

    /* Start the main loop */

    for ( jo = 0, i = 1; i < nout; i++ )
    {
        /* Better than x[i] = x[i-1] + grid; */

        x[ i ] = x[ 0 ] + i * grid;

        /* Center */

        j = jo;
        ih = nin;
        while ( ih - j > 1 )
        {
            im = ( ih + j ) / 2;
            if ( x[ i ] > wx[ im ] )
                j = im;
            else
                ih = im;
        }

        jo = j;

        /* Interpolate */

        h = wx[ ih ] - wx[ j ];
        a = ( wx[ ih ] - x[ i ] ) / h;
        b = ( x[ i ] - wx[ j ] ) / h;

        y[i] =   a * wy[ j ] + b * wy[ ih ]
               + ( ( a * a * a - a ) * y2[ j ]
                   + ( b * b * b - b ) * y2[ ih ] ) * ( h * h ) / 6.0;
    }

    x[ nout - 1 ] = wx[ nin - 1 ];
    y[ nout - 1 ] = wy[ nin - 1 ];

    return nout;
}


/***************************************
 * Specialized for image processing
 ***************************************/

int
fl_spline_int_interpolate( const int * wx,
                           const int * wy,
                           int         nin,
                           int         grid,
                           int *       y )
{
    int i,
        j,
        k,
        jo,
        ih,
        im,
        nout;
    double sig,
           p,
           h,
           a,
           b,
           x;
    static int nwork = 0;
    static double *y2 = NULL,
                  *u = NULL;

    if ( nin <= 3 )
    {
        M_warn( "fl_spline_int_interpolate",
                "too few points (less than 4) for interpolation" );
        return -1;
    }

    if ( nwork < nin )
    {
        y2 = fl_realloc( y2, nin * sizeof *y2 );
        u = fl_realloc( u, nin * sizeof *u );
        nwork = nin;
    }

    y2[ 0 ] = u[ 0 ] = 0.0;

    for ( i = 1; i < nin - 1; i++ )
    {
        sig = ( double ) ( wx[ i ] - wx[ i - 1 ] ) /
              ( wx[ i + 1 ] - wx[ i - 1 ] );
        p = sig * y2[ i - 1 ] + 2.0;
        y2[ i ] = ( sig - 1.0 ) / p;
        u[ i ] =   ( double ) ( wy[ i + 1 ] - wy[ i ] ) /
                              ( wx[ i + 1 ] - wx[ i ] )
                 - ( double ) ( wy[ i ] - wy[ i - 1 ] ) /
                              ( wx[ i ] - wx[ i - 1 ] );
        u[ i ] = (   6.0 * u[ i ] / ( wx[ i + 1 ] - wx[ i - 1 ] )
                   - sig * u[ i - 1 ] ) / p;
    }

    y2[ nin - 1 ] = 0.0;
    for ( k = nin - 2; k >= 0; k-- )
        y2[ k ] = y2[ k ] * y2[ k + 1 ] + u[ k ];

    /* Outputs */

    nout = ( int ) ( ( wx[ nin - 1 ] - wx[ 0 ] ) / grid + 1.01 );

    y[ 0 ] = wy[ 0 ];

    /* Start the main loop */

    for ( jo = 0, i = 1; i < nout; i++ )
    {
        x = wx[ 0 ] + i * grid;

        /* Center */

        j = jo;
        ih = nin - 1;
        while ( ih - j > 1 )
        {
            im = ( ih + j ) / 2;
            if ( x > wx[ im ] )
                j = im;
            else
                ih = im;
        }

        jo = j;

        /* Interpolate */

        h = wx[ ih ] - wx[ j ];
        a = ( wx[ ih ] - x ) / h;
        b = ( x - wx[ j ] ) / h;

        y[i] = ( a * wy[ j ] + b * wy[ ih ]
                 + ( ( a * a * a - a ) * y2[ j ]
                     + ( b * b * b - b ) * y2[ ih ] )
                 * ( h * h ) / 6.0 ) + 0.1;

        /* Clamp */

        if ( y[ i ] < 0 )
            y[ i ] = 0;
        else if ( y[ i ] > FL_PCMAX )
            y[ i ] = FL_PCMAX;
    }

    y[ nout - 1 ] = wy[ nin - 1 ];
    return nout;
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
