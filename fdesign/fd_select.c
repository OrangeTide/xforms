/*
 * This file is part of XForms.
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
 * \file fd_select.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * Part of the Form Designer.
 *
 * This file contains all routines and data types to maintain the current
 * selection and manipulate (move, scale) and draw it.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include <ctype.h>

#define MAXSEL  512

#define BackOBJ( )    cur_form->first->next

static FL_OBJECT *selobj[ MAXSEL ]; /* The selected objects */
static int selnumb = 0;     /* Their number */
int backf = FL_FALSE;       /* Whether the selection is the backface */

static FL_OBJECT *tmpobj[ MAXSEL ]; /* A temporary list of objects */
static int tmpnumb = 0;     /* Their number */


/***************************************
 * Returns the index of a particular object. -1 if it is not selected
 ***************************************/

static int
find_selobject( FL_OBJECT * obj )
{
    int i;

    for ( i = 0; i < selnumb; i++ )
        if ( selobj[ i ] == obj )
            return i;

    return -1;
}


/***************************************
 * Cleans up the selection, ordering the objects and creating
 * groups if all elements are in there
 ***************************************/

static void
cleanup_selection( void )
{
    FL_OBJECT *obj,
              *begobj = NULL;
    int i,
        tt,
        sel = -1;

    if ( cur_form == NULL )
    {
        selnumb = 0;
        backf = FL_FALSE;
        return;
    }

    /* Figure out whether whole groups are selected */

    obj = cur_form->first;
    while ( obj != NULL )
    {
        if ( obj->objclass == FL_BEGIN_GROUP )
        {
            sel = 1;
            begobj = obj;
        }
        else if ( obj->objclass == FL_END_GROUP )
        {
            if ( sel )
            {
                selobj[ selnumb++ ] = begobj;
                selobj[ selnumb++ ] = obj;
            }
            else
            {
                if ( ( tt = find_selobject( begobj ) ) != -1 )
                    selobj[ tt ] = NULL;
                if ( ( tt = find_selobject( obj ) ) != -1 )
                    selobj[ tt ] = NULL;
            }
        }
        else
        {
            if ( find_selobject( obj ) == -1 )
                sel = 0;
        }

        obj = obj->next;
    }

    /* Make a new, ordered list of selected items, removing duplicates and
       NULL objects */

    tmpnumb = 0;
    for ( obj = cur_form->first; obj != NULL; obj = obj->next )
        if ( find_selobject( obj ) != -1 )
            tmpobj[ tmpnumb++ ] = obj;

    for ( i = 0; i < tmpnumb; i++ )
        selobj[ i ] = tmpobj[ i ];

    selnumb = tmpnumb;
    fillin_groups( );
}


/***************************************
 * Returns whether object is selected
 ***************************************/

int
is_selected( FL_OBJECT * obj )
{
    return find_selobject( obj ) != -1;
}


/***************************************
 * Adds an object to the current selection
 ***************************************/

void
addto_selection( FL_OBJECT * obj )
{
    if ( backf )
    {
        M_warn( "", "ignoring object with backface" );
        return;         /* Don't add objects with backface */
    }

    if ( selnumb == MAXSEL )
    {
        fprintf( stderr, "Exceeding selection limits\n" );
        return;
    }

    /* find the real parent */

    while ( obj->parent )
        obj = obj->parent;

    selobj[ selnumb++ ] = obj;
    cleanup_selection( );
}


/***************************************
 * Adds a group to the current selection
 ***************************************/

void
addgroupto_selection( FL_OBJECT * obj )
{
    FL_OBJECT *ob;

    if ( backf )
        return;         /* Don't add objects with backface */

    if ( obj->objclass != FL_BEGIN_GROUP )
        return;

    for ( ob = obj; ob && ob->objclass != FL_END_GROUP; ob = ob->next )
    {
        if ( selnumb == MAXSEL )
            return;
        selobj[ selnumb++ ] = ob;
    }

    if ( ob != NULL )
        selobj[ selnumb++ ] = ob;

    cleanup_selection( );
}


/***************************************
 * Removes an object from the selection (if present)
 ***************************************/

void
deletefrom_selection( FL_OBJECT * obj )
{
    int ind = find_selobject( obj );

    if ( ind != -1 )
        selobj[ ind ] = NULL;
    cleanup_selection( );
}


/***************************************
 * Removes a group to the current selection
 ***************************************/

void
deletegroupfrom_selection( FL_OBJECT * obj )
{
    FL_OBJECT *ob;
    int ind;

    if ( backf )
        return;         /* Don't remove objects with backface */

    if ( obj->objclass != FL_BEGIN_GROUP )
        return;

    for ( ob = obj; ob != NULL && ob->objclass != FL_END_GROUP; ob = ob->next )
        if ( ( ind = find_selobject( ob ) ) != -1 )
            selobj[ ind ] = NULL;

    cleanup_selection( );
}


