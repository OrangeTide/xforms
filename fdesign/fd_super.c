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
 * \file fd_super.c
 *
 *  This file is part of XForms package
 *  Copyright (c) 1996-2002  T.C. Zhao and Mark Overmars
 *  All rights reserved.
 *
 * translation between superspec and spec
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include/forms.h"
#include "fd_main.h"
#include "fd_spec.h"

#include "private/pslider.h"
#include "private/pbrowser.h"
#include "private/ppositioner.h"
#include "private/pcounter.h"
#include "private/pspinner.h"
#include "private/pscrollbar.h"
#include "private/pdial.h"
#include "private/pxyplot.h"
#include "private/pchoice.h"
#include "private/pmenu.h"


/***************************************
 ***************************************/

SuperSPEC *
get_superspec( FL_OBJECT * ob )
{
    SuperSPEC *sp = ob->u_vdata;

    return sp ? sp : spec_to_superspec( ob );
}


/***************************************
 ***************************************/

SuperSPEC *
spec_to_superspec( FL_OBJECT * obj )
{
    SuperSPEC *spp;
    int i;
    int n;

    if ( ! obj->u_vdata )
    {
        obj->u_vdata = spp = fl_calloc( 1, sizeof *spp );

        spp->content    = NULL;
        spp->shortcut   = NULL;
        spp->callback   = NULL;
        spp->mode       = NULL;
        spp->mval       = NULL;
        spp->cspecv     = NULL;

        spp->new_menuapi = 0;
        spp->nlines = 0;
    }
    else
        spp = obj->u_vdata;

    spp->how_return = obj->how_return;

    if ( obj->objclass == FL_BROWSER )
    {
        FLI_BROWSER_SPEC *sp = obj->spec;

        spp->h_pref = sp->h_pref;
        spp->v_pref = sp->v_pref;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fli_safe_free( spp->content[ i ] );
            fli_safe_free( spp->shortcut[ i ] );
            fli_safe_free( spp->callback[ i ] );
        }

        n = fl_get_browser_maxline( obj );

        spp->content  = fl_realloc( spp->content,
                                    ( n + 1 ) * sizeof *spp->content );
        spp->shortcut = fl_realloc( spp->shortcut,
                                    ( n + 1 ) * sizeof *spp->shortcut );
        spp->callback = fl_realloc( spp->callback,
                                    ( n + 1 ) * sizeof *spp->callback );
        spp->mode     = fl_realloc( spp->mode,
                                    ( n + 1 ) * sizeof *spp->mode );
        spp->mval     = fl_realloc( spp->mval,
                                    ( n + 1 ) * sizeof *spp->mval );

        spp->nlines = n;
        for ( i = 1; i <= n; i++ )
        {
            spp->content[ i ] = fl_strdup( fl_get_browser_line( obj, i ) );
            spp->shortcut[ i ] = spp->callback[ i ] = NULL;
        }
    }
    else if ( obj->objclass == FL_CHOICE )
    {
        FLI_CHOICE_SPEC *sp = obj->spec;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fli_safe_free( spp->content[ i ] );
            fli_safe_free( spp->shortcut[ i ] );
        }

        n = spp->nlines = sp->numitems;

        spp->content  = fl_realloc( spp->content,
                                    ( n + 1 ) * sizeof *spp->content );
        spp->shortcut = fl_realloc( spp->shortcut,
                                    ( n + 1 ) * sizeof *spp->shortcut );
        spp->callback = fl_realloc( spp->callback,
                                    ( n + 1 ) * sizeof *spp->callback );
        spp->mode     = fl_realloc( spp->mode,
                                    ( n + 1 ) * sizeof *spp->mode );
        spp->mval     = fl_realloc( spp->mval,
                                    ( n + 1 ) * sizeof *spp->mval );

        spp->align   = sp->align;
        spp->int_val = sp->val;

        for ( i = 1; i <= n; i++ )
        {
            spp->mode[ i ] = sp->mode[ i ];
            spp->content[ i ] = fl_strdup( fl_get_choice_item_text( obj, i ) );

            if ( sp->shortcut[ i ] )
                spp->shortcut[ i ] = fl_strdup( sp->shortcut[ i ] );
            else
                spp->shortcut[ i ] = NULL;
        }
    }
    else if ( obj->objclass == FL_MENU )
    {
        FLI_MENU_SPEC *sp = obj->spec;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fli_safe_free( spp->content[ i ] );
            fli_safe_free( spp->shortcut[ i ] );
            fli_safe_free( spp->callback[ i ] );
        }

        n = spp->nlines = sp->numitems;

        spp->content  = fl_realloc( spp->content,
                                    ( n + 1 ) * sizeof *spp->content );
        spp->shortcut = fl_realloc( spp->shortcut,
                                    ( n + 1 ) * sizeof *spp->shortcut );
        spp->callback = fl_realloc( spp->callback,
                                    ( n + 1 ) * sizeof *spp->callback );
        spp->mode     = fl_realloc( spp->mode,
                                    ( n + 1 ) * sizeof *spp->mode );
        spp->mval     = fl_realloc( spp->mval,
                                    ( n + 1 ) * sizeof *spp->mval );

        for ( i = 1; i <= n; i++ )
        {
            spp->mode[ i ] = sp->mode[ i ];
            spp->mval[ i ] = sp->mval[ i ];
            spp->content[ i ] =
                      fl_strdup( fl_get_menu_item_text( obj, sp->mval[ i ] ) );

            if ( sp->shortcut[ i ] )
                spp->shortcut[ i ] = fl_strdup( sp->shortcut[ i ] );
            else
                spp->shortcut[ i ] = NULL;

            if ( sp->cb[ i ] )
                spp->callback[ i ] = fl_strdup( ( char * ) sp->cb[ i ] );
            else
                spp->callback[ i ] = NULL;
        }
    }
    else if (    obj->objclass == FL_SLIDER
              || obj->objclass == FL_VALSLIDER
              || obj->objclass == FL_THUMBWHEEL )
    {
        FLI_SLIDER_SPEC *sp = obj->spec;

        spp->val        = sp->val;
        spp->min        = sp->min;
        spp->max        = sp->max;
        spp->step       = sp->step;
        spp->prec       = sp->prec;
        spp->ldelta     = sp->ldelta;
        spp->rdelta     = sp->rdelta;
        spp->slsize     = sp->slsize;
    }
    else if (    ISBUTTON( obj->objclass )
              || obj->objclass == FL_PIXMAP
              || obj->objclass == FL_BITMAP )
    {
        FL_BUTTON_SPEC *sp = obj->spec;
        IconInfo *info;

        spp->mbuttons = 0;
        for ( i = 0; i < 5; i++ )
            if ( sp->react_to[ i ] )
                spp->mbuttons |= 1 << i; 

        spp->int_val = sp->val;

        if ( ! spp->cspecv )
        {
            info = spp->cspecv = fl_calloc( 1, sizeof *info );

            info->show_focus = 1;
            info->dx         = info->dy = 3;
            info->align      = FL_ALIGN_CENTER;
            info->fullpath   = 1;
        }

        info = spp->cspecv;

        if (    obj->objclass == FL_PIXMAPBUTTON
             || obj->objclass == FL_BITMAPBUTTON
             || obj->objclass == FL_PIXMAP
             || obj->objclass == FL_BITMAP )
        {
            spp->align = info->align;
            spp->dx = info->dx;
            spp->dy = info->dy;
            spp->show_focus = info->show_focus;
            spp->use_data = info->use_data;
            spp->fullpath = info->fullpath;
            strcpy( spp->filename, info->filename );
            strcpy( spp->data, info->data );
            strcpy( spp->focus_filename, info->focus_filename );
            strcpy( spp->focus_data, info->focus_data );
            strcpy( spp->width, info->width );
            strcpy( spp->height, info->height );
        }
    }
    else if ( obj->objclass == FL_POSITIONER )
    {
        FLI_POSITIONER_SPEC *sp = obj->spec;

        spp->xstep      = sp->xstep;
        spp->ystep      = sp->ystep;
        spp->xmin       = sp->xmin;
        spp->xmax       = sp->xmax;
        spp->xval       = sp->xval;
        spp->ymin       = sp->ymin;
        spp->ymax       = sp->ymax;
        spp->yval       = sp->yval;
    }
    else if ( obj->objclass == FL_COUNTER )
    {
        FLI_COUNTER_SPEC *sp = obj->spec;

        spp->val        = sp->val;
        spp->lstep      = sp->lstep;
        spp->sstep      = sp->sstep;
        spp->min        = sp->min;
        spp->max        = sp->max;
        spp->prec       = sp->prec;
    }
    else if ( obj->objclass == FL_SPINNER )
    {
        spp->dval = fl_get_spinner_value( obj );
        fl_get_spinner_bounds( obj, &spp->dmin, &spp->dmax );
        spp->dstep = fl_get_spinner_step( obj );
        spp->prec = fl_get_spinner_precision( obj );
    }
    else if ( obj->objclass == FL_DIAL )
    {
        FLI_DIAL_SPEC *sp = obj->spec;

        spp->min        = sp->min;
        spp->max        = sp->max;
        spp->val        = sp->val;
        spp->step       = sp->step;
        spp->thetai     = sp->thetai;
        spp->thetaf     = sp->thetaf;
        spp->direction  = sp->direction;
    }
    else if ( obj->objclass == FL_XYPLOT )
    {
        FLI_XYPLOT_SPEC *sp = obj->spec;

        spp->xmajor         = sp->xmajor;
        spp->xminor         = sp->xminor;
        spp->ymajor         = sp->ymajor;
        spp->yminor         = sp->yminor;
        spp->xscale         = sp->xscale;
        spp->yscale         = sp->yscale;
        spp->xgrid          = sp->xgrid;
        spp->ygrid          = sp->ygrid;
        spp->grid_linestyle = sp->grid_linestyle;
        spp->xbase          = sp->xbase;
        spp->ybase          = sp->ybase;
        spp->mark_active    = sp->mark_active;
    }
    else if ( obj->objclass == FL_SCROLLBAR )
    {
        FLI_SCROLLBAR_SPEC *scbsp = obj->spec;
        FLI_SLIDER_SPEC *sp = scbsp->slider->spec;

        spp->val    = sp->val;
        spp->min    = sp->min;
        spp->max    = sp->max;
        spp->prec   = sp->prec;
        spp->step   = sp->step;
        spp->slsize = sp->slsize;
        spp->ldelta = sp->ldelta;
        spp->rdelta = sp->rdelta;
    }
    else if ( obj->objclass == FL_SPINNER )
    {
        FLI_SPINNER_SPEC *sp = obj->spec;

        spp->i_val  = sp->i_val;
        spp->i_min  = sp->i_min;
        spp->i_max  = sp->i_max;
        spp->i_incr = sp->i_incr;
        spp->f_val  = sp->f_val;
        spp->f_min  = sp->f_min;
        spp->f_max  = sp->f_max;
        spp->f_incr = sp->f_incr;
        spp->orient = sp->orient;
        spp->prec   = sp->prec;
    }

    return spp;
}


