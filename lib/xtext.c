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
 * \file xtext.c
 *
 *  This file is part of the XForms library package.
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * All text routines. There are rooms for speed ups. For one, font
 * switching can be reduced somewhat.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "flinternal.h"

#include <string.h>
#include <ctype.h>


static int UL_thickness = -1;
static int UL_propwidth = 1;    /* 1 for proportional. 0 for constant */

static void do_underline( FL_Coord,
                          FL_Coord,
                          const char *,
                          int );
static void do_underline_all( FL_Coord,
                              FL_Coord,
                              const char *,
                              int );

#define LINES           1024
#define LINES_INCREMENT  512

static char **lines = NULL;
static int *start = NULL;       /* start position of these lines  */
static int *startx = NULL;      /* start x-FL_coordinate of these lines  */
static int *starty = NULL;      /* start y-FL_coordinate of these lines  */
static int *slen = NULL;
static int nlines = LINES;
static int max_pixelline;


/***************************************
 ***************************************/

static void
extend_workmem( int nl )
{
    lines  = fl_realloc( lines,  nl * sizeof *lines );
    start  = fl_realloc( start,  nl * sizeof *start );
    startx = fl_realloc( startx, nl * sizeof *startx );
    starty = fl_realloc( starty, nl * sizeof *starty );
    slen   = fl_realloc( slen,   nl * sizeof *slen );
}


/***************************************
 ***************************************/

void
fli_free_xtext_workmem( void )
{
    fl_safe_free( lines );
    fl_safe_free( start );
    fl_safe_free( startx );
    fl_safe_free( starty );
    fl_safe_free( slen );
}


/***************************************
 ***************************************/

int
fli_get_maxpixel_line( void )
{
    return max_pixelline;
}


/* wrong, but looks nicer */

#define CDELTA  ( flx->fdesc / 3 )

typedef int ( * DrawString )( Display *,
                              Drawable, GC,
                              int,
                              int,
                              const char *,
                              int );


/***************************************
 * Major text drawing routine
 * clip == 0:  no clipping
 * clip == 1:  do clipping here
 * clip == -1: clipping is done outside of this routine
 ***************************************/