/***************************************
 * Clears the complete selection
 ***************************************/

void
clear_selection( void )
{
    selnumb = 0;
    backf = FL_FALSE;
    cleanup_selection( );
}


/****
  Helper procedures
****/

/***************************************
 * Compute the bounding box of the selection
 ***************************************/

static void
compute_selbox( float * x,
                float * y,
                float * w,
                float * h )
{
    int i;
    float x1 =   1.0e+37,
          y1 =   1.0e+37,
          x2 = - 1.0e+37,
          y2 = - 1.0e+37;

    for ( i = 0; i < selnumb; i++ )
        if (    selobj[ i ]->objclass != FL_BEGIN_GROUP
             && selobj[ i ]->objclass != FL_END_GROUP )
        {
            if ( selobj[ i ]->fl1 < x1 )
                x1 = selobj[ i ]->fl1;
            if ( selobj[ i ]->ft1 < y1 )
                y1 = selobj[ i ]->ft1;
            if ( selobj[ i ]->fl2 > x2 )
                x2 = selobj[ i ]->fl2;
            if ( selobj[ i ]->ft2 > y2 )
                y2 = selobj[ i ]->ft2;
        }

    *x = x1;
    *y = y1;
    *w = x2 - x1;
    *h = y2 - y1;
}


/***************************************
 * Find position of the mouse mouse
 ***************************************/

static void
find_mousepos( float * mx,
               float * my )
{
    if ( cur_form == NULL )
        return;

    fl_winset( main_window );
    get_mouse_pos( mx, my );
}


/***************************************
 * Returns the object under the mouse.
 ***************************************/

static FL_OBJECT *
find_mouseobj( void )
{
    float xx,
          yy;

    if ( cur_form == NULL )
        return NULL;

    find_mousepos( &xx, &yy );

    return fli_find_last( cur_form, FLI_FIND_MOUSE, xx, yy );
}


/****
  Drawing routines
****/

#define HS  8

int hidden = FL_FALSE;


/***************************************
 * Draw the selection box
 ***************************************/

void
draw_selbox( void )
{
    float x,
          y,
          w,
          h;
    int i;
    FL_OBJECT *ob;

    if ( selnumb == 0 )
        return;

    /* Draw object boxes */

    color( fd_red );
    for ( i = 0; i < selnumb; i++ )
    {
        ob = selobj[ i ];
        if ( ob->objclass != FL_BEGIN_GROUP && ob->objclass != FL_END_GROUP )
            rect( ob->x, ob->y, ob->x + ob->w - 1.0, ob->y + ob->h - 1.0 );
    }

    if ( hidden )
        return;

    /* Draw the total box */

    compute_selbox( &x, &y, &w, &h );

    show_geometry( x, y, w, h );

    if ( ! backf )
    {
        x -= 1.0;
        y -= 1.0;
        w += 2.0;
        h += 2.0;
    }
    color( fd_red );

    rect(  x,          y,          x + w  - 1.0, y + h  - 1.0 );
    rectf( x,          y,          x + HS - 1.0, y + HS - 1.0 );
    rectf( x + w - HS, y,          x + w  - 1.0, y + HS - 1.0 );
    rectf( x + w - HS, y + h - HS, x + w  - 1.0, y + h  - 1.0 );
    rectf( x,          y + h - HS, x + HS - 1.0, y + h  - 1.0 );

    show_selmessage( selobj, selnumb );
}


/****
  Interaction handling
****/

/***************************************
 * Handles the moving of the selection (by mouse)
 ***************************************/

int
within_selection( float mx,
                  float my )
{
    float x,
          y,
          w = 0.0,
          h = 0.0;

    if ( ! selnumb || ! cur_form || ! cur_form->first )
        return 0;

    compute_selbox( &x, &y, &w, &h );

    /* If backface, only within scale knob is considered within */

    if ( selobj[ selnumb - 1 ] == BackOBJ( ) )
        return    mx >= x + w - HS
               && mx <  x + w
               && my >= y + h - HS
               && my <  y + h;

    return mx > x && mx < x + w && my > y && my < y + h;
}


/***************************************
 ***************************************/