/***************************************
 ***************************************/

void *
superspec_to_spec( FL_OBJECT * obj )
{
    void *v = obj->spec;
    SuperSPEC *spp = obj->u_vdata;
    int i = 0;

    if ( ! spp )
        return v;

    obj->how_return = spp->how_return;

    if ( obj->objclass == FL_BROWSER )
    {
        FLI_BROWSER_SPEC *sp = obj->spec;

        fl_clear_browser( obj );

        sp->h_pref = spp->h_pref;
        sp->v_pref = spp->v_pref;

        for ( i = 1; i <= spp->nlines; i++ )
            fl_addto_browser( obj, spp->content[ i ] );
    }
    else if ( obj->objclass == FL_CHOICE )
    {
        fl_clear_choice( obj );

        ( ( FLI_CHOICE_SPEC * ) obj->spec)->align = spp->align;

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fl_addto_choice( obj, spp->content[ i ] );
            fl_set_choice_item_mode( obj, i, spp->mode[ i ] );
            if ( spp->shortcut[ i ] )
                fl_set_choice_item_shortcut( obj, i, spp->shortcut[ i ] );
        }

        if ( spp->nlines >= spp->int_val )
            fl_set_choice( obj, spp->int_val );
    }
    else if ( obj->objclass == FL_MENU )
    {
        fl_clear_menu( obj );

        for ( i = 1; i <= spp->nlines; i++ )
        {
            fl_addto_menu( obj, spp->content[ i ] );
            fl_set_menu_item_mode( obj, i, spp->mode[ i ] );
            if ( spp->shortcut[ i ] )
                fl_set_menu_item_shortcut( obj, i, spp->shortcut[ i ] );
            if ( spp->callback[ i ] )
                fl_set_menu_item_callback( obj, i,
                               ( FL_PUP_CB ) fl_strdup( spp->callback[ i ] ) );
            if ( spp->mval[ i ] != i )
                fl_set_menu_item_id( obj, i, spp->mval[ i ] );
        }
    }
    else if (    obj->objclass == FL_SLIDER
              || obj->objclass == FL_VALSLIDER
              || obj->objclass == FL_THUMBWHEEL)
    {
        FLI_SLIDER_SPEC *sp = obj->spec;

        sp->val    = spp->val;
        sp->min    = spp->min;
        sp->max    = spp->max;
        sp->step   = spp->step;
        sp->prec   = spp->prec;
        sp->ldelta = spp->ldelta;
        sp->rdelta = spp->rdelta;
        sp->slsize = spp->slsize;
    }
    else if (    ISBUTTON( obj->objclass )
              || obj->objclass == FL_PIXMAP
              || obj->objclass == FL_BITMAP )
    {
        FL_BUTTON_SPEC *sp = obj->spec;
        IconInfo *info = spp->cspecv;

        for ( i = 0; i < 5; i++ )
            sp->react_to[ i ] = ( spp->mbuttons & ( 1 << i ) ) != 0;
        if ( ISBUTTON( obj->objclass ) )
            fl_set_button_mouse_buttons( obj, spp->mbuttons );

        sp->val = spp->int_val;
        if ( ISBUTTON( obj->objclass ) )
            fl_set_button( obj, sp->val );

        if (    obj->objclass == FL_PIXMAPBUTTON
             || obj->objclass == FL_BITMAPBUTTON
             || obj->objclass == FL_PIXMAP
             || obj->objclass == FL_BITMAP )
        {
            info->align      = spp->align;
            info->dx         = spp->dx;
            info->dy         = spp->dy;
            info->show_focus = spp->show_focus;
            info->use_data   = spp->use_data;
            info->fullpath   = spp->fullpath;

            strcpy( info->filename, spp->filename );
            strcpy( info->data, spp->data );
            strcpy( info->focus_filename, spp->focus_filename );
            strcpy( info->focus_data, spp->focus_data );
            strcpy( info->width, spp->width );
            strcpy( info->height, spp->height );

            if (    obj->objclass == FL_PIXMAPBUTTON
                 || obj->objclass == FL_PIXMAP )
            {
                fl_set_pixmap_align( obj, info->align | FL_ALIGN_INSIDE,
                                     info->dx, info->dy );
                fl_set_pixmapbutton_focus_outline( obj, info->show_focus );
            }

            if ( *info->filename )
            {
                if (    obj->objclass == FL_PIXMAPBUTTON
                     || obj->objclass == FL_PIXMAP )
                {
                    fl_set_pixmap_file( obj, info->filename );
                    if ( *info->focus_filename )
                        fl_set_pixmapbutton_focus_file( obj,
                                                        info->focus_filename );
                }
                else
                    fl_set_bitmap_file( obj, info->filename );
            }
        }
    }
    else if ( obj->objclass == FL_POSITIONER )
    {
        FLI_POSITIONER_SPEC *sp = obj->spec;

        sp->xstep      = spp->xstep;
        sp->ystep      = spp->ystep;
        sp->xmin       = spp->xmin;
        sp->xmax       = spp->xmax;
        sp->xval       = spp->xval;
        sp->ymin       = spp->ymin;
        sp->ymax       = spp->ymax;
        sp->yval       = spp->yval;
    }
    else if ( obj->objclass == FL_COUNTER )
    {
        FLI_COUNTER_SPEC *sp = obj->spec;

        sp->val        = spp->val;
        sp->sstep      = spp->sstep;
        sp->lstep      = spp->lstep;
        sp->min        = spp->min;
        sp->max        = spp->max;
        sp->prec       = spp->prec;
    }
    else if ( obj->objclass == FL_SPINNER )
    {
        fl_set_spinner_value( obj, spp->dval );
        fl_set_spinner_bounds( obj, spp->dmin, spp->dmax );
        fl_set_spinner_step( obj, spp->dstep );
        fl_set_spinner_precision( obj, spp->prec );
    }
    else if ( obj->objclass == FL_DIAL )
    {
        FLI_DIAL_SPEC *sp = obj->spec;

        sp->min        = spp->min;
        sp->max        = spp->max;
        sp->val        = spp->val;
        sp->step       = spp->step;
        sp->thetai     = spp->thetai;
        sp->thetaf     = spp->thetaf;
        sp->direction  = spp->direction;
    }
    else if ( obj->objclass == FL_XYPLOT )
    {
        FLI_XYPLOT_SPEC *sp = obj->spec;

        sp->xmajor         = spp->xmajor;
        sp->xminor         = spp->xminor;
        sp->ymajor         = spp->ymajor;
        sp->yminor         = spp->yminor;
        sp->xscale         = spp->xscale;
        sp->yscale         = spp->yscale;
        sp->xgrid          = spp->xgrid;
        sp->ygrid          = spp->ygrid;
        sp->xbase          = spp->xbase;
        sp->ybase          = spp->ybase;
        sp->grid_linestyle = spp->grid_linestyle;
        sp->mark_active    = spp->mark_active;
    }
    else if ( obj->objclass == FL_SCROLLBAR )
    {
        FLI_SCROLLBAR_SPEC *scbsp = obj->spec;
        FLI_SLIDER_SPEC *sp = scbsp->slider->spec;

        sp->val    = spp->val;
        sp->min    = spp->min;
        sp->max    = spp->max;
        sp->prec   = spp->prec;
        sp->step   = spp->step;
        sp->slsize = spp->slsize;
        sp->ldelta = spp->ldelta;
        sp->rdelta = spp->rdelta;
    }
    else if ( obj->objclass == FL_SLIDER )
    {
        FLI_SPINNER_SPEC *sp = obj->spec;

        sp->i_val  = spp->i_val;
        sp->i_min  = spp->i_min;
        sp->i_max  = spp->i_max;
        sp->i_incr = spp->i_incr;
        sp->f_val  = spp->f_val;
        sp->f_min  = spp->f_min;
        sp->f_max  = spp->f_max;
        sp->f_incr = spp->f_incr;
        sp->orient = spp->orient;
        sp->prec   = spp->prec;
    }

    return v;
}


/***************************************
 ***************************************/

void
copy_superspec( FL_OBJECT * target,
                FL_OBJECT * src )
{
    void *tmp;

    free_superspec( target );

    tmp = get_superspec( src );
    src->u_vdata = NULL;
    target->u_vdata = get_superspec( src );
    src->u_vdata = tmp;
}


/***************************************
 ***************************************/

void
free_superspec( FL_OBJECT * obj )
{
    int i;
    SuperSPEC *ssp = obj->u_vdata;

    if ( ! ssp )
        return;

    for ( i = 1; i < ssp->nlines; ++i )
    {
        fl_free( ssp->content[ i ] );
        fl_free( ssp->shortcut[ i ] );
        fl_free( ssp->callback[ i ] );
    }

    fl_free( ssp->mode );
    fl_free( ssp->mval );

    fl_free( ssp->cspecv );
    fli_safe_free( obj->u_vdata );
}


/*
 * Local variables:
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