int
fli_drw_string( int           horalign,
                int           vertalign,
                FL_Coord      x,
                FL_Coord      y,
                FL_Coord      w,
                FL_Coord      h,
                int           clip,
                FL_COLOR      backcol,
                FL_COLOR      forecol,
                FL_COLOR      curscol,
                int           style,
                int           size,
                int           curspos,
                int           selstart,
                int           selend,
                const char *  istr,
                int           img,
                int           topline,
                int           endline,
                FL_COLOR      bkcol )
{
    int i;
    int width;          /* string width of the lines  */
    int lnumb;          /* number of lines  */
    FL_Coord height;
    int slstart,
        slend;      /* Temporary selection positions  */
    int xsel,
        wsel;       /* position and width of selection area  */
    /* underline stuff */
    char *p,
         newlabel[ 256 ];
    int ulpos;
    int max_pixels = 0;
    int cdelta;
    DrawString XdrawString;
    char *str = fl_strdup( istr );

    if ( flx->win == None )
    {
        fl_safe_free( str );
        return 0;
    }

    if ( ! startx )
        extend_workmem( nlines = LINES );

    /* Check whether anything has to be done  */

    if ( curspos != 0 && ( ! str || ! *str ) )
        return max_pixels;

    XdrawString = img ? XDrawImageString : XDrawString;

    fl_set_font( style, size );

    height = flx->fheight - flx->fdesc;

    /* Set clipping if required  */

    if ( clip > 0 )
        fl_set_text_clipping( x, y, w, h );

    /* Split string into lines  */

    *lines = str;
    *start = 0;
    *slen = 0;
    lnumb = 1;
    i = 0;

  redo:

    while ( str[ i ] && lnumb < nlines - 1 )
    {
        slen[ lnumb - 1 ]++;
        if ( str[ i++ ] == '\n' )
        {
            str[ i - 1 ] = '\0';
            slen[ lnumb - 1 ]--;    /* remove '\0'. from spl */
            lines[ lnumb ] = str + i;
            start[ lnumb ] = i;
            slen[ lnumb ] = 0;
            lnumb++;
        }
    }

    if ( str[ i ] )
    {
        extend_workmem( nlines += LINES_INCREMENT );
        goto redo;
    }

    start[ lnumb ] = i + 1;

    if ( ( topline -= 2 ) < 0 || topline > lnumb )
        topline = 0;

    if ( endline > lnumb || endline <= 0 )
        endline = lnumb;

    /* Using fl_fheight etc. is not theorectically correct since it is the
       max height. For lines that do not have desc, we are overestimating
       the height of the string. */

    /* Calculate start FL_coordinates of lines  */

    cdelta = CDELTA;
    for ( i = topline; i < endline; i++ )
    {
        width = XTextWidth( flx->fs, lines[ i ], slen[ i ] );
        if ( width > max_pixels )
        {
            max_pixels = width;
            max_pixelline = i;
        }

        if ( i < topline || i > endline )
            continue;

        if ( horalign == FL_ALIGN_LEFT )
            startx[ i ] = x;
        else if ( horalign == FL_ALIGN_CENTER )
            startx[ i ] = x + 0.5 * ( w - width );
        else if ( horalign == FL_ALIGN_RIGHT )
            startx[ i ] = x + w - width;

        if ( vertalign == FL_ALIGN_BOTTOM )
            starty[ i ] = y + h - 1 + ( i - lnumb ) * flx->fheight + height;
        else if ( vertalign == FL_ALIGN_CENTER )
            starty[ i ] = y + 0.5 * h + ( i - 0.5 * lnumb ) * flx->fheight +
                height + cdelta;
        else if ( vertalign == FL_ALIGN_TOP )
            starty[ i ] = y + i * flx->fheight + height;
    }

    fl_bk_textcolor( bkcol );

    for ( i = topline; i < endline; i++ )
    {
        /* If clipping, check whether any of the font is visible  */

        if ( clip != 0 && starty[ i ] - flx->fasc > y + h )
            break;

        ulpos = -1;

        /* Check if have underline request */

        if ( ( p = strchr( lines[ i ], *fl_ul_magic_char ) ) )
        {
            char *q;

            ulpos = p - lines[ i ];
            q = newlabel;
            for ( p = lines[ i ]; *p; p++ )
                if ( *p != *fl_ul_magic_char )
                    *q++ = *p;
            *q = 0;

#if FL_DEBUG >= ML_DEBUG
            M_info2( "fli_drw_string", "new = %s old = %s",
                     newlabel, lines[ i ] );
#endif

            lines[ i ] = newlabel;
            slen[ i ] = strlen( lines[ i ] );
            startx[ i ] += XTextWidth( flx->fs, fl_ul_magic_char, 1 ) / 2;
        }

        /* Draw it  */

        fl_textcolor( forecol );

        XdrawString( flx->display, flx->win, flx->textgc,
                     startx[ i ], starty[ i ], lines[ i ], slen[ i ] );

        /* Set up correct underline color in proper GC */

        if ( ulpos > 0 )
        {
            fl_color( forecol );
            do_underline( startx[ i ], starty[ i ], lines[ i ], ulpos - 1 );
        }
        else if ( ulpos == 0 )
        {
            fl_color( forecol );
            do_underline_all( startx[ i ], starty[ i ], lines[ i ], slen[ i ] );
        }

        /* Draw selection area if required  */

        if ( selstart < start[ i + 1 ] && selend > start[ i ] )
        {
            size_t len;

            if ( selstart <= start[ i ] )
                slstart = start[ i ];
            else
                slstart = selstart;

            if ( selend >= start[ i + 1 ] )
                slend = start[ i + 1 ] - 1;
            else
                slend = selend;

            xsel =   startx[ i ]
                   + XTextWidth( flx->fs, lines[ i ], slstart - start[ i ] );
            len = slend - slstart;

            wsel = XTextWidth( flx->fs, str + slstart, len );
            if ( wsel > w )
                wsel = w + 1;

            fl_rectf( xsel, starty[ i ] - height, wsel, flx->fheight, forecol );
            fl_textcolor( backcol );

            XdrawString( flx->display, flx->win, flx->textgc, xsel, starty[ i ],
                         str + slstart, len );
        }
    }

    /* Draw the cursor  */

    if ( curspos >= 0 && w - 2 > 0 && h > 0 )
    {
        int tt;

        for ( i = 0; i < lnumb && start[ i ] <= curspos; i++ )
            /* empty */;
        i--;

        tt = XTextWidth( flx->fs, lines[ i ], curspos - start[ i ] );

        if ( clip >= 0 )
            fl_set_clipping( x, y, w - 2, h );
        fl_rectf( startx[ i ] + tt, starty[ i ] - height,
                  2, flx->fheight, curscol );
        if ( clip >= 0 )
            fl_unset_clipping( );
    }

    fl_free( str );

    /* Reset clipping if required  */

    if ( clip > 0 )
        fl_unset_text_clipping( );

    return max_pixels;
}