void
handle_move( const XEvent * xev )
{
    float x,
          y,
          w,
          h;
    float mx,
          my;
    float ox,
          oy,
          ow,
          oh;
    FL_Coord xx,
             yy;
    int i,
        s;

    if ( cur_form == NULL )
        return;

    fl_winset( main_window );

    s = ShiftIsDown( xev->xbutton.state );

    find_mousepos( &mx, &my );
    compute_selbox( &x, &y, &w, &h );

    if ( mx < x || mx > x + w || my < y || my > y + h )
        return;         /* not in box */

    hidden = FL_TRUE;
    redraw_the_form( 0 );

    if ( backf )
    {
        if ( mx >= x + w - HS && my >= y + h - HS )
        {
            fl_get_winsize( main_window, &xx, &yy );
            set_bounding_box( 0.0, 0.0, xx, yy );
            scale_box( &x, &y, &w, &h );
            cur_form->w_hr = cur_form->w = w;
            cur_form->h_hr = cur_form->h = h;
            selobj[ 0 ]->fl1 = selobj[ 0 ]->w = w;
            selobj[ 0 ]->fl2 = selobj[ 0 ]->fl1 + w;
            selobj[ 0 ]->fr1 = cur_form->w_hr - selobj[ 0 ]->fl1;
            selobj[ 0 ]->fr2 = cur_form->w_hr - selobj[ 0 ]->fl2;
            selobj[ 0 ]->ft1 = selobj[ 0 ]->h = h;
            selobj[ 0 ]->ft2 = selobj[ 0 ]->ft1 + h;
            selobj[ 0 ]->fb1 = cur_form->h_hr - selobj[ 0 ]->ft1;
            selobj[ 0 ]->fb2 = cur_form->h_hr - selobj[ 0 ]->ft2;
            set_bounding_box( 0.0, 0.0, cur_form->w, cur_form->h );
            fl_winresize( main_window, cur_form->w, cur_form->h );

            fl_notify_object( selobj[ 0 ], FL_RESIZED );
        }
    }
    else if ( s )
    {
        copy_selection( );
        paste_selection( );
    }
    else
    {
        ox = x;
        oy = y;
        ow = w;
        oh = h;

        /* Show the rubberband box */

        if ( mx <= x + HS && my <= y + HS )
        {
            x += w;
            y += h;
            w = -w;
            h = -h;
            scale_box( &x, &y, &w, &h );
        }
        else if ( mx <= x + HS && my >= y + h - HS )
        {
            x += w;
            w = -w;
            scale_box( &x, &y, &w, &h );
        }
        else if ( mx >= x + w - HS && my <= y + HS )
        {
            y += h;
            h = -h;
            scale_box( &x, &y, &w, &h );
        }
        else if ( mx >= x + w - HS && my >= y + h - HS )
            scale_box( &x, &y, &w, &h );
        else
            move_box( &x, &y, &w, &h, FL_TRUE );

        /* Recompute object sizes */

        for ( i = 0; i < selnumb; i++ )
        {
            if (    selobj[ i ]->objclass != FL_BEGIN_GROUP
                 && selobj[ i ]->objclass != FL_END_GROUP )
            {
                selobj[ i ]->fl1 -= ox;
                selobj[ i ]->fl2 -= ox;
                selobj[ i ]->fr1 += ox;
                selobj[ i ]->fr2 += ox;

                selobj[ i ]->ft1 -= oy;
                selobj[ i ]->ft2 -= oy;
                selobj[ i ]->fb1 += oy;
                selobj[ i ]->fb2 += oy;

                fl_scale_object( selobj[ i ], w / ow, h / oh);

                selobj[ i ]->x = selobj[ i ]->fl1 += x;
                selobj[ i ]->fl2 += x;
                selobj[ i ]->fr1 -= x;
                selobj[ i ]->fr2 -= x;

                selobj[ i ]->y = selobj[ i ]->ft1 += y;
                selobj[ i ]->ft2 += y;
                selobj[ i ]->fb1 -= y;
                selobj[ i ]->fb2 -= y;

                fl_notify_object( selobj[ i ], FL_RESIZED );
            }
        }
    }

    hidden = FL_FALSE;
    redraw_the_form( backf );
    changed = FL_TRUE;
}


/***************************************
 * We know how many pixles to move
 ***************************************/

void
move_selection( FL_Coord dx,
                FL_Coord dy )
{
    int i;
    float x,
          y,
          w,
          h;
    float ox,
          oy;
    FL_OBJECT *ob;

    if ( ! cur_form || backf || selnumb == 0 )
        return;

    compute_selbox( &x, &y, &w, &h );

    ox = x;
    oy = y;

    if ( ( x += dx ) < 0 )
        x = 0.0;
    else if ( x + w > winw )
        x = winw - w;

    if ( ( y += dy ) < 0 )
        y = 0.0;
    else if ( y + h > winh )
        y = winh - h;

    if ( ( dx = x - ox ) == 0 && ( dy = y - oy ) == 0 )
        return;

    for ( i = 0; i < selnumb; i++ )
    {
        ob = selobj[ i ];
        if ( ob->objclass != FL_BEGIN_GROUP && ob->objclass != FL_END_GROUP )
        {
            ob->x   += dx;
            ob->fl1 += dx;
            ob->fl2 += dx;
            ob->fr1 -= dx;
            ob->fr2 -= dx;

            ob->y   += dy;
            ob->ft1 += dy;
            ob->ft2 += dy;
            ob->fb1 -= dy;
            ob->fb2 -= dy;

            fl_notify_object( ob, FL_RESIZED );
        }
    }

    redraw_the_form( 1 );
    changed = FL_TRUE;
}


/***************************************
 * Change the selection  size
 ***************************************/

#define MINSIZE 5
#define DELTA 0.2

void
resize_selection( FL_Coord dx,
                  FL_Coord dy )
{
    float x,
          y,
          w,
          h,
          ox,
          oy,
          ow,
          oh;
    float yscale,
          xscale;
    int i;

    if ( ! cur_form || selnumb == 0 )
        return;

    compute_selbox(&x, &y, &w, &h);

    ox = x;
    oy = y;
    ow = w;
    oh = h;

    if ( backf )
    {
        winw = fl_scrw;
        winh = fl_scrh;
    }

    if ( ( w += dx ) > winw)
        w = winw;
    else if ( w < MINSIZE )
        w = MINSIZE;

    if ( ( h += dy ) > winh )
        h = winh;
    else if ( h < MINSIZE )
        h = MINSIZE;

    if ( w == ow && oh == h )
        return;

    xscale = ( float ) w / ow;
    yscale = ( float ) h / oh;

    /* Recompute object sizes */

    for ( i = 0; i < selnumb; i++ )
        if (    selobj[ i ]->objclass != FL_BEGIN_GROUP
             && selobj[ i ]->objclass != FL_END_GROUP )
        {
            selobj[ i ]->fl1 -= ox;
            selobj[ i ]->fl2 -= ox;
            selobj[ i ]->fr1 += ox;
            selobj[ i ]->fr2 += ox;

            selobj[ i ]->ft1 -= oy;
            selobj[ i ]->ft2 -= oy;
            selobj[ i ]->fb1 += oy;
            selobj[ i ]->fb2 += oy;

            fl_scale_object( selobj[ i ], xscale, yscale );

            selobj[ i ]->x = selobj[ i ]->fl1 += x;
            selobj[ i ]->fl2 += x;
            selobj[ i ]->fr1 -= x;
            selobj[ i ]->fr2 -= x;

            selobj[ i ]->y = selobj[ i ]->ft1 += y;
            selobj[ i ]->ft2 += y;
            selobj[ i ]->fb1 -= y;
            selobj[ i ]->fb2 -= y;

            fl_notify_object( selobj[ i ], FL_RESIZED );
        }

    if ( backf )
    {
        cur_form->w_hr = cur_form->w = selobj[ 0 ]->w;
        cur_form->h_hr = cur_form->h = selobj[ 0 ]->h;
        fl_winresize( main_window, cur_form->w, cur_form->h );
    }

    redraw_the_form( 1 );
    changed = FL_TRUE;
}


/***************************************
 * Handles the selection of objects
 ***************************************/

void
handle_select( const XEvent * xev )
{
    int s;
    FL_OBJECT *obj,
              *mouseobj;
    float x,
          y,
          w,
          h;
    float stepsize;

    if ( cur_form == NULL || ! ( mouseobj = find_mouseobj( ) ) )
        return;

    if ( ( s = ShiftIsDown( xev->xbutton.state ) ) )      /* Shift Push */
    {
        if ( ! cur_form->first )
            fprintf( stderr, "something is wrong\n" );
        if ( mouseobj == BackOBJ( ) )
            return;

        if ( find_selobject( mouseobj ) == -1 )
            addto_selection( mouseobj );
        else
            deletefrom_selection( mouseobj );

        return;
    }

    clear_selection( );

    find_mousepos( &x, &y );
    w = 0.0;
    h = 0.0;

    stepsize = get_step_size( );
    set_step_size( 0.0 );
    if ( xev->type != ButtonRelease )
        scale_box( &x, &y, &w, &h );
    set_step_size( stepsize );
    obj = BackOBJ( )->next;

    while ( obj != NULL )
    {
        if (    obj->objclass != FL_BEGIN_GROUP
             && obj->objclass != FL_END_GROUP
              && obj->x >= x
              && obj->y >= y
              && obj->x + obj->w <= x + w
              && obj->y + obj->h <= y + h )
            addto_selection( obj );

        obj = obj->next;
    }

    if ( selnumb == 0 )
    {
        if ( mouseobj == NULL )
            return;
        else if ( mouseobj == BackOBJ( ) )
        {
            addto_selection( mouseobj );
            backf = FL_TRUE;
        }
        else
            addto_selection( mouseobj );
    }
}