/***************************************
 *routine returning the position of the mouse in a string
 ***************************************/

int
fli_get_pos_in_string( int          horalign,
                       int          vertalign,
                       FL_Coord     x,
                       FL_Coord     y,
                       FL_Coord     w,
                       FL_Coord     h,
                       int          style,
                       int          size,
                       FL_Coord     xpos,
                       FL_Coord     ypos,
                       const char * str,
                       int *        xp,
                       int *        yp )
{
    int i,
        i0,
        len;
    int lnumb;          /* number of lines  */
    int theline;        /* number of line in which the mouse lies  */
    int width;          /* string width of this line  */
    const char *line;   /* line in which mouse lies  */
    int xstart;         /* start x-FL_coordinate of this line  */
    double toppos;      /* y-FL_coord of the top line  */

    /* Check whether anything has to be done  */

    if ( ! str || ! *str )
        return 0;

    fl_set_font( style, size );

    /* Split string into lines  */

    start[ 0 ] = 0;
    for ( lnumb = 1, i = 0; str[ i ]; i++ )
        if ( str[ i ] == '\n' )
            start[ lnumb++ ] = i + 1;
    start[ lnumb ] = i + 1;

    /* Calculate line in which mouse lies  */

    if ( vertalign == FL_ALIGN_BOTTOM )
        toppos = y + h - 1;
    else if ( vertalign == FL_ALIGN_CENTER )
        toppos = y + 0.5 * h - 0.5 * lnumb * flx->fheight + CDELTA;
    else if ( vertalign == FL_ALIGN_TOP )
        toppos = y;
    else
        toppos = y + 0.5 * h - 0.5 * lnumb * flx->fheight;

    theline = ( ypos - toppos ) / flx->fheight + 0.01;

    if ( theline < 0 )
    {
        *yp = 1;
        theline = 0;
    }

    if ( theline >= lnumb )
    {
        theline = lnumb - 1;
        *yp = lnumb;
    }

    line = str + start[ theline ];

    *yp = theline + 1;

    /* Calculate start FL_coordinate of the line  */

    width = XTextWidth( flx->fs, ( char * ) line,
                        start[ theline + 1 ] - start[ theline ] );

    if ( horalign == FL_ALIGN_LEFT )
        xstart = x;
    else if ( horalign == FL_ALIGN_CENTER )
        xstart = x + 0.5 * ( w - width );
    else if ( horalign == FL_ALIGN_RIGHT )
        xstart = x + w - width;
    else
        xstart = x;

    /* total pixels away from the begining of line */

    xpos = xpos + 2 - xstart;

    /* take a guess for the char offset. Assuming char H > W */

    i0 = xpos / flx->fheight + 1;

    len = start[ theline + 1 ] - start[ theline ];

    for ( i = i0; i < len; i++ )
    {
        if ( XTextWidth( flx->fs, line, i ) > xpos )
        {
            *xp = i - 1;
            return start[ theline ] + i - 1;
        }
    }

    *xp = len;
    return start[ theline + 1 ] - 1;
}

/***
  Misselaneous text drawing routines
***/


/***************************************
 * Draws a (multi-line) text with a cursor
 ***************************************/

void
fl_drw_text_cursor( int          align,
                    FL_Coord     x,
                    FL_Coord     y,
                    FL_Coord     w,
                    FL_Coord     h,
                    FL_COLOR     c,
                    int          style,
                    int          size,
                    const char * str,
                    int          cc,
                    int          pos )
{
    int horalign,
        vertalign;

    flx->fheight = fl_get_char_height( style, size, &flx->fasc, &flx->fdesc );
    fli_get_hv_align( align, &horalign, &vertalign );
    fli_drw_string( horalign, vertalign, x, y, w, h, 0, FL_WHITE, c, cc,
                    style, size, pos, 0, -1, str, 0, 0, 0, 0 );
}


/***************************************
 ***************************************/

static void
fli_draw_text_cursor( int          align,
                      FL_Coord     x,
                      FL_Coord     y,
                      FL_Coord     w,
                      FL_Coord     h,
                      const char * str,
                      int          style,
                      int          size,
                      FL_COLOR     c,
                      FL_COLOR     bc,
                      FL_COLOR     cc,
                      int          bk,
                      int          pos )
{
    int horalign,
        vertalign;

    flx->fheight = fl_get_char_height( style, size, &flx->fasc, &flx->fdesc );
    fli_get_hv_align( align, &horalign, &vertalign );
    fli_drw_string( horalign, vertalign, x, y, w, h, 0, FL_WHITE, c, cc,
                    style, size, pos, 0, -1, str, bk, 0, 0, bc );
}


#define D( x, y, c )                                      \
    fli_draw_text_cursor( align, x, y, w, h, str,         \
                          style,size, c, bc, 0, bk, -1 )


/***************************************
 ***************************************/

void
fli_draw_text_inside( int          align,
                      FL_Coord     x,
                      FL_Coord     y,
                      FL_Coord     w,
                      FL_Coord     h,
                      const char * istr,
                      int          style,
                      int          size,
                      FL_COLOR     c,
                      FL_COLOR     bc,
                      int          bk )
{
    int special = 0;
    int xoff,
        yoff;
    char *str;
    int sw = w,
        sh = h,
        sx = x,
        sy = y;

    if ( ! istr || ! *istr )
        return;

    str = fl_strdup( istr );

    if ( str[ 0 ] == '@' && str[ 1 ] != '@' )
    {
        if ( w < 5 && h < 5 )
        {
            sw = sh = 6 + 1.1 * size;
            sx -= sw / 2;
            sy -= sh / 2;
        }

        if ( fl_draw_symbol( str, sx, sy, sw, sh, c ) )
        {
            fl_free( str );
            return;
        }
        else
            str[ 0 ] = ' ';
    }

    if ( str[ 0 ] == '@' )
        str++;

    xoff = 5;
    yoff = 4;

    x += xoff;
    w -= 2 * xoff;
    y += yoff;
    h -= 2 * yoff;

    if ( special_style( style ) )
    {
        special = ( style / FL_SHADOW_STYLE ) * FL_SHADOW_STYLE;
        style %= FL_SHADOW_STYLE;
    }

    /* Take care of special effects stuff  */

    if ( special == FL_SHADOW_STYLE )
        D( x + 2, y + 2, FL_BOTTOM_BCOL );
    else if ( special == FL_ENGRAVED_STYLE )
    {
        D( x - 1, y,     FL_RIGHT_BCOL );
        D( x,     y - 1, FL_RIGHT_BCOL );
        D( x - 1, y - 1, FL_RIGHT_BCOL );
        D( x + 1, y,     FL_TOP_BCOL );
        D( x,     y + 1, FL_TOP_BCOL );
        D( x + 1, y + 1, FL_TOP_BCOL );
    }
    else if ( special == FL_EMBOSSED_STYLE )
    {
        D( x - 1, y,     FL_TOP_BCOL );
        D( x,     y - 1, FL_TOP_BCOL );
        D( x - 1, y - 1, FL_TOP_BCOL );
        D( x + 1, y,     FL_RIGHT_BCOL );
        D( x,     y + 1, FL_RIGHT_BCOL );
        D( x + 1, y + 1, FL_RIGHT_BCOL );
    }

    fli_draw_text_cursor( align, x, y, w, h, str, style, size,
                          c, bc, 0, special ? 0 : bk, -1 );

    fl_free( str );
}


/***************************************
 * Draws a text inside a box.
 ***************************************/