/***************************************
 * Selects all objects in the form.
 ***************************************/

void
select_all( void )
{
    FL_OBJECT *obj;

    if ( ! cur_form )
        return;

    clear_selection( );
    
    for ( obj = BackOBJ( )->next; obj != NULL; obj = obj->next )
        if ( ! obj->parent )
            selobj[ selnumb++ ] = obj;

    cleanup_selection( );
}


/****
  Operations on the selection
****/

/***************************************
 * Clone curobj's attributes to the currently selected objects
 ***************************************/

void
change_selected_objects( FL_OBJECT * curobj )
{
    int i;
    FL_OBJECT *ob;

    for ( i = 0; i < selnumb; i++ )
    {
        ob = selobj[ i ];
        if ( ob->objclass != FL_BEGIN_GROUP && ob->objclass != FL_END_GROUP )
        {
            change_type( ob, curobj->type );
            set_attribs( ob, curobj->boxtype, curobj->col1, curobj->col2,
                         curobj->lcol, curobj->align,
                         curobj->lsize, curobj->lstyle, ob->label );
            set_miscattribs( ob, curobj->nwgravity, curobj->segravity,
                             curobj->resize );
            fli_handle_object( ob, FL_ATTRIB, 0, 0, 0, NULL, 0 );
        }
    }
}


/***************************************
 * Interactively change the attributes of the selection
 ***************************************/

void
change_selection( void )
{
    FL_OBJECT *firstobj = NULL;
    int objclass = -1,
        i;
    FL_OBJECT *ob;

    if ( ! cur_form )
        return;

    if ( selnumb == 0 )
    {
        fl_show_alert( "", "Please select an object first",
                       "by clicking the right button on the object", 0 );
        return;
    }

    if ( selnumb == 1 )
        change_object( selobj[ 0 ], FL_TRUE );
    else
    {
        for ( i = 0; i < selnumb; i++ )
        {
            ob = selobj[ i ];
            if (    ob->objclass != FL_BEGIN_GROUP
                 && ob->objclass != FL_END_GROUP )
            {
                if ( firstobj == NULL )
                {
                    firstobj = ob;
                    objclass = ob->objclass;
                }
                else if ( objclass != ob->objclass )
                {
                    fl_show_messages( "Selected objects have different "
                                      "classes" );
                    return;
                }
            }
        }

        if ( firstobj == NULL )
            return;

        if ( ! change_object( firstobj, FL_FALSE ) )
            return;

        change_selected_objects( firstobj );
    }

    changed = 1;
}


/***************************************
 * Aligns the objects in the selection
 ***************************************/