void
fl_drw_text( int            align,
             FL_Coord       x,
             FL_Coord       y,
             FL_Coord       w,
             FL_Coord       h,
             FL_COLOR       c,
             int            style,
             int            size,
             const char *   istr )
{
    fli_draw_text_inside( align, x, y, w, h, istr, style, size, c, 0, 0 );
}


/***************************************
 ***************************************/

#if 0

void
fl_draw_text_beside( int      align,
                     FL_Coord x,
                     FL_Coord y,
                     FL_Coord w,
                     FL_Coord h,
                     char *   str,
                     int      len,
                     int      style,
                     int      size,
                     FL_COLOR c,
                     FL_COLOR bc,
                     int      bk )
{
    int newa,
        newx,
        newy,
        dx = 0,
        dy = 0;

    if ( ! str || ! *str )
        return;

    if ( align & FL_ALIGN_INSIDE )
        M_warn( "drw_text_beside", "align request is inside" );

    if ( align & FL_ALIGN_LEFT )
    {
        if ( align & FL_ALIGN_BOTTOM || align & FL_ALIGN_TOP )
            dx = - 4;
        else
            dx = 1;
    }
    else if ( align & FL_ALIGN_RIGHT )
    {
        if ( align & FL_ALIGN_BOTTOM || align & FL_ALIGN_TOP )
            dx = 4;
        else
            dx = - 1;
    }

    if ( align & FL_ALIGN_BOTTOM )
        dy = - 2;
    else if ( align & FL_ALIGN_TOP )
        dy = 2;

    x += dx;
    y += dy;

    fli_get_outside_align( align, x, y, w, h, &newa, &newx, &newy );
    fli_draw_text_inside( align, x, y, w, h, str, style, size, c, bc, bk );
}
#endif


/***************************************
 ***************************************/

void
fl_drw_text_beside( int          align,
                    FL_Coord     x,
                    FL_Coord     y,
                    FL_Coord     w,
                    FL_Coord     h,
                    FL_COLOR     c,
                    int          style,
                    int          size,
                    const char * str )
{
    int newa,
        newx,
        newy,
        dx = 0,
        dy = 0;

    if ( ! str || ! *str || w <= 0 || h <= 0 )
        return;

    if ( align & FL_ALIGN_INSIDE )
        M_warn( "drw_text_beside", "align request is inside" );

    if ( align & FL_ALIGN_LEFT )
    {
        if ( align & FL_ALIGN_BOTTOM || align & FL_ALIGN_TOP )
            dx = - 4;
        else
            dx = 1;
    }
    else if ( align & FL_ALIGN_RIGHT )
    {
        if ( align & FL_ALIGN_BOTTOM || align & FL_ALIGN_TOP )
            dx = 4;
        else
            dx = - 1;
    }

    if ( align & FL_ALIGN_BOTTOM )
        dy = - 2;
    else if ( align & FL_ALIGN_TOP )
        dy = 2;

    x += dx;
    y += dy;

    fli_get_outside_align( align, x, y, w, h, &newa, &newx, &newy );
    fl_drw_text( newa, newx, newy, w, h, c, style, size, str );
}


/***************************************
 * Do underlined text, single character only
 * if underline width to be proportional or fixed width
 ***************************************/

void
fl_set_ul_property( int prop,
                    int thickness )
{
    UL_propwidth = prop;
    if ( thickness > 0 )
        UL_thickness = thickness;
}


#define DESC( c )   ( c == 'g' || c == 'j' || c == 'q' || c == 'y' || c == 'p' )
#define NARROW( c ) ( c == 'i' || c == 'j' || c == 'l' || c == 'f' || c == '1' )

/***************************************
 ***************************************/