void
align_selection( int dir )
{
    float x,
          y,
          w,
          h,
          gap,
          shift;
    int used[ MAXSEL ],
        current;
    int i,
        j;

    if ( backf || ! cur_form )
        return;         /* Cannot align the backface */

    if ( selnumb <= 1 )
        return;         /* Nothing to align */

    compute_selbox( &x, &y, &w, &h );

    if ( dir == FD_HEQUAL ) /* Horizontal equal distance */
    {
        gap = 0.0;

        for ( i = 0; i < selnumb; i++ )
            gap += selobj[ i ]->w;

        gap = ( w - gap ) / ( selnumb - 1 );

        for ( i = 0; i < selnumb; i++ )
            used[ i ] = 0;

        for ( j = 0; j < selnumb; j++ )
        {
            current = -1;
            for ( i = 0; i < selnumb; i++ )
                if ( ! used[ i ] )
                    if (    current == -1
                         || selobj[ i ]->x < selobj[ current ]->x )
                        current = i;
            used[ current ] = 1;
            shift = x - selobj[ current ]->x;
            selobj[ current ]->x   += shift;
            selobj[ current ]->fl1 += shift;
            selobj[ current ]->fl2 += shift;
            selobj[ current ]->fr1 -= shift;
            selobj[ current ]->fr2 -= shift;
            x += selobj[ current ]->w + gap;
        }
    }
    else if ( dir == FD_VEQUAL )    /* Vertical equal distance */
    {
        gap = 0.0;

        for ( i = 0; i < selnumb; i++ )
            gap += selobj[ i ]->h;

        gap = ( h - gap ) / ( selnumb - 1 );

        for ( i = 0; i < selnumb; i++ )
            used[ i ] = 0;

        for ( j = 0; j < selnumb; j++ )
        {
            current = -1;
            for ( i = 0; i < selnumb; i++ )
                if ( ! used[ i ] )
                    if (    current == -1
                         || selobj[ i ]->y < selobj[ current ]->y )
                        current = i;
            used[ current ] = 1;
            shift = y - selobj[ current ]->y;
            selobj[ current ]->y   += shift;
            selobj[ current ]->ft1 += shift;
            selobj[ current ]->ft2 += shift;
            selobj[ current ]->fb1 -= shift;
            selobj[ current ]->fb2 -= shift;
            y += selobj[ current ]->h + gap;
        }
    }
    else
        for ( i = 0; i < selnumb; i++ )
        {
            switch ( dir )
            {
                case FD_LEFT:   /* Left */
                    shift = x - selobj[ i ]->x;
                    selobj[ i ]->x   += shift;
                    selobj[ i ]->fl1 += shift;
                    selobj[ i ]->fl2 += shift;
                    selobj[ i ]->fr1 -= shift;
                    selobj[ i ]->fr2 -= shift;
                    break;

                case FD_HCENTER:    /* Center */
                    shift = x + w / 2.0 - selobj[ i ]->w / 2.0 - selobj[ i ]->x;
                    selobj[ i ]->x   += shift;
                    selobj[ i ]->fl1 += shift;
                    selobj[ i ]->fl2 += shift;
                    selobj[ i ]->fr1 -= shift;
                    selobj[ i ]->fr2 -= shift;
                    break;

                case FD_RIGHT:  /* Right */
                    shift = x + w - selobj[ i ]->w - selobj[ i ]->x;
                    selobj[ i ]->x   += shift;
                    selobj[ i ]->fl1 += shift;
                    selobj[ i ]->fl2 += shift;
                    selobj[ i ]->fr1 -= shift;
                    selobj[ i ]->fr2 -= shift;
                    break;

                case FD_TOP:
                    shift = y - selobj[ i ]->y;
                    selobj[ i ]->y   += shift;
                    selobj[ i ]->ft1 += shift;
                    selobj[ i ]->ft2 += shift;
                    selobj[ i ]->fb1 -= shift;
                    selobj[ i ]->fb2 -= shift;
                    break;

                case FD_VCENTER:    /* Center */
                    shift = y + h / 2.0 - selobj[ i ]->h / 2.0 - selobj[ i ]->y;
                    selobj[ i ]->y   += shift;
                    selobj[ i ]->ft1 += shift;
                    selobj[ i ]->ft2 += shift;
                    selobj[ i ]->fb1 -= shift;
                    selobj[ i ]->fb2 -= shift;
                    break;

                case FD_BOTTOM:
                    shift = y + h - selobj[ i ]->h - selobj[ i ]->y;
                    selobj[ i ]->y   += shift;
                    selobj[ i ]->ft1 += shift;
                    selobj[ i ]->ft2 += shift;
                    selobj[ i ]->fb1 -= shift;
                    selobj[ i ]->fb2 -= shift;
                    break;
            }
        }

    redraw_the_form( 0 );
    changed = 1;
}


/***************************************
 * Shows all objects in the selection.
 ***************************************/

void
show_selection( void )
{
    int i;

    if ( backf )
        return;         /* Cannot show the backface */

    if ( ! cur_form )
        return;

    for ( i = 0; i < selnumb; i++ )
        fl_show_object( selobj[ i ] );
}


/***************************************
 * Hides all objects in the selection.
 ***************************************/

void
hide_selection( void )
{
    int i;

    if ( backf )
        return;         /* Cannot hide the backface */

    if ( ! cur_form )
        return;

    for ( i = 0; i < selnumb; i++ )
        fl_hide_object( selobj[ i ] );
}


/***************************************
 * Raises the selected objects
 ***************************************/

void
raise_selection( void )
{
    int i;

    if ( backf )
        return;         /* Cannot raise the backface */

    if ( ! cur_form )
        return;

    for ( i = 0; i < selnumb; i++ )
    {
        fl_delete_object( selobj[ i ] );
        if (    selobj[ i ]->objclass != FL_BEGIN_GROUP
             && selobj[ i ]->objclass != FL_END_GROUP )
            fl_add_object( cur_form, selobj[ i ] );
    }

    changed = 1;
}


/***************************************
 * Lowers the selection
 ***************************************/

void
lower_selection( void )
{
    int i;

    if ( backf )
        return;         /* Cannot lower the backface. */

    if ( ! cur_form )
        return;

    for ( i = selnumb - 1; i >= 0; i-- )
    {
        if ( selobj[ i ] != BackOBJ( )->next )
        {
            fl_delete_object( selobj[ i ] );
            fli_insert_object( selobj[ i ], BackOBJ( )->next );
        }
    }

    changed = 1;
}


static FL_OBJECT *cutbuf[ MAXSEL ]; /* Buffered objects */
static int ncut = 0;                /* and their number */

/***************************************
 ***************************************/

static void
clear_cutbuffer( void )
{
    for ( ; --ncut >= 0; )
    {
        if ( cutbuf[ ncut ]->u_vdata )
            fl_free( cutbuf[ ncut ]->u_vdata );
        fl_free_object( cutbuf[ ncut ] );
    }
}


/***************************************
 * Removes all elements in the selection.
 ***************************************/

void
cut_selection( void )
{
    int i;

    if (backf)
        return;         /* Cannot cut the backface. */

    if (! cur_form )
    {
        fl_show_alert("Warning", "Please Add a form first", "", 0);
        return;
    }

    if ( selnumb == 0 )
        return;

    clear_cutbuffer( );

    /* Make new deletion and save it */

    for ( i = 0; i < selnumb; i++ )
    {
        fl_delete_object( selobj[ i ] );
        cutbuf[ i ] = copy_object( selobj[ i ], 1 );
    }

    ncut = selnumb;
    clear_selection( );

    changed = 1;
}


/***************************************
 * Pastes elements from buffer into form
 ***************************************/

void
paste_selection(void)
{
    FL_OBJECT *obj;
    float x,
          y,
          w,
          h,
          ox,
          oy,
          shift;
    int i;

    if ( ! cur_form || ncut == 0 )
        return;

    is_pasting = 1;     /* horrible hack */

    /* Copy selection from buffer */

    clear_selection( );
    redraw_the_form( 0 );

    for ( i = 0; i < ncut; i++ )
    {
        if (    selobj[ i ]->objclass == FL_BEGIN_GROUP
             || selobj[ i ]->objclass == FL_END_GROUP )
            continue;

        obj = copy_object( cutbuf[ i ], 1 );

        /* Fix label:  if underlining caused by cutbuf shortcut, remove it.
           Note can't use cutbuf as cutbuf does not contain shortcut info */

        if (    obj->label
             && strchr( obj->label, *fl_ul_magic_char )
             && selobj[ i ]->shortcut[ 0 ] )
        {
            char *t, *b;

            b = t = fl_strdup( obj->label );
            while ( ( b = strchr( b, *fl_ul_magic_char ) ) )
                memmove( b, b + 1, strlen( b ) );
            fl_set_object_label( obj, t );
            fl_free( t );
        }

        fl_add_object( cur_form, obj );
        selobj[ selnumb++ ] = obj;
    }

    /* Move the selection to the correct place */

    compute_selbox( &x, &y, &w, &h );
    ox = x;
    oy = y;
    move_box( &x, &y, &w, &h, FL_FALSE );

    /* Recompute object position */

    for ( i = 0; i < selnumb; i++ )
        if (    selobj[ i ]->objclass != FL_BEGIN_GROUP
             && selobj[ i ]->objclass != FL_END_GROUP)
        {
            shift = x - ox;
            selobj[ i ]->x   += shift;
            selobj[ i ]->fl1 += shift;
            selobj[ i ]->fl2 += shift;
            selobj[ i ]->fr1 -= shift;
            selobj[ i ]->fr2 -= shift;

            shift = y - oy;
            selobj[ i ]->y   += shift;
            selobj[ i ]->ft1 += shift;
            selobj[ i ]->ft2 += shift;
            selobj[ i ]->fb1 -= shift;
            selobj[ i ]->fb2 -= shift;

            fl_notify_object( selobj[ i ], FL_RESIZED );
        }

    cleanup_selection( );
    redraw_the_form( 0 );
    changed = 1;
    is_pasting = 0;
}


/***************************************
 * Copies all elements in the selection to the buffer
 ***************************************/

void
copy_selection( void )

{
    FL_OBJECT *obj;
    int i;

    if ( backf || selnumb == 0 || ! cur_form )
        return;

    clear_cutbuffer( );

    /* Copy the objects */

    for ( i = 0; i < selnumb; i++ )
    {
        obj = copy_object( selobj[ i ], 0 );
        cutbuf[ i ] = obj;
    }

    ncut = selnumb;
}


/***************************************
 * Makes a copy of the current selection
 ***************************************/

void *
dup_selection( void )
{
    FL_OBJECT **ob;
    int i;

    if ( ! selnumb )
        return NULL;

    ob = fl_calloc( selnumb + 1, sizeof *ob );

    for ( i = 0; i < selnumb; i++ )
        ob[ i ] = copy_object( selobj[ i ], 1 );

    return ob;
}