XRectangle *
fli_get_underline_rect( XFontStruct * fs,
                        FL_Coord      x,
                        FL_Coord      y,
                        const char *  cstr,
                        int           n )
{
    static XRectangle xr;
    int ul_width,
        ul_rwidth,
        xoff;
    unsigned long ul_pos,
                  ul_thickness = 0;
    char *str = ( char * ) cstr;
    int ch = *( str + n );
    int pre;                /* stuff in front of the string, such as ^H */

    if ( UL_thickness < 0 )
        XGetFontProperty( flx->fs, XA_UNDERLINE_THICKNESS, &ul_thickness );
    else
        ul_thickness = UL_thickness;

    if ( ul_thickness == 0 || ul_thickness > 100 )
        ul_thickness = strstr( fli_curfnt, "bold" ) ? 2 : 1;

    if ( ! XGetFontProperty( fs, XA_UNDERLINE_POSITION, &ul_pos ) )
        ul_pos = DESC( ch ) ? ( 1 + flx->fdesc ) : 1;

    /* if the character is narrow, use the width of g otherwise use the width
       of D. Of course, if UL_width == proportional, this really does not
       matter */

    ul_width = XTextWidth( fs, NARROW( ch ) ? "h" : "D", 1 );
    ul_rwidth = XTextWidth( fs, str + n, 1 );

    pre = str[ 0 ] == *fl_ul_magic_char;

    xoff = fli_get_string_widthTABfs( fs, str + pre, n - pre );

    /* try to center the underline on the correct character */

    if ( UL_propwidth )
        x = x + xoff;
    else
        x = x + xoff + ( ul_rwidth - ul_width ) / 2;

    xr.x = x;
    xr.y = y + ul_pos;
    xr.width = UL_propwidth ? ul_rwidth : ul_width;
    xr.height = ul_thickness;
    return &xr;
}


/***************************************
 ***************************************/

static void
do_underline( FL_Coord     x,
              FL_Coord     y,
              const char * cstr,
              int          n )
{
    XRectangle *xr = fli_get_underline_rect( flx->fs, x, y, cstr, n );

    if ( flx->win == None || xr->width <= 0 ||  xr->height <= 0 )
        return;

    XFillRectangle( flx->display, flx->win, flx->gc, xr->x, xr->y,
                    xr->width, xr->height );
}


#define has_desc( s )    (    strchr( s, 'g' )  \
                           || strchr( s, 'j' )  \
                           || strchr( s, 'q' )  \
                           || strchr( s, 'y' )  \
                           || strchr( s, 'p' ) )


/***************************************
 * Underline whole string
 ***************************************/

static void
do_underline_all( FL_Coord     x,
                  FL_Coord     y,
                  const char * cstr,
                  int          n )
{
    int ul_width;
    unsigned long ul_pos,
                  ul_thickness = 0;
    char *str = ( char * ) cstr;

    if ( flx->win == None )
        return;

    if ( UL_thickness < 0 )
        XGetFontProperty( flx->fs, XA_UNDERLINE_THICKNESS, &ul_thickness );
    else
        ul_thickness = UL_thickness;

    if ( ul_thickness == 0 || ul_thickness > 100 )
        ul_thickness = strstr( fli_curfnt, "bold" ) ? 2 : 1;

    if ( ! XGetFontProperty( flx->fs, XA_UNDERLINE_POSITION, &ul_pos ) )
        ul_pos = has_desc( str ) ? ( 1 + flx->fdesc ) : 1;

    ul_width = XTextWidth( flx->fs, str, n );

    /* do it */

    if ( ul_width > 0 && ul_thickness > 0 )
        XFillRectangle( flx->display, flx->win, flx->gc, x, y + ul_pos,
                        ul_width, ul_thickness );
}


/***************************************
 * Draw a single string possibly with embedded tabs
 ***************************************/

int
fli_drw_stringTAB( Window       win,
                   GC           gc,
                   int          x,
                   int          y,
                   int          style,
                   int          size,
                   const char * s,
                   int          len,
                   int          img )
{
    int w, tab;
    const char *p,
               *q;
    XFontStruct *fs = fl_get_font_struct( style, size );
    DrawString XdrawString;

    if ( win == 0 )
        return 0;

    tab = fli_get_tabpixels( fs );
    XdrawString = img ? XDrawImageString : XDrawString;

    XSetFont( flx->display, gc, fs->fid );

    for ( w = 0, q = s; *q && ( p = strchr( q, '\t' ) ) && p - s < len;
          q = p + 1 )
    {
        XdrawString( flx->display, win, gc, x + w, y, ( char * ) q, p - q );
        w += XTextWidth( fs, q, p - q );
        w = ( w / tab + 1 ) * tab;
    }

    XdrawString( flx->display, win, gc, x + w, y, ( char * ) q, s - q + len );

    return 0;           /* w + XTextWidth(fs, q, len - (q - s)); */
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