/***************************************
 ***************************************/

void
free_dupped_selection( void *a )
{
    FL_OBJECT **ob = a;

    for ( ; *ob; ob++ )
        fl_free_object( *ob );

    fl_free( a );
}


/***************************************
 * Changes the selection to a new list of objects and show it
 ***************************************/

void
set_selection( void *a )
{
    FL_OBJECT *obj,
              **ob = a;
    int i;

    for ( i = 0; i < selnumb; i++ )
        fl_delete_object( selobj[ i ] );

    clear_selection( );
    redraw_the_form( 0 );

    for ( selnumb = 0; ob[ selnumb ]; selnumb++ )
    {
        obj = copy_object( ob[ selnumb ], 1 );
        if (    selobj[ i ]->objclass != FL_BEGIN_GROUP
             && selobj[ i ]->objclass != FL_END_GROUP )
            fl_add_object( cur_form, obj );
        selobj[ selnumb ] = obj;
    }

    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
next_selection( void )
{
    if ( ! cur_form || ! BackOBJ( ) )
        return;

    do
    {
        if ( ! selnumb || ! selobj[ 0 ]->next )
            selobj[ 0 ] = BackOBJ( )->next ? BackOBJ( )->next : BackOBJ( );
        else if ( selnumb && selobj[ 0 ]->next )
            selobj[ 0 ] = selobj[ 0 ]->next;

    } while ( selobj[ 0 ]->parent );

    selnumb = 1;
    redraw_the_form( 0 );
}


/***************************************
 ***************************************/

void
prev_selection( void )
{
    if ( ! cur_form || ! BackOBJ( ) )
        return;

    do
    {
        if ( ! selnumb || ! selobj[ 0 ]->prev )
            selobj[ 0 ] = BackOBJ( )->prev ? BackOBJ( )->prev : BackOBJ( );
        else if ( selnumb && selobj[ 0 ]->prev )
            selobj[ 0 ] = selobj[ 0 ]->prev;
    } while (selobj[ 0 ]->parent );

    selnumb = 1;
    redraw_the_form( 0 );
}


/***************************************
 * Groups all elements in the selection into a group
 ***************************************/

void
group_selection( void )
{
    int i;
    FL_OBJECT *obj;
    const char *s;
    const char *sp;

    if ( backf )
        return;         /* Cannot group the backface */

    if ( ! cur_form || selnumb == 0 )
        return;

 get_new_group_name:

    if (    ! ( s = fl_show_input( "Enter group name (must be usable as "
                                   "a C variable):", "" ) )
          || ! *s )
        return;

    if (    ! isascii( ( unsigned char ) *s )
         || ! ( isalpha( ( unsigned char ) *s ) || *s == '_' ) )
    {
        fl_show_alert( "Error", "Invalid C identifier specified for group "
                       "name:", s, 0 );
        goto get_new_group_name;
    }

    for ( sp = s + 1; *sp; sp++ )
        if (    ! isascii( ( unsigned char ) *sp )
             || ! ( isalnum( ( unsigned char ) *sp ) || *sp == '_' ) )
        {
            fl_show_alert( "Error", "Invalid C identifier specified for group "
                           "name:", s, 0 );
            goto get_new_group_name;
        }

    obj = add_an_object( FL_BEGIN_GROUP, -1, 0, 0, 0, 0 );

    for ( i = 0; i < selnumb; i++ )
    {
        fl_delete_object( selobj[ i ] );

        if (    selobj[ i ]->objclass != FL_BEGIN_GROUP
             && selobj[ i ]->objclass != FL_END_GROUP )
            fl_add_object( cur_form, selobj[ i ] );
    }

    add_an_object( FL_END_GROUP, -1, 0, 0, 0, 0 );
    clear_selection( );
    set_object_name( obj, s, "", "" );
    addgroupto_selection( obj );
    changed = 1;
}


/***************************************
 * Removes begin and endgroups from the selection
 ***************************************/

void
flatten_selection( void )
{
    int i;

    if ( backf )
        return;         /* Cannot flatten the backface */

    if ( ! cur_form )
        return;

    for ( i = 0; i < selnumb; i++ )
        if (    selobj[ i ]->objclass == FL_BEGIN_GROUP
             || selobj[ i ]->objclass == FL_END_GROUP)
        {
            fl_delete_object( selobj[ i ] );

            /* CAUSES PROBLEMS WITH NAMING fl_free_object(selobj[i]); */

            selobj[ i ] = NULL;
        }

    cleanup_selection( );
    changed = 1;
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
